/*
	Every function that is implemented on our terminal will be in this file.
	This allows for easy editing and extensions.
*/

#ifndef TERMINALFUNCTIONS_H
#define TERMINALFUNCTIONS_H

/* Includes */
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "conversions.h"
#include "alt_types.h"
#include "efs.h"
//#include "ls.h"
#include "SD_functions.h"
#include "altstring.h"


/* Magic numbers */
#define NUMBER_OF_LEDS 18
#define BINARY_BITS_IN_DECIMAL(decimalNumber) log2(decimalNumber)+1
#define MAX_LED_REGISTER 131071
#define MIN_LED_REGISTER -131072


/* Function prototypes */
alt_32 echo(alt_32 argc, alt_8* argv[]);
alt_32 add(alt_32 argc, alt_8* argv[]);
alt_32 ledr(alt_32 argc, alt_8* argv[]);
alt_32 switch_function(alt_32 argc, alt_8* argv[]);
alt_32 ls_path(alt_32 argc, alt_8* argv[]);
alt_32 change_dir(alt_32 argc, alt_8* argv[]);
alt_32 tf_mount(alt_32 argc, alt_8* argv[]);
alt_32 tf_unmount(alt_32 argc, alt_8* argv[]);
alt_32 make_directory(alt_32 argc, alt_8* argv[]);
alt_32 delete_file(alt_32 argc, alt_8* argv[]);
alt_32 write_new_file(alt_32 argc, alt_8* argv[]);
alt_32 read_file(alt_32 argc, alt_8* argv[]);
alt_32 copy_file(alt_32 argc, alt_8* argv[]);
alt_32 wav_play(alt_32 argc, alt_8* argv[]);

/* ----------------------------------- Functions ----------------------------------- */

/* Print to the LCD and the console */
alt_32 echo(alt_32 argc, alt_8* argv[]){
	alt_32 i;
	LCD_Init();

	/* Loop over each argument provided to the function and print each one to the LCD and console */
	for (i=1; i<argc; i++){
		puttyPrintLine(argv[i]);
		puttyPrintLine("\r\n");
		LCD_Show_Text(argv[i]);
		LCD_Show_Text((alt_8*) " ");
	}

	if (argc <= 1){
		puttyPrintLine("<Empty input string>\n\r");
	}
	return 0;
}

/* Add two or more numbers and show the sum on the LCD */
alt_32 add(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s number1 number2 ...\n\r", argv[0]);
		return -1;
	}

	alt_32 sum = 0;
	alt_32 i;
	alt_32 temp;
	alt_32 tempsum;
	LCD_Init();
	alt_8 printstring[MAX_STRINGLEN];
	altmemset(printstring, '\0', MAX_STRINGLEN);

	for (i=1; i<argc; i++){
		/* Print each argument */
		altstrcat(printstring, argv[i]);

		/* Print a trailing '+' for all but the last argument */
		if (i<argc-1) {
			altstrcat(printstring, "+");
		}

		/* Get an int from each arg and add it to temp */
		temp = intfromstring(argv[i]);
		if (temp == 0 && (argv[i][0]!='0' || (argv[i][0]!='-' && argv[i][1]!='0') )){
			puttyPrintLine("Invalid input integer (too big/not a number)\n\r");
			return -1;
		}
		tempsum = temp + sum;
		/* Check for signed overflow */
		if (temp > 0 && sum > 0 && tempsum < 0){
			puttyPrintLine("Addition overflow\n\r");
			return -1;
		} else if (temp < 0 && sum < 0 && tempsum >= 0){
			puttyPrintLine("Signed addition overflow\n\r");
			return -1;
		}
		sum = tempsum;
	}
	puttyPrintLine("%s=%d\n\r",printstring,sum);
	LCD_Show_Decimal(sum);
	return 0;
}

