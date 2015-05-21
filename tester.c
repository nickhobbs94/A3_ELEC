/* Tests FATKINS.h */

#include "FATKINS.h"
#include "alt_types.h"
//#include "spi_sd.h"
#include <stdio.h>
#include "altstring.h"

#define SECTOR_SIZE 512
#define CLUSTERS_IN_FATSECTOR 128
#define BYTES_PER_ENTRY 4

/* Only when not using NIOS board */
alt_32 sd_readSector(alt_32 address, alt_u8* buffer){
	FILE* fp = fopen("/Users/nicholashobbs/Desktop/SD.dmg", "r");
	//FILE* fp = fopen("/Users/nicholashobbs/Downloads/Torrent/2015-05-05-raspbian-wheezy.img", "r");
	if (fp == NULL){
		return -1;
	}
	
	fseek(fp, address*512, SEEK_SET);
	fread(buffer, 1, 512, fp);
	fclose(fp);
	return 0;
}


void printSector(alt_u8* buffer){
	alt_32 i;
	for (i=0; i<512; i++){
		printf("%02X  ", buffer[i] & 0xff);
		//printf("%c", buffer[i]);
		if ((i+1) % 8 == 0){
			printf("\n");
		}
	}
	printf("\n");


}


alt_32 extract_little(alt_u8* buffer, alt_32 size){
	alt_32 sum = 0;
	alt_32 i;
	for (i=0; i<size; i++){
		sum += buffer[i] << i*8;
	}
	return sum;
}

alt_32 getBootSector(alt_32 bootSectorAddress, EmbeddedBootSector* bootSectorPtr){
	alt_u8 buffer[SECTOR_SIZE];

	sd_readSector(bootSectorAddress, buffer);
	//printSector(buffer);

	bootSector* bootSec = (bootSector*) buffer;

	bootSectorPtr->num_reserved_sectors = extract_little(bootSec->numReservedSectors,2);
	bootSectorPtr->startingSectorOfFAT = bootSectorAddress + bootSectorPtr->num_reserved_sectors;
	bootSectorPtr->numOfFATs = bootSec->numFileAllocTables;
	bootSectorPtr->sectorsPerFAT = extract_little(bootSec->sectorsPerFAT,4);
	bootSectorPtr->firstClusterStart = bootSectorPtr->startingSectorOfFAT + bootSectorPtr->numOfFATs * bootSectorPtr->sectorsPerFAT;
	bootSectorPtr->sectorsPerCluster = bootSec->sectorsPerCluster;
	bootSectorPtr->rootClusterStart = extract_little(bootSec->clusterNumOfRootDir, 4);

	return 0;
}

alt_32 getFATentry(alt_32 clusterNum, alt_32 startingSectorOfFAT){
	alt_u8 buffer[SECTOR_SIZE];
	alt_32 FATSector = startingSectorOfFAT + clusterNum / CLUSTERS_IN_FATSECTOR;
	alt_32 clusterByteOffset = BYTES_PER_ENTRY * (clusterNum % CLUSTERS_IN_FATSECTOR); // 4 bytes for FAT32 entries
	sd_readSector(FATSector, buffer);
	alt_u8* FAT_Entry = buffer + clusterByteOffset;
	return extract_little(FAT_Entry, BYTES_PER_ENTRY);
}

alt_u8 countPartitions(alt_u8* buffer){
	masterBootRecord* MasterBootRecord = (masterBootRecord*) buffer;
	alt_u8 count = 0;
	partitionTable* partition;

	partition = (partitionTable*) MasterBootRecord->partitionTable1;
	if (partition->typeCode) count++;
	partition = (partitionTable*) MasterBootRecord->partitionTable2;
	if (partition->typeCode) count++;
	partition = (partitionTable*) MasterBootRecord->partitionTable3;
	if (partition->typeCode) count++;
	partition = (partitionTable*) MasterBootRecord->partitionTable4;
	if (partition->typeCode) count++;

	return count;

}

alt_32 efs_init_partitions(EmbeddedFileSystem* fileSystem){
	alt_u8 buffer[SECTOR_SIZE];
	masterBootRecord* MBR;
	MBR = (masterBootRecord*) buffer;
	sd_readSector(0, buffer);

	partitionTable* partition;
	partition = (partitionTable*) MBR->partitionTable1;

	fileSystem->numberOfPartitions = countPartitions(buffer);
	return 0;
}

alt_32 efs_init(EmbeddedFileSystem* fileSystem, alt_8 deviceName[]){
	//printf("%x\n", if_initInterface());   // if on NIOS board uncomment this line

	/* Load partition info into EmbeddedFileSystem struct */
	efs_init_partitions(fileSystem);

	/* TODO: check which device to mount and change partition depending on that (deviceName[]) */
	/* for now, assume mounting first partition */

	return 0;
}

int main(void){
	EmbeddedFileSystem* efsl;
	alt_32 check = efs_init(efsl, "/dev/sda");

	if (check==0){
		printf("Filesystem correctly initialized\n");
	} else {
		printf("Could not init filesystem\n");
	}

	return 0;
}

