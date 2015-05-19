/* Tests FATKINS.h */

#include "FATKINS.h"
#include "alt_types.h"
#include "spi_sd.h"
#include <stdio.h>

#define SECTOR_SIZE 512

/*alt_32 sd_readSector(alt_32 address, alt_8* buffer){
	FILE* fp = fopen("/Users/nicholashobbs/Desktop/SD.dmg", "r");
	if (fp == NULL){
		return -1;
	}
	
	fseek(fp, address*512, SEEK_SET);
	fread(buffer, 1, 512, fp);
	fclose(fp);
	return 0;
}*/


void printSector(alt_u8* buffer){
	alt_32 i;
	for (i=0; i<512; i++){
		//printf("%02X  ", buffer[i] & 0xff);
		printf("%c", buffer[i]);
		if ((i+1) % 8 == 0){
			printf("\n");
		}
	}
	printf("\n");


}

/*int main(void){
	alt_8 buffer[512];
	alt_32 result = sd_readSector(0, buffer);
	if (result != 0){
		printf("Unable to read sector\n");
		return -1;
	}
	printSector(buffer);
	return 0;
}*/

alt_32 extract_little(alt_u8* buffer, alt_32 size){
	alt_32 sum = 0;
	alt_32 i;
	for (i=0; i<size; i++){
		sum += buffer[i] << i*8;
	}
	return sum;
}


int main(void){
	printf("%x\n", if_initInterface());
	alt_u8 buffer[SECTOR_SIZE];
	struct masterBootRecord* MBR;
	MBR = (struct masterBootRecord*) buffer;
	sd_readSector(0, buffer);
	printSector(buffer);

	struct partitionTable* partition1;
	partition1 = MBR->partitionTable1;

	printf("typecode: %x\n", partition1->typeCode);
	printf("LBA_begin: %d\n", extract_little(partition1->LBA_begin,4));
	printf("Num_sectors: %d\n", extract_little(partition1->Num_sectors,4));

	/* For a single partition */

	alt_32 bootSector = extract_little(partition1->LBA_begin,4);
	alt_32 numSectors = extract_little(partition1->Num_sectors,4);

	sd_readSector(bootSector, buffer);
	//printSector(buffer);

	struct bootSector* bootSec = (struct bootSector*) buffer;

	alt_32 num_reserved_sectors = extract_little(bootSec->numReservedSectors,2);
	alt_32 startingSectorOfFAT = bootSector + num_reserved_sectors;
	alt_32 numOfFATs = bootSec->numFileAllocTables;
	alt_32 sectorsPerFAT = extract_little(bootSec->sectorsPerFAT,4);
	alt_32 firstClusterStart = startingSectorOfFAT + numOfFATs * sectorsPerFAT;
	alt_32 sectorsPerCluster = bootSec->sectorsPerCluster;
	alt_32 rootClusterStart = extract_little(bootSec->clusterNumOfRootDir, 4);

	printf("Num res sectors: %d\n", num_reserved_sectors);
	printf("starting sector of FAT: %d\n", startingSectorOfFAT);
	printf("numofFATs: %d\n", numOfFATs);
	printf("sectorsPerFat: %d\n", sectorsPerFAT);
	printf("firstClusterStart: %d\n", firstClusterStart);
	printf("sectorsPerCluster: %d\n", sectorsPerCluster);
	printf("rootClusterStart: %d\n", rootClusterStart);

	/* for a cluster num */
	alt_32 cluster_num = rootClusterStart;
	alt_32 clusterFirstSector = firstClusterStart + sectorsPerCluster * (cluster_num-2);

	sd_readSector(clusterFirstSector, buffer);
	printSector(buffer);


	return 0;
}
