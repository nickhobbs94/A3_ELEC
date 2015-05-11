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

#endif

