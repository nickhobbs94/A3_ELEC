/*
terminalParse.h
*/
#ifndef TERMINALPARSE_H
#define TERMINALPARSE_H

/* Includes */
#include <stdio.h>
#include "altstring.h"
#include <stdlib.h>
#include "terminalFunctions.h"
#include "alt_types.h"

/* Magic numbers */
#define STRING_PARSER_MAXNUM_WORDS(string) string/2+1

/* Function prototypes */
alt_8 string_parser(alt_8* string, alt_8* array_of_words[]);
alt_32 command_interpreter(alt_32 argc, alt_8* argv[]);
alt_8 split_string(alt_8* string, alt_8* array_of_words[], alt_8 character);

/* ----------------------------------- Functions ----------------------------------- */

alt_32 command_interpreter(alt_32 argc, alt_8* argv[]){
	struct terminal_functions {
		char* command_string;
		alt_32 (*command_function)(alt_32 argc, alt_8* argv[]);
	} terminal_commands[] = {
		/* 
		Enter new terminal funtions below. 
		SYNTAX: {"terminalcommand", function_name} 
		*/
		{"echo",echo}, 
		{"add",add}, 
		{"ledr",ledr}, 
		{"switch",switch_function}, 
		{"mount",tf_mount},
		{"unmount",tf_unmount},
		{"ls",ls_path},
		{"cd",change_dir},
		{"mkdir",make_directory},
		{"rm",delete_file},
		{"touch",write_new_file},
		{"cp",copy_file},
		{"cat",read_file},
		{"wav",wav_play},
		{NULL,NULL} // This null function is to check we've read all the functions, new functions must go above it.
	};
	
	int i;
	/* loops through the terminal commands and compares to the input arg */
	for (i=0; terminal_commands[i].command_string != NULL; i++) {
		if (altstrcmp(terminal_commands[i].command_string, argv[0]) == 0){
			terminal_commands[i].command_function(argc, argv);
			return 0;
		}
	}
	return -1;
}


/*
USAGE: alt_8 string_parser(alt_8* string, alt_8* array_of_words[]);
GOTCHAS: the size of array_of_words should be malloc'd to be able to hold all the words, use at your own risk
SUGGESTION: use STRING_PARSER_MAXNUM_WORDS(string) to return the suggested size of array_of_words
*/
alt_8 string_parser(alt_8* string, alt_8* array_of_words[]){
	return altsplitstring(string, array_of_words[], ' ');
}


#endif

