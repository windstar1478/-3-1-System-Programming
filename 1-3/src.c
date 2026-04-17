///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c                                                         //
// Date : 2026/04/18                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : FTP Server srv program                                            //
// Description : This program receives FTP client commands from standard      //
// input, processes file and directory related requests on the server side,   //
// and prints the corresponding results or error messages to standard output  //
// and standard error. It supports directory listing, path movement, file     //
// transfer, rename, and delete related commands in a simple FTP server       //
// environment.                                                              //
///////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_ENTRIES 1024
#define MAX_NAME_LEN 1024
#define BUF_SIZE 1024

///////////////////////////////////////////////////////////////////////////////////////
// print_error                                                                       //
// ==================================================================================//
// Input: const char* -> error message                                               //
// Output: none                                                                      //
// Purpose: print error message to standard error                                    //
///////////////////////////////////////////////////////////////////////////////////////
void print_error(const char* message) { // function to print error message to stderr
    write(2, message, strlen(message)); // write the error message to stderr (file descriptor 2)
}

///////////////////////////////////////////////////////////////////////////////////////
// cmpstr                                                                            //
// ==================================================================================//
// Input: const void* a -> first string                                              //
//        const void* b -> second string                                             //
// Output: int -> comparison result                                                  //
// Purpose: compare two strings for sorting                                          //
///////////////////////////////////////////////////////////////////////////////////////
int cmpstr(const void* a, const void* b) { // function to compare two strings
    return strcmp((const char*)a, (const char*)b); // compare strings in ASCII order
}

///////////////////////////////////////////////////////////////////////////////////////
// write_str                                                                         //
// ==================================================================================//
// Input: const char* s -> output string                                             //
// Output: none                                                                      //
// Purpose: write string to standard output                                          //
///////////////////////////////////////////////////////////////////////////////////////
void write_str(const char* s) { // function to write string to stdout
    write(1, s, strlen(s)); // write string to stdout (file descriptor 1)
}

///////////////////////////////////////////////////////////////////////////////////////
// is_hidden_name                                                                    //
// ==================================================================================//
// Input: const char* name -> file or directory name                                 //
// Output: int -> 1 if hidden, 0 otherwise                                           //
// Purpose: check whether the name starts with '.'                                   //
///////////////////////////////////////////////////////////////////////////////////////
int is_hidden_name(const char* name) { // function to check hidden entry
    return name[0] == '.'; // hidden files/directories start with '.'
}

