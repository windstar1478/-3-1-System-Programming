
///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c														 //
// Date : 2026/04/18                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : System Programming Assignment #1-3                                //
// Description: This program is a command line interface (CLI) for a simple  //
// FTP client. It takes a command and its arguments as input and outputs the //
// corresponding FTP command to stdout. It also handles errors and invalid	 //
// options. The supported commands are										 //
// ls, dir, pwd, quit, cd, mkdir, delete, rmdir, and rename.			     //
///////////////////////////////////////////////////////////////////////////////


#include <unistd.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////
// print_error																		 //
// ==================================================================================//
// Input: const char* -> error message												 //
// Output: none																		 //
// Purpose: print error message to standard error									 //
///////////////////////////////////////////////////////////////////////////////////////
void print_error(const char* message) { // function to print error message to stderr
	write(2, message, strlen(message)); // write the error message to stderr (file descriptor 2)
} 
///////////////////////////////////////////////////////////////////////////////////////
// main																				 //
// ==================================================================================//
// Input: int argc -> number of arguments											 //
//        char* argv[] -> argument vector											 //
// Output: int -> 0 success, 1 fail												     //
// Purpose: convert user commands to FTP commands and write them to standard output  //
///////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
	if (argc < 2) { // if there is no command, then write error message to stdout
		print_error("Error: command is required\n"); // write error message to stdout
		return 1; // terminate the program (error)
	}
	else if (strcmp(argv[1], "ls") == 0) { 
		if (argc == 2) { // if there is only "ls"
			write(1, "NLST\n", strlen("NLST\n")); // write NLST command to stdout
			return 0; // terminate the program
		}
		else if (argc == 3) { // if there are two arguments
			if (strcmp(argv[2], "-a") == 0 || strcmp(argv[2], "-l") == 0 || strcmp(argv[2], "-al") == 0) { // if the second argument is an option
				write(1, "NLST ", strlen("NLST ")); // write NLST to stdout
				write(1, argv[2], strlen(argv[2])); // write the option to stdout
				write(1, "\n", 1); // write newline to stdout
				return 0; //terminate the program
			}
			else if (argv[2][0] == '-') { // if the first character of the second argument is '-'
				print_error("Error: invalid option\n"); // write error message to stdout
				return 1; // terminate the program (error)
			}
			else { // if the second argument is not an option
				write(1, "NLST ", strlen("NLST ")); // write NLST to stdout
				write(1, argv[2], strlen(argv[2])); // write the argument to stdout
				write(1, "\n", 1); // write newline to stdout
				return 0; // terminate the program
			}
		}
		else if (argc == 4) { // if there are three arguments
			if (strcmp(argv[2], "-a") == 0 || strcmp(argv[2], "-l") == 0 || strcmp(argv[2], "-al") == 0) { // if the second argument is an option
				write(1, "NLST ", strlen("NLST ")); // write NLST to stdout
				write(1, argv[2], strlen(argv[2])); // write the option to stdout
				write(1, " ", 1); // write space to stdout
				write(1, argv[3], strlen(argv[3])); // write the argument to stdout
				write(1, "\n", 1); // write newline to stdout
				return 0; // terminate the program
			}
			else { // if the second argument is not an option
				print_error("Error: invalid option\n"); // write error message to stdout
				return 1; // terminate the program (error)
			}
		}
		else { // if there are more than three arguments
			print_error("Error: argument is not required\n"); // write error message to stdout
			return 1; // terminate the program (error)
		}
	}
	else if (strcmp(argv[1], "dir") == 0) { 
		if (argc == 2) { // if there is only "dir"
			write(1, "LIST\n", strlen("LIST\n")); // write LIST command to stdout
			return 0; // terminate the program
		}
		else if (argc == 3) { // if there are two arguments
			if (argv[2][0] == '-') { // if the first character of the second argument is '-'
				print_error("Error: invalid option\n"); // write error message to stdout
				return 1; // terminate the program (error)
			}
			else { // if the second argument is not an option
				write(1, "LIST ", strlen("LIST ")); // write LIST to stdout
				write(1, argv[2], strlen(argv[2])); // write the argument to stdout
				write(1, "\n", 1); // write newline to stdout
				return 0; // terminate the program
			}
		}
		else { // if there are more than three arguments
			print_error("Error: argument is not required\n"); // write error message to stdout
			return 1; // terminate the program (error)
		}
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
		for (int i = 2; i < argc; i++) { // for each argument
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
		for (int i = 2; i < argc; i++) { // for each argument
			write(1, "DELE ", strlen("DELE ")); //write DELE to stdout
			write(1, argv[i], strlen(argv[i])); //write the argument to stdout
			write(1, "\n", strlen("\n")); //write newline to stdout
		}

		return 0; //terminate the program
	}
	else if (strcmp(argv[1], "rmdir") == 0) {
		if (argc == 2) { //if there is only "rmdir"
			print_error("Error: argument is required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		for (int i = 2; i < argc; i++) { //for each argument
			if (argv[i][0] == '-') { //if the first character of the argument is '-'
				print_error("Error: invalid option\n"); //write error message to stdout
				return 1; //terminate the program (error)
			}
		}
		for (int i = 2; i < argc; i++) { // for each argument
			write(1, "RMD ", strlen("RMD ")); //write RMD to stdout
			write(1, argv[i], strlen(argv[i])); //write the argument to stdout
			write(1, "\n", strlen("\n")); //write newline to stdout
		}

		return 0; //terminate the program
	}
	else if (strcmp(argv[1], "rename") == 0) {
		char buffer[1024]; //buffer to store the command

		if (argc != 4) { //if there are not exactly 4 arguments
			print_error("Error: two arguments are required\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		else if (argv[2][0] == '-' || argv[3][0] == '-') { //if the first character of the second or third argument is '-'
			print_error("Error: invalid option\n"); //write error message to stdout
			return 1; //terminate the program (error)
		}
		else {
			snprintf(buffer, sizeof(buffer), "RNFR     %s\nRNTO   %s\n", argv[2], argv[3]); //format the command and store it in buffer
			write(1, buffer, strlen(buffer)); //write the command to stdout
			return 0; //terminate the program
		}
	}
	else if (strcmp(argv[1], "get") == 0) {
		if (argc == 2) { //if there is only "get"
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
		else {
			write(1, "RETR ", strlen("RETR ")); //write RETR to stdout
			write(1, argv[2], strlen(argv[2])); //write the argument to stdout
			write(1, "\n", 1); //write newline to stdout	
			return 0; //terminate the program
		}
	}
	else if (strcmp(argv[1], "put") == 0) {
		if (argc == 2) { //if there is only "put"
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
		else {
			write(1, "STOR ", strlen("STOR ")); //write STOR to stdout
			write(1, argv[2], strlen(argv[2])); //write the argument to stdout
			write(1, "\n", 1); //write newline to stdout	
			return 0; //terminate the program
		}
		}

	else {
		print_error("Error: unknown command\n"); //write error message to stdout
		return 1; //terminate the program (error)
	}
}