/* Updates the red LEDs to be the binary representation of a number */
alt_32 ledr(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s number1\n\r", argv[0]);
		return -1;
	}
	alt_32 dec=0;
	dec = intfromstring(argv[1]);

	/* Return from the program if input wasn't a number or bigger than the max for the LEDs*/
	if ((dec == 0 && (altstrcmp(argv[1], (alt_8*)"0") != 0))){
		puttyPrintLine("Argument 1 for ledr is invalid: %s\n\r",argv[1]);
		return -1;
	} else if (dec > MAX_LED_REGISTER || dec < MIN_LED_REGISTER){
		puttyPrintLine("User input outside of acceptable range (%d,%d)\n\r",MIN_LED_REGISTER,MAX_LED_REGISTER);
		return -1;
	}
	IOWR(LED_RED_BASE, 0, dec);
	return 0;
}

/* shows the switch configuration as hex on the 7-seg display */
alt_32 switch_function(alt_32 argc, alt_8* argv[]){
	alt_32 dec = IORD(SWITCH_PIO_BASE,0);
	alt_32 i;
	alt_32* hex = calloc(8,sizeof(alt_32));
	decimaltohex(hex,dec);
	static const alt_u8  Map[] = {
		63, 6, 91, 79, 102, 109, 125, 7,
		127, 111, 119, 124, 57, 94, 121, 113
	};  // 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f
	for(i=0;i<8;++i) {
		//printf("%x\n",hex[i]);
		IOWR(SEG7_DISPLAY_BASE,i,Map[hex[i]]);
	}
	free(hex);
	return 0;
}

/* lists the contents of a directory */
alt_32 ls_path(alt_32 argc, alt_8* argv[]){
	EmbeddedFileSystem* efsl;
	DirList list;
	alt_8 isEmptyDir = 1;
	
	efsl = *(SD_mount());

	if (efsl==NULL) return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());

	if (argc>1){
		SD_updatePath(path,argv[1]);
	}
	printf("PATH: %s\n",path);
	puttyPrintLine("PATH: %s\n\r",path);

	/* Read directory */
	alt_8 checkpath = ls_openDir(&list,&(efsl->myFs),path);
	//printf("checkpath=%d\n",checkpath);
	if (checkpath!=0){
		printf("Unable to read path: %s\n",path);
		puttyPrintLine("Unable to read path: %s\n\r",path);
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	char attribute;
	while(ls_getNext(&list)==0){
		isEmptyDir = 0;
		attribute = SD_getFileAttribute(list.currentEntry.Attribute);
		printf("%s \t%c \t(%d)\n",
				list.currentEntry.FileName,
				attribute,
				list.currentEntry.FileSize
				);
		puttyPrintLine((alt_8*)list.currentEntry.FileName);
		if (attribute == 'd'){
			puttyPrintLine(" \t[Folder] \t");
		} else {
			puttyPrintLine(" \t[ File ] \t");
		}
		puttyPrintLine("(%d bytes)\n\r",list.currentEntry.FileSize);
	}

	if (isEmptyDir){
		printf("The directory is empty\n");
		puttyPrintLine("The directory is empty\n\r");
	}
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	return 0;
}

/* changes the current directory */
alt_32 change_dir(alt_32 argc, alt_8* argv[]){
	EmbeddedFileSystem* efsl;
	DirList list;
	efsl = *(SD_mount());


	alt_8* path;
	path = SD_getCurrentPath();

	/* check it is a valid directory that the user wants to change the path to */
	alt_8 checkpath=0;
	if (argc>1){
		checkpath = ls_openDir(&list,&(efsl->myFs),(char*)argv[1]);
		if (checkpath==0){
			SD_updatePath(path,argv[1]);
		} else {
			printf("Invalid path: %s\n",argv[1]);
			puttyPrintLine("Invalid path: %s\n\r",argv[1]);
		}
	}
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	
	printf("PATH: %s\n",path);
	puttyPrintLine("PATH: %s\n\r",path);
	return 0;
}

alt_32 tf_mount(alt_32 argc, alt_8* argv[]){
	SD_mount();
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	return 0;
}

alt_32 tf_unmount(alt_32 argc, alt_8* argv[]){
	SD_unmount();
	return 0;
}


