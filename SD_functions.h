#ifndef SD_FUNCTIONS_H
#define SD_FUNCTIONS_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "altstring.h"
#include "alt_types.h"
#include "efs.h"
//#include "ls.h"
#include "conversions.h"

/* Magic numbers */
#define SD_MAX_PATH_LENGTH 200
#define SD_FOLDER_BITMASK 0x10
#define UNMOUNT_SD_AFTER_OPERATION 1
#define PUTTY_LINE_LENGTH 150   // max line length able to be entered into putty

/* Function prototypes */
EmbeddedFileSystem** SD_mount();
void SD_unmount(void);
alt_8 SD_getFileAttribute(alt_u8 input);
alt_8* SD_getCurrentPath();
void SD_updatePath(alt_8* currentPath, alt_8 argument[]);
void puttyPrintLine(const char* format,...);
//void puttyPrintInt(alt_32 number);

/* ------------------------- Functions -------------------------- */

/* Mount the SD card */
EmbeddedFileSystem** SD_mount(){
	static EmbeddedFileSystem someFileSystem;
	static EmbeddedFileSystem* someFileSystemPointer;

	if (someFileSystemPointer!=NULL) return &someFileSystemPointer;

	someFileSystemPointer = &someFileSystem;
	esint8 check;
	printf("Will init efsl now\n");
	check = efs_init(someFileSystemPointer,"/dev/sda");
	if (check==0){
		printf("Filesystem correctly initialized\n");
	} else {
		puttyPrintLine("Could not init filesystem\n\r");
		printf("Could not init filesystem\n");
		someFileSystemPointer = NULL;
	}
	return &someFileSystemPointer;
}

/* Unmount the SD card */
void SD_unmount(void){
	EmbeddedFileSystem** efsl;
	efsl = SD_mount();
	fs_umount(&((*efsl)->myFs));
	if (!UNMOUNT_SD_AFTER_OPERATION){
		*efsl=0;
	}
	
	printf("Unmounted the SD card\n");
}

/* Tell whether the current entry is a directory or a file */
alt_8 SD_getFileAttribute(alt_u8 input){
	alt_8 attribute=0;
	if ((input & SD_FOLDER_BITMASK) != 0){
		attribute='d'; // Directory
	} else {
		attribute='f'; // File
	}
	return attribute;
}

/* Get the path of the current directory */
alt_8* SD_getCurrentPath(){
	static alt_8 path[SD_MAX_PATH_LENGTH];
	static alt_8* pathPointer;
	if (pathPointer != NULL) return pathPointer;

	altmemset(path,'\0',sizeof(path));
	altstrcpy(path,(alt_8*)"/");
	pathPointer = path;

	return pathPointer;
}

/* Update the path given */
void SD_updatePath(alt_8* currentPath, alt_8 argument[]){
	if (argument[0]=='/'){
		altstrcpy(currentPath,argument);
	} else {
		if (currentPath[altstrlen(currentPath)-1] != '/'){
			altstrcat(currentPath,(alt_8*)"/");
		}
		altstrcat(currentPath,argument);
	}
}

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

/*void puttyPrintInt(alt_32 number){
	alt_32 temp;
	alt_8 array[SD_PUTTY_MAX_INT_DIGITS];
	alt_32 stringindex = SD_PUTTY_MAX_INT_DIGITS - 1;
	alt_8 charnum;
	altmemset(array, '\0', SD_PUTTY_MAX_INT_DIGITS);
	
	do {
		temp = number % 10;
		if (number != 0){
			stringindex--;
			charnum = charfromint(temp);
			if (charnum == -1){
				printf("Error printing an integer: internal error");
				return;
			}
			array[stringindex] = charnum;
		}
		number = number / 10;
		printf("%d\n%d\n",stringindex,charnum);
	} while (number != 0 && stringindex > 0);
	printf("%s\n",array+stringindex);
	puttyPrintLine(array+stringindex);
}
*/
#endif

