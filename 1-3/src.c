///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c														 //
// Date : 2026/04/18                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : 																	 //
///////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void print_error(const char* message) { // function to print error message to stderr
	write(2, message, strlen(message)); // write the error message to stderr (file descriptor 2)
}

int main(void) {
	while (1) {
		char buffer[1024]; // buffer to store the command
		int n = read(0, buffer, sizeof(buffer) - 1); // read command from stdin (file descriptor 0)
		if (n <= 0) { // if there is an error or end of file
			break; // exit the loop
		}
		buffer[n] = '\0'; // null-terminate the buffer
		
		char* cmd = strtok(buffer, " \n"); // get the command (first token)
		if (cmd == NULL) { // if there is no command
			continue; // skip to the next iteration of the loop
		}


		if (strcmp(cmd, "PWD") == 0) {
			char* extra = strtok(NULL, " \n"); //get the next token

			if (extra != NULL) { //if there are more tokens after the command
				if (extra[0] == '-' ) { //if there is another token after the next token
					print_error("Error: invalid option\n"); //write error message to stderr
				}
				else {
					print_error("Error: argument is not required\n"); //write error message to stderr
				}
				return 1; //terminate the program (error)

			}
			char cwd[1024]; // buffer to store the current working directory
			if (getcwd(cwd, sizeof(cwd)) == NULL) { //get the current working directory
				print_error("Error: failed to get current working directory\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}

			write(1, "'", 1); //write single quote to stdout
			write(1, cwd, strlen(cwd)); //write the current working directory to stdout
			write(1, "' is current directory\n", strlen("' is current directory\n")); //write the current working directory to stdout
			
		}
		else if (strcmp(cmd, "QUIT") == 0) {
			char* extra = strtok(NULL, " \n"); //get the next token

			if (extra != NULL) {
				if (extra[0] == '-') { //if there is another token after the next token
					print_error("Error: invalid option\n"); //write error message to stderr
				}
				else {
					print_error("Error: argument is not required\n"); //write error message to stderr)
				}
				return 1; //terminate the program (error)
			}
			write(1, "Quit success\n", strlen("Quit success\n")); //write quit success message to stdout
			break; //exit the loop
		}
		else if (strcmp(cmd, "CWD") == 0) {
			char* arg = strtok(NULL, " \n"); //get the next token
			char* extra = strtok(NULL, " \n"); //get the next token

			if (arg == NULL){ //if there is no argument
				print_error("Error: argument is required\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else if (arg[0] == '-') { //if the first character of the argument is '-'
				print_error("Error: invalid option\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else if (extra != NULL) { //if there are more tokens after the argument
				print_error("Error: argument is not required\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else {
				if (chdir(arg) != 0) { //change the current working directory to the argument
					print_error("Error: failed to change directory\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}

				char cwd[1024]; // buffer to store the current working directory
				if (getcwd(cwd, sizeof(cwd)) == NULL) { //get the current working directory
					print_error("Error: failed to get current working directory\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}

				write(1, "'", 1); //write single quote to stdout
				write(1, cwd, strlen(cwd)); //write the current working directory to stdout
				write(1, "' is current directory\n", strlen("' is current directory\n")); //write the current working directory to stdout

			}
		}
		else if (strcmp(cmd, "CDUP") == 0) {
			char* extra = strtok(NULL, " \n"); //get the next token

			if (extra != NULL) {
				if (extra[0] == '-') { //if there is another token after the next token
					print_error("Error: invalid option\n"); //write error message to stderr
				}
				else {
					print_error("Error: argument is not required\n"); //write error message to stderr
				}
				return 1; //terminate the program (error)
			}
			if (chdir("..") != 0) { //change the current working directory to the parent directory
				print_error("Error: failed to change directory\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			char cwd[1024]; // buffer to store the current working directory
			if (getcwd(cwd, sizeof(cwd)) == NULL) { //get the current working directory
				print_error("Error: failed to get current working directory\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			write(1, "'", 1); //write single quote to stdout
			write(1, cwd, strlen(cwd)); //write the current working directory to stdout
			write(1, "' is current directory\n", strlen("' is current directory\n")); //write the current working directory to stdout

		}
		else if (strcmp(cmd, "MKD") == 0) {
			char* arg = strtok(NULL, " \n"); //get the next token

			if (arg == NULL){ //if there is no argument
				print_error("Error: argument is required\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			while (arg != NULL) { //while there are more tokens
				if (arg[0] == '-') { //if the first character of the argument is '-'
					print_error("Error: invalid option\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}
				if (mkdir(arg, 0755) != 0) { //create a directory with the argument as the name
					print_error("Error: failed to create directory\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}
				arg = strtok(NULL, " \n"); //get the next token
			}

		}
		else if (strcmp(cmd, "DELE") == 0) {
				char* arg = strtok(NULL, " \n"); //get the next token

				if (arg == NULL){ //if there is no argument
					print_error("Error: argument is required\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}
				while (arg != NULL) { //while there are more tokens
					if (arg[0] == '-') { //if the first character of the argument is '-'
						print_error("Error: invalid option\n"); //write error message to stderr
						return 1; //terminate the program (error)
					}
					if (remove(arg) != 0) { //delete the file with the argument as the name
						print_error("Error: failed to delete file\n"); //write error message to stderr
						return 1; //terminate the program (error)
					}
					arg = strtok(NULL, " \n"); //get the next token
				}
			}
		else if (strcmp(cmd, "RMD") == 0) {
				char* arg = strtok(NULL, " \n"); //get the next token
				if (arg == NULL){ //if there is no argument
					print_error("Error: argument is required\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}
				while (arg != NULL) { //while there are more tokens
					if (arg[0] == '-') { //if the first character of the argument is '-'
						print_error("Error: invalid option\n"); //write error message to stderr
						return 1; //terminate the program (error)
					}
					if (rmdir(arg) != 0) { //delete the directory with the argument as the name
						print_error("Error: failed to delete directory\n"); //write error message to stderr
						return 1; //terminate the program (error)
					}
					arg = strtok(NULL, " \n"); //get the next token
				}
			}
		else if (strcmp(cmd, "RNFR") == 0) {
			char* arg1 = strtok(NULL, " \n"); //get the next token
			char* arg2 = strtok(NULL, " \n"); //get the next token
			char* extra = strtok(NULL, " \n"); //get the next token

			if (arg1 == NULL || arg2 == NULL) { //if there are not enough arguments
				print_error("Error: two arguments are required\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else if (arg1[0] == '-' || arg2[0] == '-') { //if the first character of the argument is '-'
				print_error("Error: invalid option\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else if (extra != NULL) { //if there are more tokens after the arguments
				print_error("Error: too many arguments\n"); //write error message to stderr
				return 1; //terminate the program (error)
			}
			else {
				if (rename(arg1, arg2) != 0) { //rename the file or directory with the first argument as the name to the second argument as the name
					print_error("Error: failed to rename file or directory\n"); //write error message to stderr
					return 1; //terminate the program (error)
				}
			}
		}
		else {
			print_error("Error: unknown command\n"); //write error message to stderr
			return 1; //terminate the program (error)
		}
	}
	
	return 0; //terminate the program
}