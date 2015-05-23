/*
altstring.h
*/
#ifndef ALTSTRING_H
#define ALTSTRING_H

/* Includes */
#include "alt_types.h"

/* Magic numbers */
#define MAX_STRINGLEN 200

/* Function prototypes */
alt_8* altstrcpy(alt_8* destination, alt_8* source);
alt_32 altstrcmp(alt_8* string1, alt_8* string2);
alt_32 altstrlen(alt_8* string);
/* ------------------------- Functions -------------------------- */
alt_32 altstrlen(alt_8* string){
	alt_32 i;
	for (i=0; i<MAX_STRINGLEN && string[i]!='\0'; i++){
		;
	}
	return i;
}

alt_8* altstrcpy(alt_8* destination, alt_8* source){
	alt_8* startofstring = destination;
	
	/* increment the strings */
	while (*source != '\0'){
		*destination++ = *source++;
	}
	*destination = '\0';
	return startofstring;
}

alt_32 altstrcmp(alt_8* string1, alt_8* string2){
	while (*string1 != '\0' && *string2 != '\0'){
		if (*string1 != *string2){
			break;
		}
		string1++;
		string2++;
	}
	if (*string1 < *string2){
		return -1;
	} else if (*string1 > *string2){
		return 1;
	}
	return 0;
}

alt_8* altstrcat(alt_8* destination, alt_8* source){
	alt_32 i = altstrlen(destination);
	alt_32 j;
	for (j=0; source[j] != '\0'; j++){
		destination[i] = source[j];
		i++;
	}
	destination[i] = '\0';
	return destination;
}

void altmemset(alt_8* destination, alt_8 value, alt_32 size){
	alt_32 i;
	for (i=0; i<size; i++){
		destination[i] = value;
	}
}


alt_8 altsplitstring(alt_8* string, alt_8* array_of_words[], alt_8 character){
	if (*(string)=='\0'){
		return 0;   // return 0 if empty string
	}
	alt_u8 count = 0;
	alt_32 string_length = altstrlen(string);
	
	if (*(string)!=character){	// if the first character is not a space then start the first word there
		count++;
		array_of_words[0] = string;
	}
	alt_32 i;
	for (i=0; i<string_length; i++){	
		if (*(string+i)==character){
			*(string+i)='\0'; // replace spaces with nulls
			if(*(string+i+1)!='\0' && *(string+i+1)!=character){
				array_of_words[count]=string+i+1; // set pointer to the next word
				count++;
			}
		} else if (*(string+i)=='\0'){
			i=string_length;
		}
	}
	return count;  // returns the number of words found
}

#endif

