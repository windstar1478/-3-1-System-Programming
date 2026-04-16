///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c                                                         //
// Date : 2026/04/18                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : FTP Server srv program                                            //
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

void print_error(const char* message) {
    write(2, message, strlen(message));
}

int cmpstr(const void* a, const void* b) {
    return strcmp((const char*)a, (const char*)b);
}

void write_str(const char* s) {
    write(1, s, strlen(s));
}

int is_hidden_name(const char* name) {
    return name[0] == '.';
}

void make_full_path(char* dest, size_t size, const char* dir, const char* name) {
    if (strcmp(dir, ".") == 0) {
        snprintf(dest, size, "%s", name);
    }
    else {
        snprintf(dest, size, "%s/%s", dir, name);
    }
}

void format_mode_string(mode_t mode, char* out) {
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
    out[10] = '\0';
}

int copy_file(const char* src, const char* dst) {
    int fd_src = open(src, O_RDONLY);
    if (fd_src < 0) return -1;

    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dst < 0) {
        close(fd_src);
        return -1;
    }

    char buf[BUF_SIZE];
    ssize_t nread;
    while ((nread = read(fd_src, buf, sizeof(buf))) > 0) {
        ssize_t total = 0;
        while (total < nread) {
            ssize_t nwritten = write(fd_dst, buf + total, nread - total);
            if (nwritten < 0) {
                close(fd_src);
                close(fd_dst);
                return -1;
            }
            total += nwritten;
        }
    }

    close(fd_src);
    close(fd_dst);

    return (nread < 0) ? -1 : 0;
}

int handle_nlst_like(int aflag, int lflag, const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        print_error("Error: failed to access file or directory\n");
        return 1;
    }

    if (S_ISDIR(st.st_mode)) {
        DIR* dp = opendir(path);
        struct dirent* dirp;

        char names[MAX_ENTRIES][MAX_NAME_LEN];
        int count = 0;

        if (dp == NULL) {
            print_error("Error: failed to open directory\n");
            return 1;
        }

        while ((dirp = readdir(dp)) != NULL) {
            if (!aflag && is_hidden_name(dirp->d_name)) {
                continue;
            }

            if (count >= MAX_ENTRIES) {
                break;
            }

            char filepath[MAX_NAME_LEN];
            struct stat entry_st;

            make_full_path(filepath, sizeof(filepath), path, dirp->d_name);

            if (stat(filepath, &entry_st) != 0) {
                closedir(dp);
                print_error("Error: failed to access file or directory\n");
                return 1;
            }

            strcpy(names[count], dirp->d_name);

            if (S_ISDIR(entry_st.st_mode)) {
                strcat(names[count], "/");
            }

            count++;
        }

        closedir(dp);

        qsort(names, count, sizeof(names[0]), cmpstr);

        if (!lflag) {
            for (int i = 0; i < count; i++) {
                write(1, names[i], strlen(names[i]));
                if ((i + 1) % 5 == 0 || i == count - 1) {
                    write(1, "\n", 1);
                }
                else {
                    write(1, "\t", 1);
                }
            }
        }
        else {
            for (int i = 0; i < count; i++) {
                char original_name[MAX_NAME_LEN];
                strcpy(original_name, names[i]);

                size_t len = strlen(original_name);
                if (len > 0 && original_name[len - 1] == '/') {
                    original_name[len - 1] = '\0';
                }

                char filepath[MAX_NAME_LEN];
                make_full_path(filepath, sizeof(filepath), path, original_name);

                struct stat entry_st;
                if (stat(filepath, &entry_st) != 0) {
                    print_error("Error: failed to access file or directory\n");
                    return 1;
                }

                char mode_str[11];
                format_mode_string(entry_st.st_mode, mode_str);

                struct tm* tm_info = localtime(&entry_st.st_mtime);
                char time_buf[256];
                strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);

                struct passwd* pw = getpwuid(entry_st.st_uid);
                struct group* gr = getgrgid(entry_st.st_gid);

                char line[2048];
                snprintf(line, sizeof(line), "%s %lu %s %s %5ld %s %s\n",
                    mode_str,
                    (unsigned long)entry_st.st_nlink,
                    pw ? pw->pw_name : "unknown",
                    gr ? gr->gr_name : "unknown",
                    (long)entry_st.st_size,
                    time_buf,
                    names[i]);

                write_str(line);
            }
        }
    }
    else {
        char* filename = strrchr(path, '/');
        if (filename == NULL) filename = (char*)path;
        else filename++;

        if (!aflag && is_hidden_name(filename)) {
            return 0;
        }

        if (!lflag) {
            write(1, filename, strlen(filename));
            write(1, "\n", 1);
        }
        else {
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

            write_str(line);
        }
    }

    return 0;
}

