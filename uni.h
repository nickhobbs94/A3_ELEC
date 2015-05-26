#ifndef UNI_H
#define UNI_H

#define BACKSPACE 0x7f
#define UP 0x41
#define DOWN 0x42
#define LEFT 0x44
#define RIGHT 0x43
#define SPECIAL1 0x1b
#define SPECIAL2 0x5b

#define PUTTY_LINE_LENGTH 150   // max line length able to be entered into putty

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "LCD_Control.h"
#include "spi_sd.h"
#include "io.h"
#include "system.h"
#include "altera_up_avalon_audio_dgz.h"
#include "AUDIO.h"
#include "altera_up_avalon_audio_regs_dgz.h"

void puttyPrintLine(const char* format,...){
	va_list vlist;
	va_start(vlist,format);
	char buf[PUTTY_LINE_LENGTH];
	vsprintf(buf,format,vlist);
	alt_32 uart_pointer = open("/dev/uart_0", O_WRONLY);
	write(uart_pointer, buf, strlen(buf));
	close(uart_pointer);
}

void puttyPrintChars(alt_8 string[], alt_32 length){
	alt_32 uart_pointer = open("/dev/uart_0", O_WRONLY);
	write(uart_pointer, string, length);
	close(uart_pointer);
}

alt_32 puttyGetline(alt_8 string[], alt_32 lineLength);

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

#endif