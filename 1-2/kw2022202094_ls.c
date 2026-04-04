///////////////////////////////////////////////////////////////////////////////
// File Name : kw2022202094_ls.c                                             //
// Date : 2026/04/03                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : System Programming Assignment #1-2                                //
// Description : This program displays file names in a directory using       //
// opendir(), readdir(), and closedir(), and handles simple error cases such //
// as invalid path and access denial.										 //
///////////////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) { // argc: number of command-line arguments, argv: argument vector
DIR *dp; //directory stream pointer (store the value of opendir())
struct dirent *dirp; //pointer that indicates inner directory (readdir())


char* path = NULL;
if (argc == 1) { //current directory
	path = ".";
}
else if (argc == 2) { //received directory 
	path = argv[1];
}
else { //more than one directory
	printf("only one directory path can be processed\n");
	return 1; //terminate program (error)
}

dp = opendir(path); //if failed, handle error by errno


if (dp == NULL) { //directory is unavailable
	printf("%s: cannot access '%s' : ", argv[0], path);
	if (errno == ENOENT) { //path does not exist
		printf("No such directory\n"); 
		return 1; //terminate program (error)
	}
	else if (errno == EACCES) { //permission denied
		printf("Access denied\n");
		return 1;
	}
	else if (errno == ENOTDIR) { //path is not directory
		printf("No such directory\n");
		return 1; //terminate program (error)
	}
	else { //the other cases
		printf("operation failed\n");
		return 1;
	}
}
else {
	while ((dirp = readdir(dp)) != NULL) { //read each entry in directory
		printf("%s\n", dirp->d_name);
	}
	closedir(dp); //close directory stream before program end
}

return 0;

}