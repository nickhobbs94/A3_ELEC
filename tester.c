/* Tests FATKINS.h */

#include "FATKINS.h"
#include "alt_types.h"
#include <stdio.h>

alt_32 sd_readSector(alt_32 address, alt_8* buffer){
	FILE* fp = fopen("/Users/nicholashobbs/Desktop/SD.dmg", "r");
	if (fp == NULL){
		return -1;
	}
	
	fseek(fp, address*512, SEEK_SET);
	fread(buffer, 1, 512, fp);
	fclose(fp);
	return 0;
}


void pralt_32Sector(alt_8* buffer){
	alt_32 i;
	for (i=0; i<512; i++){
		printf("%02X  ", buffer[i] & 0xff);
		if ((i+1) % 8 == 0){
			printf("\n");
		}
	}
	printf("\n");


}

int main(void){
	alt_8 buffer[512];
	alt_32 result = sd_readSector(0, buffer);
	if (result != 0){
		printf("Unable to read sector\n");
		return -1;
	}
	pralt_32Sector(buffer);
	return 0;
}
