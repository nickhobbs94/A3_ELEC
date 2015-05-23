/* 
	Handy conversions will live in this header file e.g. extracting an int from a string.
*/

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

/* Includes */
#include <math.h>
#include <stdlib.h>
#include "alt_types.h"
#include "altstring.h"

/* Magic numbers */
#define BINARY_DIVISOR 2
#define HEX_DIVISOR 16
#define LARGEST_POSITIVE_INT "2147483647"
#define ABS_SMALLEST_NEGATIVE_INT "2147483648"
#define MAX_NUM_STRINGLEN 10

/* Function prototypes */
alt_32 intfromstring(alt_8 string[]);
void decimaltobinary(alt_32* binaryArray, alt_32 decimal);
void decimaltohex(alt_32* hexArray, alt_32 decimal);
alt_8 charfromint(alt_32 number);
void uppercasestring(alt_8* lowerCaseString);
alt_32 formatStringForFAT(alt_8* instring, alt_8* outstring);

/* ----------------------------------- Functions ----------------------------------- */

alt_32 intfromstring(alt_8 string[]) {
	alt_32 result=0;
	alt_8 isPositive = 1;
	
	/* If the string starts with a + or - then increment the pointer of the string so as to ignore it */
	if (string[0]=='+'){
		string+=1;
	} else if (string[0]=='-'){
		isPositive = 0;
		string+=1;
	}
	
	/* Check that the string to be converted is not larger than can be stored in a 32 bit signed int */
	if (altstrlen(string) == MAX_NUM_STRINGLEN){
		if (altstrcmp(string, LARGEST_POSITIVE_INT) > 0 && isPositive){
			return 0;
		} else if (altstrcmp(string, ABS_SMALLEST_NEGATIVE_INT) > 0 && !isPositive){
			return 0;
		}
	} else if (altstrlen(string) > MAX_NUM_STRINGLEN){
		return 0;
	}

	alt_32 stringlength = altstrlen(string);
	alt_32 i;
	
	
	for (i = 0; i<stringlength; i++){
		if (string[i] < '0' || string[i] > '9') {
			return 0; // return 0 if a non number is read from the string (check the user didnt just type 0)
		}
		result = result * 10;
		result = result + (string[i] & 0x0f);
	}
	
	/* Interpret sign */
	if (!isPositive){
		result = result * -1;
	}
	
	return result;
}

alt_8 charfromint(alt_32 number){
//	alt_8 answer;
	if (number < 0 || number > 9){
		return -1;
	}

	alt_8 lookup_table[]={'0','1','2','3','4','5','6','7','8','9'};
	return lookup_table[number];
}

void decimaltobinary(alt_32* binaryArray, alt_32 decimal){
	alt_32 i;
	for(i=0; decimal>0; i++){
		binaryArray[i] = (decimal % BINARY_DIVISOR);
		decimal /= BINARY_DIVISOR;
	}
}

void decimaltohex(alt_32* hexArray, alt_32 decimal){
	alt_32 i;
	for(i=0; decimal>0; i++){
		hexArray[i] = (decimal % HEX_DIVISOR);
		decimal /= HEX_DIVISOR;
	}
}

alt_32 extract_little(alt_u8* buffer, alt_32 size){
	alt_32 sum = 0;
	alt_32 i;
	for (i=0; i<size; i++){
		sum += buffer[i] << i*8;
	}
	return sum;
}

void uppercasestring(alt_8* lowerCaseString){
	while(*lowerCaseString != '\0'){
		if(*lowerCaseString >= 'a' && *lowerCaseString <= 'z'){
			*lowerCaseString -= 0x20;
		}
		++lowerCaseString;
	}
}

/* Outstring is an array of 12 */
alt_32 formatStringForFAT(alt_8* instring, alt_8* outstring){
	alt_32 instring_length = altstrlen(instring);
	alt_32 outstring_length = 11;
	alt_32 check;

	alt_8* instringCopy = malloc(instring_length);
	alt_8* fileExtension;
	if (instringCopy == NULL){
		return -1;
	}
	altstrcpy(instringCopy, instring);
	uppercasestring(instringCopy);
	check = string_replace(instringCopy, '.', '\0', 1, -1); // replce 1 period from the back of the string with a null
	if (check == 1){
		fileExtension = instringCopy + altstrlen(instringCopy) + 1;
	} else {
		fileExtension = (alt_8*) &"   ";
	}

	if (altstrlen(instringCopy) > 8){
		instringCopy[8] = '\0';
	}

	altstrcpy(outstring, instringCopy);
	// append spaces to filename
	alt_32 i;
	for (i=0; i < 8 - altstrlen(instringCopy); i++){
		altstrcat(outstring, " ");
	}
	altstrcat(outstring, fileExtension);
	free(instringCopy);
	return 0;
}



#endif