///////////////////////////////////////////////////////////////////////////////////////
// make_full_path                                                                    //
// ==================================================================================//
// Input: char* dest -> destination buffer                                           //
//        size_t size -> buffer size                                                 //
//        const char* dir -> directory path                                          //
//        const char* name -> file or directory name                                 //
// Output: none                                                                      //
// Purpose: make full path from directory path and entry name                        //
///////////////////////////////////////////////////////////////////////////////////////
void make_full_path(char* dest, size_t size, const char* dir, const char* name) { // function to build full path
    if (strcmp(dir, ".") == 0) { // if current directory
        snprintf(dest, size, "%s", name); // use only name
    }
    else {
        snprintf(dest, size, "%s/%s", dir, name); // combine directory and name
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// format_mode_string                                                                //
// ==================================================================================//
// Input: mode_t mode -> file mode bits                                              //
//        char* out -> output permission string buffer                               //
// Output: none                                                                      //
// Purpose: convert mode bits to ls -l style permission string                       //
///////////////////////////////////////////////////////////////////////////////////////
void format_mode_string(mode_t mode, char* out) { // function to make permission string
    out[0] = S_ISDIR(mode) ? 'd' : '-';
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0'; // terminate string
}

///////////////////////////////////////////////////////////////////////////////////////
// copy_file                                                                         //
// ==================================================================================//
// Input: const char* src -> source file path                                        //
//        const char* dst -> destination file path                                   //
// Output: int -> 0 success, -1 fail                                                 //
// Purpose: copy one file from source path to destination path                       //
///////////////////////////////////////////////////////////////////////////////////////
int copy_file(const char* src, const char* dst) { // function to copy file contents
    int fd_src = open(src, O_RDONLY); // open source file for reading
    if (fd_src < 0) return -1; // fail if source open error

    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644); // open destination file for writing
    if (fd_dst < 0) {
        close(fd_src); // close source before returning
        return -1;
    }

    char buf[BUF_SIZE];
    ssize_t nread;
    while ((nread = read(fd_src, buf, sizeof(buf))) > 0) { // read source file in chunks
        ssize_t total = 0;
        while (total < nread) { // handle partial write
            ssize_t nwritten = write(fd_dst, buf + total, nread - total); // write chunk to destination
            if (nwritten < 0) {
                close(fd_src);
                close(fd_dst);
                return -1; // fail if write error
            }
            total += nwritten; // accumulate written bytes
        }
    }

    close(fd_src); // close source file
    close(fd_dst); // close destination file

    return (nread < 0) ? -1 : 0; // return fail if read error occurred
}

///////////////////////////////////////////////////////////////////////////////////////
// handle_nlst_like                                                                  //
// ==================================================================================//
// Input: int aflag -> show hidden entries option                                    //
//        int lflag -> long format option                                            //
//        const char* path -> target path                                            //
// Output: int -> 0 success, 1 fail                                                  //
// Purpose: process NLST/LIST style listing for file or directory                    //
///////////////////////////////////////////////////////////////////////////////////////
int handle_nlst_like(int aflag, int lflag, const char* path) { // function to handle listing commands
    struct stat st;
    if (stat(path, &st) != 0) { // check whether path exists
        print_error("Error: failed to access file or directory\n");
        return 1;
    }

    if (S_ISDIR(st.st_mode)) { // if path is directory
        DIR* dp = opendir(path); // open directory
        struct dirent* dirp;

        char names[MAX_ENTRIES][MAX_NAME_LEN]; // store entry names
        int count = 0; // number of valid entries

        if (dp == NULL) {
            print_error("Error: failed to open directory\n");
            return 1;
        }

        while ((dirp = readdir(dp)) != NULL) { // read entries one by one
            if (!aflag && is_hidden_name(dirp->d_name)) { // skip hidden names if -a is not set
                continue;
            }

            if (count >= MAX_ENTRIES) { // prevent overflow
                break;
            }

            char filepath[MAX_NAME_LEN];
            struct stat entry_st;

            make_full_path(filepath, sizeof(filepath), path, dirp->d_name); // build full path for stat

            if (stat(filepath, &entry_st) != 0) {
                closedir(dp);
                print_error("Error: failed to access file or directory\n");
                return 1;
            }

            strcpy(names[count], dirp->d_name); // store entry name

            if (S_ISDIR(entry_st.st_mode)) {
                strcat(names[count], "/"); // append '/' for directories
            }

            count++;
        }

        closedir(dp); // close directory stream

        qsort(names, count, sizeof(names[0]), cmpstr); // sort names

        if (!lflag) { // short listing format
            for (int i = 0; i < count; i++) {
                write(1, names[i], strlen(names[i])); // print name
                if ((i + 1) % 5 == 0 || i == count - 1) {
                    write(1, "\n", 1); // newline after every 5 names or last name
                }
                else {
                    write(1, "\t", 1); // tab between names
                }
            }
        }
        else { // long listing format
            for (int i = 0; i < count; i++) {
                char original_name[MAX_NAME_LEN];
                strcpy(original_name, names[i]); // copy displayed name

                size_t len = strlen(original_name);
                if (len > 0 && original_name[len - 1] == '/') {
                    original_name[len - 1] = '\0'; // remove trailing '/' before stat
                }

                char filepath[MAX_NAME_LEN];
                make_full_path(filepath, sizeof(filepath), path, original_name); // rebuild full path

                struct stat entry_st;
                if (stat(filepath, &entry_st) != 0) {
                    print_error("Error: failed to access file or directory\n");
                    return 1;
                }

                char mode_str[11];
                format_mode_string(entry_st.st_mode, mode_str); // make permission string

                struct tm* tm_info = localtime(&entry_st.st_mtime); // get modified time
                char time_buf[256];
                strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info); // format time

                struct passwd* pw = getpwuid(entry_st.st_uid); // get owner name
                struct group* gr = getgrgid(entry_st.st_gid); // get group name

                char line[2048];
                snprintf(line, sizeof(line), "%s %lu %s %s %5ld %s %s\n",
                    mode_str,
                    (unsigned long)entry_st.st_nlink,
                    pw ? pw->pw_name : "unknown",
                    gr ? gr->gr_name : "unknown",
                    (long)entry_st.st_size,
                    time_buf,
                    names[i]); // build long-format output line

                write_str(line); // print formatted line
            }
        }
    }
    else { // if path is single file
        char* filename = strrchr(path, '/'); // find last '/'
        if (filename == NULL) filename = (char*)path;
        else filename++;

        if (!aflag && is_hidden_name(filename)) { // skip hidden file if -a is not set
            return 0;
        }

        if (!lflag) { // short format for single file
            write(1, filename, strlen(filename));
            write(1, "\n", 1);
        }
        else { // long format for single file
            char mode_str[11];
            format_mode_string(st.st_mode, mode_str);

            struct tm* tm_info = localtime(&st.st_mtime);
            char time_buf[256];
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);

            struct passwd* pw = getpwuid(st.st_uid);
            struct group* gr = getgrgid(st.st_gid);

            char line[2048];
            snprintf(line, sizeof(line), "%s %lu %s %s %5ld %s %s\n",
                mode_str,
                (unsigned long)st.st_nlink,
                pw ? pw->pw_name : "unknown",
                gr ? gr->gr_name : "unknown",
                (long)st.st_size,
                time_buf,
                filename);

            write_str(line); // print long-format single file line
        }
    }

    return 0; // success
}