alt_32 make_directory(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		printf("Syntax: %s directory\n",argv[0]);
		puttyPrintLine("Syntax: %s directory\n\r",argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	printf("Making %s\n",path);
	puttyPrintLine("Making %s\n\r",path);
	alt_32 check;

	check = makedir(&(efsl->myFs), path);
	if (check != 0){
		puttyPrintLine("This directory already exists\n\r");
	}

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

/* For ability to delete directories
 * Had to edit efsl file: /FileSystemLibSrc/src/ui.c line 139 from
 * 		if((fs_findFile(fs,(eint8*)filename,&loc,0))==1){
 * to
 * 		if((fs_findFile(fs,(eint8*)filename,&loc,0))){
 */
alt_32 delete_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;

	efsl = *(SD_mount());

	if (efsl==NULL){
		return -1;
	}

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	/* Remove the file */
	alt_16 result = rmfile(&(efsl->myFs), path);

	if(result == -1){
		puttyPrintLine("Error: file does not exist\n\r");
	}

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	
	return 0;
}

alt_32 write_new_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath contents...\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file;
	alt_8 write_buffer[SD_MAX_PATH_LENGTH];
	altmemset(write_buffer,'\0',SD_MAX_PATH_LENGTH);
	alt_32 i;
	for(i=2;i<argc;i++){
		altstrcat(write_buffer,argv[i]);
		if (i<argc-1){
			altstrcat(write_buffer," ");
		}
	}

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	if(file_fopen(&file, &(efsl->myFs), path, 'w')!=0){
				puttyPrintLine("Could not open file for writing\n\r");
				if (UNMOUNT_SD_AFTER_OPERATION){
					SD_unmount();
				}
				return 0;
	}
	puttyPrintLine("File opened for writing.\n\r");

	if(file_write(&file,altstrlen((alt_8*)write_buffer),(alt_8*)write_buffer) == altstrlen((alt_8*)write_buffer)){
		puttyPrintLine("File written.\n\r");
	} else {
		puttyPrintLine("Could not write file.\n\r");
	}

	file_fclose(&file);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

alt_32  read_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file;
	euint8 buffer[513];
	euint16 e;//,f;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
				puttyPrintLine("Could not open file for reading\n\r");
				if (UNMOUNT_SD_AFTER_OPERATION){
					SD_unmount();
				}
				return -1;
	}
	printf("File %s opened for reading.\n",path);
	//puttyPrintLine("Opened file: ");
	//puttyPrintLine(path);
	//puttyPrintLine(" for reading\n");

	while((e=file_read(&file,512,buffer))){
		//printf("%d\n", e);
		/*for(f=0;f<e;f++){
			printf("%c",buffer[f]);
		}*/
		if (e>=513) {
			printf("ERROR READING FILE\n\r");
		} else {
			buffer[e] = '\0';
			puttyPrintLine("%s", buffer);
		}
		

	}
	puttyPrintLine("\n\r");

	file_fclose(&file);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

