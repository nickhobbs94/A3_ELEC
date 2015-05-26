/*
	This is where the main function for every question for assessment 2 will be. Along with some general functions
	that will be used to communicate via putty.
*/

/* Includes */
//#include "home.h"
#include "uni.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "terminalParse.h"
#include "alt_types.h"
#include "SD_functions.h"

/* Magic numbers */



/* ----------------------------------- Functions ----------------------------------- */

int main() {
	  //LCD_Init();

	while(1){
		alt_8 string[PUTTY_LINE_LENGTH];
		alt_32 stringlength = puttyGetline(string,PUTTY_LINE_LENGTH);

		if (stringlength < 0){
			puttyPrintLine("\n\rYou have entered too many characters for that command\n\r");
		}

		alt_8** array_of_words;
		array_of_words = malloc(STRING_PARSER_MAXNUM_WORDS(stringlength));
		alt_8 numwords = string_parser(string,array_of_words);
		alt_32 returncode = command_interpreter(numwords, array_of_words);

		if (returncode<0 && numwords > 0){
			puttyPrintLine("Command not found\n\r");
		}

		free(array_of_words);
	}
	return 0;
}



