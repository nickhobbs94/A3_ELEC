#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H
#include <stdio.h>
#include "altstring.h"
#include <unistd.h>  // for usec()
#include "alt_types.h"
#include "system.h"
#include "io.h"
#include "conversions.h"

#define lcd_write_cmd(base, data)                     IOWR(base, 0, data)
#define lcd_read_cmd(base)                            IORD(base, 1)
#define lcd_write_data(base, data)                    IOWR(base, 2, data)
#define lcd_read_data(base)                           IORD(base, 3) 

#define MAX_DECIMAL_DIGITS_32BIT_NUM 10
//-------------------------------------------------------------------------
void  LCD_Init();
void  LCD_Show_Text(alt_8* Text);
void  LCD_Line2();
void  LCD_Test();
void  LCD_Show_Decimal(alt_32 input);
//-------------------------------------------------------------------------

void LCD_Line2() {
  lcd_write_cmd(LCD_16207_0_BASE,0xC0);
  usleep(2000);
}

void LCD_Init() {
  IOWR(LCD_16207_0_BASE,0,0x38); 
  usleep(2000);
  IOWR(LCD_16207_0_BASE,0,0x0C);
  usleep(2000);
  IOWR(LCD_16207_0_BASE,0,0x01);
  usleep(2000);
  IOWR(LCD_16207_0_BASE,0,0x06);
  usleep(2000);
  IOWR(LCD_16207_0_BASE,0,0x80);
  usleep(2000);
}

void LCD_Show_Text(alt_8* Text) {
  int i;
  for(i=0;i<altstrlen(Text);i++) {
    lcd_write_data(LCD_16207_0_BASE,Text[i]);
    usleep(2000);
  }
}

void  LCD_Show_Decimal(alt_32 input){
	alt_8 string[MAX_DECIMAL_DIGITS_32BIT_NUM+1];
	alt_8* outputstring;
	alt_32 i;
	alt_8 isNegative = 0;

	if (input <0){
		input = input * -1;
		isNegative = 1;
	}

	//alt_32 j=0;
	string[MAX_DECIMAL_DIGITS_32BIT_NUM]='\0';
	for (i=MAX_DECIMAL_DIGITS_32BIT_NUM-1; i>=0; i--){
		string[i]=charfromint(input%10);
		input=input/10;
		if (input==0) {
			if (isNegative && i>0){
				i--;
				string[i] = '-';
			}
			outputstring = string+i;
			i=-1;
		}
	}
	LCD_Show_Text(outputstring);
}

#endif