alt_32 copy_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 2){
		puttyPrintLine("Syntax: %s source destination\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file1, file2;
	euint8 buffer[512];
	euint16 e;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path of read file */
	alt_8 read_file_path[SD_MAX_PATH_LENGTH];
	altstrcpy(read_file_path,SD_getCurrentPath());
	SD_updatePath(read_file_path,argv[1]);

	/* Get absolute path of write file */
	alt_8 write_file_path[SD_MAX_PATH_LENGTH];
	altstrcpy(write_file_path,SD_getCurrentPath());
	SD_updatePath(write_file_path,argv[2]);

	if(file_fopen(&file1, &(efsl->myFs), read_file_path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	puttyPrintLine("File opened for reading.\n\r");

	e=file_read(&file1,512,buffer);
	if(file_fopen(&file2, &(efsl->myFs), write_file_path, 'w')!=0){
		puttyPrintLine("Could not open file for writing\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	puttyPrintLine("File opened for writing.\n\r");

	if(file_write(&file2,e,(alt_8*)buffer)==e){
		puttyPrintLine("File written.\n\r");
	} else {
		puttyPrintLine("could not write file.\n\r");
	}

	file_fclose(&file1);
	file_fclose(&file2);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}


#define SIZE_OF_HEADER 44
#define STRING_LEN 4
#define OUTPUT_BUFFER_LEN 1024

/* Define structure of header */
typedef struct{
	/* "RIFF" Descriptor */
	alt_8 Chunk_ID[STRING_LEN];
	alt_32 Chunk_Size;
	alt_8 Format[STRING_LEN];
	/* "fmt" sub-chunk */
	alt_8 Subchunk_ID[STRING_LEN];
	alt_32 Subchunk_Size;
	alt_16 Audio_Format;
	alt_16 Num_Channels;
	alt_32 Sample_Rate;
	alt_32 Byte_Rate;
	alt_16 Block_Align;
	alt_16 Bits_Per_Sample;
	/* "data" sub-chunk */
	alt_8 Subchunk2_ID[STRING_LEN];
	alt_32 Subchunk2_Size;
} Wave_Header;

alt_32 wav_play(alt_32 argc, alt_8* argv[]){
	/* Check number of arguments */
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}

	/* Try to mount SD card */
	EmbeddedFileSystem* efsl;
	efsl = *(SD_mount());
	if (efsl==NULL){
		return -1;
	}

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	/* Try to open file */
	File file;
	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	/* Try to read header from file */
	euint8 buffer[SIZE_OF_HEADER];
	euint16 result;
	result = file_read(&file,SIZE_OF_HEADER,buffer);
	if (result != SIZE_OF_HEADER){
		puttyPrintLine("Error reading header\n\r");
		return -1;
	}

	/* Set pointer to header structure to the newly read header */
	Wave_Header* header_data;
	header_data = (Wave_Header*) buffer;

	/* Init audio codec and device (only needs to be done once) */
	if ( !AUDIO_Init() ) {
		printf("Unable to initialise audio codec\n");
		return -1;
	}
	alt_up_audio_dev*  audio_dev;
	audio_dev = alt_up_audio_open_dev(AUDIO_NAME);

	/* Reset the FIFO in audio codec (do this every time a new stream is loaded) */
	alt_up_audio_reset_audio_core(audio_dev);

	/* Set the sampling frequency (do this every time a new stream is loaded) */
	switch(header_data->Sample_Rate) {
	  case 8000:  AUDIO_SetSampleRate(RATE_ADC8K_DAC8K_USB); break;
	  case 32000: AUDIO_SetSampleRate(RATE_ADC32K_DAC32K_USB); break;
	  case 44100: AUDIO_SetSampleRate(RATE_ADC44K_DAC44K_USB); break;
	  case 48000: AUDIO_SetSampleRate(RATE_ADC48K_DAC48K_USB); break;
	  case 96000: AUDIO_SetSampleRate(RATE_ADC96K_DAC96K_USB); break;
	  default:  puttyPrintLine("Non-standard sampling rate\n\r"); return -1;
   }

	/* Print some header data */
	puttyPrintLine("Chunk ID: ");
	puttyPrintChars(header_data->Chunk_ID, STRING_LEN);
	puttyPrintLine("\n\rFormat: ");
	puttyPrintChars(header_data->Format, STRING_LEN);

	puttyPrintLine("\n\rSampleRate: %d\n\rNumChannels: %d\n\r", header_data->Sample_Rate, header_data->Num_Channels);
	puttyPrintLine("Size: %d\n\rBits Per Sample: %d\n\r", header_data->Subchunk2_Size, header_data->Bits_Per_Sample);

	/* Check the number of channels */
	if (header_data->Num_Channels != 1 && header_data->Num_Channels != 2){
		puttyPrintLine("Invalid number of channels\n\r");
	}

	/* --------------------------------------- Play the audio data --------------------------------------- */
	alt_32 bytesPerSample = header_data->Bits_Per_Sample / 8;
	alt_32 FILE_BUFFER_LEN = OUTPUT_BUFFER_LEN * header_data->Num_Channels * bytesPerSample;
	alt_32 totalBytesRead = 0;
	
	alt_u32 buffer1[OUTPUT_BUFFER_LEN];
	alt_u32 buffer2[OUTPUT_BUFFER_LEN];

	/* Init file buffer depending on sample depth */

	alt_u8 *fileBuffer = malloc(FILE_BUFFER_LEN);

	printf("%d \t %d\n", FILE_BUFFER_LEN, OUTPUT_BUFFER_LEN);

	alt_32 i;
	alt_32 bytesRead;
	
	/* Loop over the length of the file */

	//this is bad remove it later
	//file.FileSize = header_data->Subchunk2_Size;

	while (totalBytesRead < header_data->Subchunk2_Size){
		/* Read enough data for the number of channels to load one buffer's worth each */
		if (totalBytesRead + FILE_BUFFER_LEN < header_data->Subchunk2_Size){
			bytesRead = file_read(&file, FILE_BUFFER_LEN, fileBuffer);
			totalBytesRead += bytesRead;
		} else {
			printf("LAST THINGY\n");
			bytesRead = file_read(&file, (header_data->Subchunk2_Size - totalBytesRead), fileBuffer);
			totalBytesRead += bytesRead;
		}

		/* If the bytes just read is not large enough and the end of the file 
			has not been reached, then return an error */
		if (bytesRead != FILE_BUFFER_LEN   &&   totalBytesRead <= header_data->Subchunk2_Size){
			puttyPrintLine("End of file was unexpectedly reached %d %d %d\n\r", bytesRead, totalBytesRead, header_data->Subchunk2_Size);
			free(fileBuffer);
			file_fclose(&file);
			if (UNMOUNT_SD_AFTER_OPERATION){
				SD_unmount();
			}
			return -1;
		}

		/* Split the buffer into the number of channels and amplify if not 32 bit sampling */
		for (i=0; i<bytesRead/header_data->Num_Channels/bytesPerSample; i++){
			switch(header_data->Bits_Per_Sample){
				case 8:
					buffer1[i] = *((alt_u8*)fileBuffer + (header_data->Num_Channels)*i);
					buffer2[i] = *((alt_u8*)fileBuffer + (header_data->Num_Channels)*i + (header_data->Num_Channels)/2);
					buffer1[i] = (buffer1[i] << 24)/2;
					buffer2[i] = (buffer2[i] << 24)/2;
					break;
				case 16:
					buffer1[i] = *((alt_u16*)fileBuffer + header_data->Num_Channels*i);
					buffer2[i] = *((alt_u16*)fileBuffer + header_data->Num_Channels*i + header_data->Num_Channels/2);
					buffer1[i] = (buffer1[i] << 16);
					buffer2[i] = (buffer2[i] << 16);
					break;
				case 32:
					buffer1[i] = *((alt_u32*)fileBuffer + (header_data->Num_Channels)*i);
					buffer2[i] = *((alt_u32*)fileBuffer + (header_data->Num_Channels)*i + (header_data->Num_Channels)/2);
					break;
				default:
					puttyPrintLine("Unsupported number of sampling bits\n\r");
					file_fclose(&file);
					if (UNMOUNT_SD_AFTER_OPERATION){
						SD_unmount();
					}
					free(fileBuffer);
					return -1;
			}
		}

		/* Wait for both left and right FIFOs to be free */
		while(alt_up_audio_write_fifo_space(audio_dev,ALT_UP_AUDIO_RIGHT) < OUTPUT_BUFFER_LEN);
		while(alt_up_audio_write_fifo_space(audio_dev,ALT_UP_AUDIO_LEFT) < OUTPUT_BUFFER_LEN);

		/* Write data into right and left channel of audio codec FIFO */
		alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer1, bytesRead/bytesPerSample/header_data->Num_Channels, ALT_UP_AUDIO_RIGHT);
		alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer2, bytesRead/bytesPerSample/header_data->Num_Channels, ALT_UP_AUDIO_LEFT);
	}

	file_fclose(&file);
	free(fileBuffer);

    if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
    return 0;
}


#endif

