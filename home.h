#ifndef HOME_H
#define HOME_H

#define BACKSPACE 0x7f
#define UP 0x41
#define DOWN 0x42
#define LEFT 0x44
#define RIGHT 0x43
#define SPECIAL1 0x1b
#define SPECIAL2 0x5b

#define LED_RED_BASE 0
#define AUDIO_NAME 0
#define SWITCH_PIO_BASE 0
#define SEG7_DISPLAY_BASE 0

#define RATE_ADC8K_DAC8K_USB 8000
#define RATE_ADC32K_DAC32K_USB 32000
#define RATE_ADC44K_DAC44K_USB 44000
#define RATE_ADC48K_DAC48K_USB 48000
#define RATE_ADC96K_DAC96K_USB 96000

#define ALT_UP_AUDIO_RIGHT 0
#define ALT_UP_AUDIO_LEFT 1

#define PUTTY_LINE_LENGTH 150

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "altstring.h"
#include "conversions.h"
#include "SDDir.h"
#include "alt_types.h"
#include "newtypes.h"


typedef int alt_up_audio_dev;


/* ------------------------------------- Function prototypes ------------------------------------- */
alt_32 puttyGetline(alt_8 string[], alt_32 lineLength);
void  LCD_Init();
alt_32 sd_readSector(alt_32 address, alt_u8* buffer);
int* alt_up_audio_open_dev(int x);

/* ------------------------------------- Functions ------------------------------------- */
void  LCD_Show_Text(alt_8* Text) {
	printf("%s", Text);
}
void  LCD_Line2() {
	printf("\n");
}
void  LCD_Show_Decimal(alt_32 input) {
	printf("%d", input);
}

alt_32 sd_readSector(alt_32 address, alt_u8* buffer){
	FILE* fp = fopen(OUR_SD_DIR_LOCATION, "r");
	//FILE* fp = fopen("/Users/nicholashobbs/Downloads/Torrent/2015-05-05-raspbian-wheezy.img", "r");
	if (fp == NULL){
		return -1;
	}
	
	fseek(fp, address*512, SEEK_SET);
	fread(buffer, 1, 512, fp);
	fclose(fp);
	return 0;
}

alt_32 puttyGetline(alt_8 string[], alt_32 lineLength){
	alt_8* newstring = malloc(lineLength + 1);
	unsigned long nbytes = lineLength;
	getline(&newstring, &nbytes, stdin);
	altstrcpy(string, newstring);
	printf("> %s\n", string);
	return altstrlen(string);
}



alt_32 if_initInterface(){
	return 0;
}

void IOWR(int x, ...){
	return;
}

int IORD(int x, ...){
	return 0;
}

int* alt_up_audio_open_dev(int x){
	return NULL;
}

void AUDIO_SetSampleRate(int x){
	printf("Audio set rate %d\n", x);
}

int alt_up_audio_write_fifo_space(int *audio_dev, int x){
	static int i;
	i++;
	return i;
}

int alt_up_audio_write_fifo(int* audio_dev, unsigned int* buffer, int length, int channel){
	return 0;
}

int AUDIO_Init(){
	return 0;
}

int alt_up_audio_reset_audio_core(int * x){
	return 0;
}

void LCD_Init(){
	return;
}


void puttyPrintLine(const char* format,...){
	va_list vlist;
	va_start(vlist,format);
	vprintf(format,vlist);
}

void puttyPrintChars(alt_8 string[], alt_32 length){
	int i;
	for (i=0; i<length; i++){
		printf("%c", string[i]);
	}
	printf("\n");
}

#endif