int main(void) {
    char rename_from[1024];
    int rnfr_ready = 0;

    while (1) {
        char buffer[1024];
        int n = read(0, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';

        char* cmd = strtok(buffer, " \n");
        if (cmd == NULL) {
            continue;
        }

        if (strcmp(cmd, "PWD") == 0) {
            char* extra = strtok(NULL, " \n");

            if (extra != NULL) {
                if (extra[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else {
                    print_error("Error: argument is not required\n");
                }
                continue;
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                print_error("Error: failed to get current working directory\n");
                continue;
            }

            write(1, "'", 1);
            write(1, cwd, strlen(cwd));
            write(1, "' is current directory\n", strlen("' is current directory\n"));

        }
        else if (strcmp(cmd, "QUIT") == 0) {
            char* extra = strtok(NULL, " \n");

            if (extra != NULL) {
                if (extra[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else {
                    print_error("Error: argument is not required\n");
                }
                continue;
            }
            write(1, "Quit success\n", strlen("Quit success\n"));
            break;
        }
        else if (strcmp(cmd, "CWD") == 0) {
            char* arg = strtok(NULL, " \n");
            char* extra = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            else if (arg[0] == '-') {
                print_error("Error: invalid option\n");
                continue;
            }
            else if (extra != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }
            else {
                if (chdir(arg) != 0) {
                    print_error("Error: failed to change directory\n");
                    continue;
                }

                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    print_error("Error: failed to get current working directory\n");
                    continue;
                }

                write(1, "'", 1);
                write(1, cwd, strlen(cwd));
                write(1, "' is current directory\n", strlen("' is current directory\n"));
            }
        }
        else if (strcmp(cmd, "CDUP") == 0) {
            char* extra = strtok(NULL, " \n");

            if (extra != NULL) {
                if (extra[0] == '-') {
                    print_error("Error: invalid option\n");
                }
                else {
                    print_error("Error: argument is not required\n");
                }
                continue;
            }
            if (chdir("..") != 0) {
                print_error("Error: failed to change directory\n");
                continue;
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                print_error("Error: failed to get current working directory\n");
                continue;
            }
            write(1, "'", 1);
            write(1, cwd, strlen(cwd));
            write(1, "' is current directory\n", strlen("' is current directory\n"));
        }
        else if (strcmp(cmd, "MKD") == 0) {
            char* arg = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            while (arg != NULL) {
                if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                    continue;
                }
                if (mkdir(arg, 0755) != 0) {
                    print_error("Error: failed to create directory\n");
                    continue;
                }
                arg = strtok(NULL, " \n");
            }
        }
        else if (strcmp(cmd, "DELE") == 0) {
            char* arg = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            while (arg != NULL) {
                if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                    continue;
                }
                if (remove(arg) != 0) {
                    print_error("Error: failed to delete file\n");
                    continue;
                }
                arg = strtok(NULL, " \n");
            }
        }
        else if (strcmp(cmd, "RMD") == 0) {
            char* arg = strtok(NULL, " \n");
            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            while (arg != NULL) {
                if (arg[0] == '-') {
                    print_error("Error: invalid option\n");
                    continue;
                }
                if (rmdir(arg) != 0) {
                    print_error("Error: failed to delete directory\n");
                    continue;
                }
                arg = strtok(NULL, " \n");
            }
        }
        else if (strcmp(cmd, "RNFR") == 0) {
            char* arg = strtok(NULL, " \n");
            char* extra = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            else if (arg[0] == '-') {
                print_error("Error: invalid option\n");
                continue;
            }
            else if (extra != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }
            else {
                struct stat st;
                if (stat(arg, &st) != 0) {
                    print_error("Error: failed to access file or directory\n");
                    continue;
                }
                strcpy(rename_from, arg);
                rnfr_ready = 1;
            }
        }
        else if (strcmp(cmd, "RNTO") == 0) {
            char* arg = strtok(NULL, " \n");
            char* extra = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            else if (arg[0] == '-') {
                print_error("Error: invalid option\n");
                continue;
            }
            else if (extra != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }
            else if (rnfr_ready == 0) {
                print_error("Error: no file or directory selected for rename\n");
                continue;
            }
            else {
                if (rename(rename_from, arg) != 0) {
                    print_error("Error: failed to rename file or directory\n");
                    continue;
                }
                rnfr_ready = 0;
            }
        }
        else if (strcmp(cmd, "NLST") == 0) {
            char* t1 = strtok(NULL, " \n");
            char* t2 = strtok(NULL, " \n");
            char* t3 = strtok(NULL, " \n");

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
                    continue;
                }

                if (t2 != NULL) {
                    if (t2[0] == '-') {
                        print_error("Error: invalid option\n");
                        continue;
                    }
                    path = t2;
                }

                if (t3 != NULL) {
                    print_error("Error: argument is not required\n");
                    continue;
                }
            }
            else {
                path = t1;

                if (t2 != NULL) {
                    print_error("Error: argument is not required\n");
                    continue;
                }
            }

            if (handle_nlst_like(aflag, lflag, path) != 0) {
                continue;
            }
        }
        else if (strcmp(cmd, "LIST") == 0) {
            char* t1 = strtok(NULL, " \n");
            char* t2 = strtok(NULL, " \n");
            char* path = ".";

            if (t1 == NULL) {
                path = ".";
            }
            else {
                if (t1[0] == '-') {
                    print_error("Error: invalid option\n");
                    continue;
                }
                path = t1;
            }

            if (t2 != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }

            if (handle_nlst_like(1, 1, path) != 0) {
                continue;
            }
        }
        else if (strcmp(cmd, "RETR") == 0) {
            char* arg = strtok(NULL, " \n");
            char* extra = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            else if (arg[0] == '-') {
                print_error("Error: invalid option\n");
                continue;
            }
            else if (extra != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }

            char src[1024];
            char dst[1024];
            snprintf(src, sizeof(src), "server_root/%s", arg);
            snprintf(dst, sizeof(dst), "client_root/%s", arg);

            struct stat st;
            if (stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
                char err_msg[1024];
                snprintf(err_msg, sizeof(err_msg), "Error: '%s' does not exist in server_root\n", arg);
                print_error(err_msg);
                continue;
            }

            if (copy_file(src, dst) != 0) {
                print_error("Error: failed to retrieve file\n");
                continue;
            }

            char ok_msg[1024];
            snprintf(ok_msg, sizeof(ok_msg), "OK: %ld bytes copied to client_root\n", (long)st.st_size);
            write_str(ok_msg);
        }
        else if (strcmp(cmd, "STOR") == 0) {
            char* arg = strtok(NULL, " \n");
            char* extra = strtok(NULL, " \n");

            if (arg == NULL) {
                print_error("Error: argument is required\n");
                continue;
            }
            else if (arg[0] == '-') {
                print_error("Error: invalid option\n");
                continue;
            }
            else if (extra != NULL) {
                print_error("Error: argument is not required\n");
                continue;
            }

            char src[1024];
            char dst[1024];
            snprintf(src, sizeof(src), "client_root/%s", arg);
            snprintf(dst, sizeof(dst), "server_root/%s", arg);

            struct stat st;
            if (stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
                char err_msg[1024];
                snprintf(err_msg, sizeof(err_msg), "Error: '%s' does not exist in client_root\n", arg);
                print_error(err_msg);
                continue;
            }

            if (copy_file(src, dst) != 0) {
                print_error("Error: failed to store file\n");
                continue;
            }

            char ok_msg[1024];
            snprintf(ok_msg, sizeof(ok_msg), "OK: %ld bytes copied to server_root (expected %ld bytes)\n", (long)st.st_size, (long)st.st_size);
            write_str(ok_msg);
        }
    }

    return 0;
}