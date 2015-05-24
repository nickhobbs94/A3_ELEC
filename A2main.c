/*
	This is where the main function for every question for assessment 2 will be. Along with some general functions
	that will be used to communicate via putty.
*/

/* Includes */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "terminalParse.h"
#include "alt_types.h"
#include "LCD_Control.h"
#include "SD_functions.h"

/* Magic numbers */

#define BACKSPACE 0x7f
#define UP 0x41
#define DOWN 0x42
#define LEFT 0x44
#define RIGHT 0x43
#define SPECIAL1 0x1b
#define SPECIAL2 0x5b

/* Function prototypes */
alt_32 puttyGetline(alt_8 string[], alt_32 lineLength);

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

alt_32 puttyGetline(alt_8 string[], alt_32 lineLength){
	alt_32 i=0;
	alt_32 uart_pointer = open("/dev/uart_0", O_RDWR);//, O_NONBLOCK);
	altmemset(string,'\0',lineLength);
	alt_8 charbuffer='\0';
	alt_8 enterPressed=0;
	
	do {
		read(uart_pointer,&charbuffer,1);
		
		/* Echo newlines correctly to putty */
		if (charbuffer=='\r'){
			charbuffer='\n';
			write(uart_pointer,&charbuffer,1);
			charbuffer='\r';
			write(uart_pointer,&charbuffer,1);
			enterPressed=1;
		} 
		
		/* Interpret backspaces correctly in the string by decrementing i */
		else if (charbuffer==BACKSPACE){
			if (i>0){
				write(uart_pointer,&charbuffer,1);
				i=i-1;
				string[i]='\0';
			}
		} 
		
		/* Interpret arrow keys */
		else if (charbuffer==SPECIAL1){
			//write(uart_pointer,&charbuffer,1);
			read(uart_pointer,&charbuffer,1);
			//write(uart_pointer,&charbuffer,1);
			if (charbuffer==SPECIAL2){
				read(uart_pointer,&charbuffer,1);
				switch(charbuffer){
					case LEFT:
						charbuffer=SPECIAL1;
						write(uart_pointer,&charbuffer,1);
						charbuffer=SPECIAL2;
						write(uart_pointer,&charbuffer,1);
						charbuffer=LEFT;
						write(uart_pointer,&charbuffer,1);
						if (i>0) i--;
						break;
					case RIGHT:
						if (string[i]!='\0'){
							charbuffer=SPECIAL1;
							write(uart_pointer,&charbuffer,1);
							charbuffer=SPECIAL2;
							write(uart_pointer,&charbuffer,1);
							charbuffer=RIGHT;
							write(uart_pointer,&charbuffer,1);
							i++;
						}
						break;
					default:
						break;
				}
			}
		}

		/* Echo everything else */
		else{
			//printf("%x\n",string[i]);
			write(uart_pointer,&charbuffer,1);
			string[i] = charbuffer;
			i++;
		}
		//printf("i is %d\nstring is %s\n",i,string);

	} while(!enterPressed && i<lineLength); // loop until newline or line length is reached
	if (i>=lineLength){
		string[0]='\0';
		return -1;
	}
	printf("> %s\n",string);
	close(uart_pointer);
	return i;
}

