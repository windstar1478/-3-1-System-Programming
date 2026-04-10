
///////////////////////////////////////////////////////////////////////////////
// File Name : kw2022202094_cli.c                                            //
// Date : 2026/04/10                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : System Programming Assignment #1-3                                //
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


void print_error(const char* message) {
	write(1, message, strlen(message));
}
int main(int argc, char* argv[]) {
	//(input)./cli pwd -> (output) PWD
	//(input)./cli ls -al test_dir -> (output) NLST -al test_dir
	//(input)./cli cd .. -> (output) CDUP
	//(input)./cli mkdir new_dir -> (output) MKD new_dir
	//1. check argc
	//2. check what command is argv[1]
	//3. make FTP command argument according to the rules
	//4. write(1, buffer, strlen(buffer));


	if (argc < 2) {
		print_error("Error: command is required\n");
		return 1;
	}


	if (strcmp(argv[1], "ls") == 0) {
	}
	else if (strcmp(argv[1], "dir") == 0) {
	}
	else if (strcmp(argv[1], "pwd") == 0) { //if command is pwd, then write PWD to stdout
		if (argc == 2) { //if there is only "pwd"
			write(1, "PWD\n", strlen("PWD\n")); //write PWD command to stdout
			return 0; //terminate the program
		}
		else if (argv[2][0] == '-') { //if the first character of the second argument is '-'
			print_error("Error: invalid option\n"); //write error message to stdout
		}
		else { //if the second argument is not an option
			print_error("Error: argument is not required\n"); //write error message to stdout
		}
		return 1; //terminate the program (error)
	}
	else if (strcmp(argv[1], "quit") == 0) {
		if (argc == 2) { //if there is only "quit"
			write(1, "QUIT\n", strlen("QUIT\n")); //write QUIT command to stdout
			return 0; //terminate the program
		}
		else if (argv[2][0] == '-') { //if the first character of the second argument is '-'
			print_error("Error: invalid option\n"); //write error message to stdout
		}
		else { //if the second argument is not an option
			print_error("Error: argument is not required\n"); //write error message to stdout
		}
		return 1; //terminate the program (error)
	}
	else if (strcmp(argv[1], "cd") == 0) {
		if (argc == 2) { //if there is only "cd"
			print_error("Error: argument is required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		else if (argc > 3) { //if there are more than 3 arguments
			print_error("Error: argument is not required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		else if (argv[2][0] == '-') { //if the first character of the second argument is '-'
			print_error("Error: invalid option\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		else if (strcmp(argv[2], "..") == 0) { //if the argument is ".."
			write(1, "CDUP\n", strlen("CDUP\n")); //write CDUP to stdout
			return 0; //terminate the program
		}
		else {
			write(1, "CWD ", strlen("CWD ")); //write CWD to stdout
			write(1, argv[2], strlen(argv[2])); //write the argument to stdout
			write(1, "\n", strlen("\n")); //write newline to stdout	
			return 0; //terminate the program
		}
	}
	else if (strcmp(argv[1], "mkdir") == 0) {
		if (argc == 2) { //if there is only "mkdir"
			print_error("Error: argument is required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		for (int i = 2; i < argc; i++) { //for each argument
			if (argv[i][0] == '-') { //if the first character of the argument is '-'
				print_error("Error: invalid option\n"); //write error message to stdout
				return 1; //terminate the program (error)
			}
		}
		for (int i = 2; i < argc; i++){ // for each argument
			write(1, "MKD ", strlen("MKD ")); //write MKD to stdout
			write(1, argv[i], strlen(argv[i])); //write the argument to stdout
			write(1, "\n", strlen("\n")); //write newline to stdout
		}

		return 0; //terminate the program
	}
	else if (strcmp(argv[1], "delete") == 0) {
		if (argc == 2) { //if there is only "delete"
			print_error("Error: argument is required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		for (int i = 2; i < argc; i++) { //for each argument
			if (argv[i][0] == '-') { //if the first character of the argument is '-'
				print_error("Error: invalid option\n"); //write error message to stdout
				return 1; //terminate the program (error)
			}
		}
		for (int i = 2; i < argc; i++){ // for each argument
			write(1, "DELE ", strlen("DELE ")); //write DELE to stdout
			write(1, argv[i], strlen(argv[i])); //write the argument to stdout
			write(1, "\n", strlen("\n")); //write newline to stdout
		}

		return 0; //terminate the program
	}
	else {
		// unknown command
	}
}