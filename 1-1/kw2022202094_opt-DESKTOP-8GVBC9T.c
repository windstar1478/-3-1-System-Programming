///////////////////////////////////////////////////////////////////////////////
// File Name : kw2022202094_opt.c                                             //
// Date : 2026/03/27                                                         //
// OS : Ubuntu 20.04.6 LTS 64bits                                            //
// Author : LEE WONWOO                                                       //
// Student ID : 2022202094                                                   //
// ------------------------------------------------------------------------- //
// Title : System Programming Assignment #1-1                                //
// Description : This program parses command-line options using getopt().     //
// getopt() is repeatedly called until it returns -1, which means there are  //
// no more options to process. A switch statement handles each option, and   //
// the remaining non-option arguments can be processed after option parsing.  //
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv) { // argc: number of command-line arguments, argv: argument vector
	int aflag = 0, bflag = 0; // counts -a or -b options
	char* cvalue = NULL; // stores the argument value of the -c option
	int index, c; //index for iterating over non-option arguments starting from optind

	opterr = 0; // disables getopt()'s automatic error message output

	while ((c = getopt(argc, argv, "abc:")) != -1)
		/* repeatedly reads options from argv,
		returns -1 when no more options remain.
		'a' and 'b' do not require arguments,
		while 'c' requires an argument value. */
	{
		switch (c) // handle each option returned by getopt()
		{
		case 'a': // when 'a' is identified
			aflag++; // increase counts for option -a
			break;
		case 'b': // when 'b' is identified
			bflag++;  // increase counts for option -b
			break;
		case 'c': // when 'c' is identified
			cvalue = optarg; // store argument of option -c (optarg)
			break;
		default:
			break;
		}
	}
	printf("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue); 

	index = optind;
	while (argv[index] != NULL) { //loop when there are specific arguments
		printf("Non-option argument %s\n", argv[index++]);
	}

	return 0;
}