///////////////////////////////////////////////////////////////////////////////////////
// main                                                                              //
// ==================================================================================//
// Input: none                                                                       //
// Output: int -> 0 success                                                          //
// Purpose: read FTP commands from standard input and process server operations       //
///////////////////////////////////////////////////////////////////////////////////////
int main(void) { // main function of FTP server program
    char rename_from[1024]; // store source path for RNFR/RNTO sequence
    int rnfr_ready = 0; // indicate whether RNFR was completed successfully

    while (1) { // keep processing commands until QUIT or input end
        char buffer[1024];
        int n = read(0, buffer, sizeof(buffer) - 1); // read command input from stdin

        if (n <= 0) { // end if EOF or read error
            break;
        }

        buffer[n] = '\0'; // null-terminate input buffer

        char* start = buffer; // start position for one line

        while (start < buffer + n) { // process line by line within the buffer
            char* end = strchr(start, '\n'); // find line break

            if (end != NULL) {
                *end = '\0'; // split one command line
            }

            if (*start == '\0') { // skip empty line
                if (end == NULL) {
                    break;
                }
                start = end + 1;
                continue;
            }

            char line[1024];
            strcpy(line, start); // copy current line for tokenizing

            char* cmd = strtok(line, " "); // extract command token
            if (cmd == NULL) {
                if (end == NULL) {
                    break;
                }
                start = end + 1;
                continue;
            }

            if (strcmp(cmd, "PWD") == 0) { // print current working directory
                char* extra = strtok(NULL, " ");

                if (extra != NULL) {
                    if (extra[0] == '-') {
                        print_error("Error: invalid option\n");
                    }
                    else {
                        print_error("Error: argument is not required\n");
                    }
                }
                else {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) == NULL) {
                        print_error("Error: failed to get current working directory\n");
                    }
                    else {
                        write(1, "'", 1);
                        write(1, cwd, strlen(cwd));
                        write(1, "' is current directory\n", strlen("' is current directory\n"));
                    }
                }
            }
            else if (strcmp(cmd, "QUIT") == 0) { // terminate server program
                char* extra = strtok(NULL, " ");

                if (extra != NULL) {
                    if (extra[0] == '-') {
                        print_error("Error: invalid option\n");
                    }
                    else {
                        print_error("Error: argument is not required\n");
                    }
                }
                else {
                    write(1, "Quit success\n", strlen("Quit success\n"));
                    return 0;
                }
            }
            else if (strcmp(cmd, "CWD") == 0) { // change working directory
                char* arg = strtok(NULL, " ");
                char* extra = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else if (extra != NULL) {
                    print_error("Error: argument is not required\n");
                }
                else {
                    if (chdir(arg) != 0) {
                        print_error("Error: failed to change directory\n");
                    }
                    else {
                        char cwd[1024];
                        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                            print_error("Error: failed to get current working directory\n");
                        }
                        else {
                            write(1, "'", 1);
                            write(1, cwd, strlen(cwd));
                            write(1, "' is current directory\n", strlen("' is current directory\n"));
                        }
                    }
                }
            }
            else if (strcmp(cmd, "CDUP") == 0) { // move to parent directory
                char* extra = strtok(NULL, " ");

                if (extra != NULL) {
                    if (extra[0] == '-') {
                        print_error("Error: invalid option\n");
                    }
                    else {
                        print_error("Error: argument is not required\n");
                    }
                }
                else {
                    if (chdir("..") != 0) {
                        print_error("Error: failed to change directory\n");
                    }
                    else {
                        char cwd[1024];
                        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                            print_error("Error: failed to get current working directory\n");
                        }
                        else {
                            write(1, "'", 1);
                            write(1, cwd, strlen(cwd));
                            write(1, "' is current directory\n", strlen("' is current directory\n"));
                        }
                    }
                }
            }
            else if (strcmp(cmd, "MKD") == 0) { // create one or more directories
                char* arg = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else {
                    while (arg != NULL) {
                        if (arg[0] == '-') {
                            print_error("Error: invalid option\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        if (mkdir(arg, 0755) != 0) {
                            print_error("Error: failed to create directory\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        arg = strtok(NULL, " ");
                    }
                }
            }
            else if (strcmp(cmd, "DELE") == 0) { // delete one or more files
                char* arg = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else {
                    while (arg != NULL) {
                        if (arg[0] == '-') {
                            print_error("Error: invalid option\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        if (remove(arg) != 0) {
                            print_error("Error: failed to delete file\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        arg = strtok(NULL, " ");
                    }
                }
            }
            else if (strcmp(cmd, "RMD") == 0) { // remove one or more directories
                char* arg = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else {
                    while (arg != NULL) {
                        if (arg[0] == '-') {
                            print_error("Error: invalid option\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        if (rmdir(arg) != 0) {
                            print_error("Error: failed to delete directory\n");
                            arg = strtok(NULL, " ");
                            continue;
                        }

                        arg = strtok(NULL, " ");
                    }
                }
            }
            else if (strcmp(cmd, "RNFR") == 0) { // select file or directory to rename
                char* arg = strtok(NULL, " ");
                char* extra = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else if (extra != NULL) {
                    print_error("Error: argument is not required\n");
                }
                else {
                    struct stat st;
                    if (stat(arg, &st) != 0) {
                        print_error("Error: failed to access file or directory\n");
                    }
                    else {
                        strcpy(rename_from, arg); // store rename source
                        rnfr_ready = 1; // mark RNFR state ready
                    }
                }
            }
            else if (strcmp(cmd, "RNTO") == 0) { // rename selected file or directory
                char* arg = strtok(NULL, " ");
                char* extra = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else if (extra != NULL) {
                    print_error("Error: argument is not required\n");
                }
                else if (rnfr_ready == 0) {
                    print_error("Error: no file or directory selected for rename\n");
                }
                else {
                    if (rename(rename_from, arg) != 0) {
                        print_error("Error: failed to rename file or directory\n");
                    }
                    else {
                        rnfr_ready = 0; // clear rename state after success
                    }
                }
            }
            else if (strcmp(cmd, "NLST") == 0) { // handle NLST command with optional -a/-l and path
                char* t1 = strtok(NULL, " ");
                char* t2 = strtok(NULL, " ");
                char* t3 = strtok(NULL, " ");

                int aflag = 0;
                int lflag = 0;
                char* path = ".";

                if (t1 == NULL) {
                    path = ".";
                }
                else if (t1[0] == '-') {
                    if (strcmp(t1, "-a") == 0) {
                        aflag = 1;
                    }
                    else if (strcmp(t1, "-l") == 0) {
                        lflag = 1;
                    }
                    else if (strcmp(t1, "-al") == 0 || strcmp(t1, "-la") == 0) {
                        aflag = 1;
                        lflag = 1;
                    }
                    else {
                        print_error("Error: invalid option\n");
                        goto next_line;
                    }

                    if (t2 != NULL) {
                        if (t2[0] == '-') {
                            print_error("Error: invalid option\n");
                            goto next_line;
                        }
                        path = t2;
                    }

                    if (t3 != NULL) {
                        print_error("Error: argument is not required\n");
                        goto next_line;
                    }
                }
                else {
                    path = t1;

                    if (t2 != NULL) {
                        print_error("Error: argument is not required\n");
                        goto next_line;
                    }
                }

                handle_nlst_like(aflag, lflag, path); // execute listing
            }
            else if (strcmp(cmd, "LIST") == 0) { // handle LIST command in long format including hidden entries
                char* t1 = strtok(NULL, " ");
                char* t2 = strtok(NULL, " ");
                char* path = ".";

                if (t1 == NULL) {
                    path = ".";
                }
                else {
                    if (t1[0] == '-') {
                        print_error("Error: invalid option\n");
                        goto next_line;
                    }
                    path = t1;
                }

                if (t2 != NULL) {
                    print_error("Error: argument is not required\n");
                    goto next_line;
                }

                handle_nlst_like(1, 1, path); // LIST behaves like -a -l
            }
            else if (strcmp(cmd, "RETR") == 0) { // copy file from server_root to client_root
                char* arg = strtok(NULL, " ");
                char* extra = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else if (extra != NULL) {
                    print_error("Error: argument is not required\n");
                }
                else {
                    char src[1024];
                    char dst[1024];
                    snprintf(src, sizeof(src), "server_root/%s", arg); // source path on server
                    snprintf(dst, sizeof(dst), "client_root/%s", arg); // destination path on client

                    struct stat st;
                    if (stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
                        char err_msg[1024];
                        snprintf(err_msg, sizeof(err_msg), "Error: '%s' does not exist in server_root\n", arg);
                        print_error(err_msg);
                    }
                    else if (copy_file(src, dst) != 0) {
                        print_error("Error: failed to retrieve file\n");
                    }
                    else {
                        char ok_msg[1024];
                        snprintf(ok_msg, sizeof(ok_msg), "OK: %ld bytes copied to client_root\n", (long)st.st_size);
                        write_str(ok_msg);
                    }
                }
            }
            else if (strcmp(cmd, "STOR") == 0) { // copy file from client_root to server_root
                char* arg = strtok(NULL, " ");
                char* extra = strtok(NULL, " ");

                if (arg == NULL) {
                    print_error("Error: argument is required\n");
                }
                else if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else if (extra != NULL) {
                    print_error("Error: argument is not required\n");
                }
                else {
                    char src[1024];
                    char dst[1024];
                    snprintf(src, sizeof(src), "client_root/%s", arg); // source path on client
                    snprintf(dst, sizeof(dst), "server_root/%s", arg); // destination path on server

                    struct stat st;
                    if (stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
                        char err_msg[1024];
                        snprintf(err_msg, sizeof(err_msg), "Error: '%s' does not exist in client_root\n", arg);
                        print_error(err_msg);
                    }
                    else if (copy_file(src, dst) != 0) {
                        print_error("Error: failed to store file\n");
                    }
                    else {
                        char ok_msg[1024];
                        snprintf(ok_msg, sizeof(ok_msg), "OK: %ld bytes copied to server_root (expected %ld bytes)\n",
                            (long)st.st_size, (long)st.st_size);
                        write_str(ok_msg);
                    }
                }
            }

        next_line:
            if (end == NULL) { // no more line in buffer
                break;
            }
            start = end + 1; // move to next line
        }
    }

    return 0; // normal termination
}