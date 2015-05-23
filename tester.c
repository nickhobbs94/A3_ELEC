/* Tests FATKINS.h */

#include "FATKINS.h"
#include "alt_types.h"
//#include "spi_sd.h"
#include <stdio.h>
#include <stdlib.h>
#include "altstring.h"

#define SECTOR_SIZE 512
#define CLUSTERS_IN_FATSECTOR 128
#define BYTES_PER_ENTRY 4
#define DIRECTORY_ENTRIES_PER_SECTOR 16

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


/*alt_32 getBootSector(alt_32 bootSectorAddress, EmbeddedBootSector* bootSectorPtr){
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
}*/

EmbeddedBootSector getBootSector(alt_32 bootSectorAddress){
	alt_u8 buffer[SECTOR_SIZE];
	sd_readSector(bootSectorAddress, buffer);
	return newEmbeddedBootSector(buffer, bootSectorAddress);
}

alt_32 getFATentry(alt_32 clusterNum, alt_32 startingSectorOfFAT){
	alt_u8 buffer[SECTOR_SIZE];
	alt_32 FATSector = startingSectorOfFAT + clusterNum / CLUSTERS_IN_FATSECTOR;
	alt_32 clusterByteOffset = BYTES_PER_ENTRY * (clusterNum % CLUSTERS_IN_FATSECTOR); // 4 bytes for FAT32 entries
	sd_readSector(FATSector, buffer);
	alt_u8* FAT_Entry = buffer + clusterByteOffset;
	return extract_little(FAT_Entry, BYTES_PER_ENTRY);
}


alt_32 efs_init_partitions(EmbeddedFileSystem* fileSystem){
	alt_u8 buffer[SECTOR_SIZE];
	masterBootRecord* MBR;
	MBR = (masterBootRecord*) buffer;
	sd_readSector(0, buffer);
	partitionTable* partition;

	alt_u8 partitionCount = 0;
	alt_32 i;
	for (i=0; i<NUM_FAT_PARTITIONS; i++){
		partition = &(MBR->partition_tables[i]);
		if (partition->typeCode) partitionCount++;
		fileSystem->partitions[i] = newEmbeddedPartition((alt_u8*)partition);
	}
	fileSystem->populatedPartitionCount = partitionCount;
	return 0;
}

void printEFS(EmbeddedFileSystem* fileSystem){
	printf("           Partition 0:\n");
	printf("typeCode:                       %x\n", fileSystem->partitions[0].typeCode);
	printf("bootSectorAddress:              %x\n", fileSystem->partitions[0].bootSectorAddress);
	printf("numSectors:                     %x\n", fileSystem->partitions[0].numSectors);
	printf("partitionCount:                 %x\n", fileSystem->populatedPartitionCount);

	printf("\n");
	printf("           Boot sector:\n");
	printf("address:                        %x\n", fileSystem->myFs.bootSector.address);
	printf("res sectors:                    %d\n", fileSystem->myFs.bootSector.num_reserved_sectors);
	printf("startOfFAT:                     %x\n", fileSystem->myFs.bootSector.startingSectorOfFAT);
	printf("numOfFATs:                      %x\n", fileSystem->myFs.bootSector.numOfFATs);
	printf("sectorsPerFAT:                  %d\n", fileSystem->myFs.bootSector.sectorsPerFAT);
	printf("firstCluster:                   %d\n", fileSystem->myFs.bootSector.firstClusterStart);
	printf("sectorsPerClust:                %d\n", fileSystem->myFs.bootSector.sectorsPerCluster);
	printf("rootClusterStart:               %d\n", fileSystem->myFs.bootSector.rootClusterStart);

	printf("\n");
}

alt_32 efs_init(EmbeddedFileSystem* fileSystem, alt_8 deviceName[]){
	//printf("%x\n", if_initInterface());   // if on NIOS board uncomment this line
	/* Load partition info into EmbeddedFileSystem struct */
	efs_init_partitions(fileSystem);

	/* TODO: check which device to mount and change partition depending on that (deviceName[]) */
	/* for now, assume mounting first partition */
	fileSystem->myFs.partition_number = 0;
	fileSystem->myFs.partition = fileSystem->partitions[0];

	fileSystem->myFs.bootSector = getBootSector(fileSystem->partitions[0].bootSectorAddress);
	printEFS(fileSystem);

	return 0;
}

File searchDirectory(alt_32 directoryCluster, alt_8 filename[], FileSystemInfo* myFs){
	File foundFile;
	alt_u8 buffer[SECTOR_SIZE];
	//TODO: format string properly
	alt_32 entry = 0; // current entry in the directory
	alt_32 startingSector = (directoryCluster  - 2) * myFs->bootSector.sectorsPerCluster + myFs->bootSector.firstClusterStart;
	alt_32 FAT_result, currentSector;
	do {
		currentSector = startingSector + entry / DIRECTORY_ENTRIES_PER_SECTOR;
		sd_readSector(currentSector, buffer);
		foundFile = newFile(buffer, entry % DIRECTORY_ENTRIES_PER_SECTOR);
		foundFile.startSector = (foundFile.startCluster  - 2) * myFs->bootSector.sectorsPerCluster + myFs->bootSector.firstClusterStart;
		foundFile.startSectorOfFAT = myFs->bootSector.startingSectorOfFAT;
		if (altstrcmp(foundFile.fileName, filename) == 0){
			return foundFile;
		}

		FAT_result = getFATentry(directoryCluster, myFs->bootSector.startingSectorOfFAT);
		entry++;
	} while (foundFile.fileName[0] != END_OF_DIR);

	return foundFile;
}

//TODO find cluster number from filename
File findFile(FileSystemInfo* myFs, alt_8 filename[]){
	printf("%s\n",filename);
	string_replace(filename, 'e', '.', 1, -1);
	printf("%s\n",filename);
	printf("%d\n", altstrcount(filename, 'e'));
	return searchDirectory(2, filename, myFs);
}

alt_32 file_fopen(File* file, FileSystemInfo* myFs, alt_8 filenameLiteral[], alt_u8 mode){
	alt_8* filename = malloc(altstrlen(filenameLiteral));
	if (filename == NULL) return -1;
	altstrcpy(filename, filenameLiteral);
	*file = findFile(myFs, filename);
	free(filename);
	return 0;
}

alt_32 file_read(File* file, alt_32 length, alt_u8 buffer[]){
	printf("FAT ENTRY OF CLUSTER %x: %x\n", file->startCluster, getFATentry(file->startCluster, file->startSectorOfFAT));
	alt_u8 buf[SECTOR_SIZE];
	//sd_readSector(file->startSector, buf);
	//printSector(buf);
	
	alt_32 i;
	for (i=0; i<length && i<file->fileSize; i++){
		if (i%SECTOR_SIZE == 0){
			sd_readSector(file->startSector + i/SECTOR_SIZE + file->currentPosition, buf);
			printf("%d\n", i);
		}
		buffer[i] = buf[i%SECTOR_SIZE];
	}
	printf("%d\n", file->fileSize);
	return 0;
}

alt_32 file_fclose(File* file){
	return 0;
}

int main(void){
	EmbeddedFileSystem efsl;
	alt_32 check = efs_init(&efsl, "/dev/sda");

	if (check==0){
		printf("Filesystem correctly initialized\n");
	} else {
		printf("Could not init filesystem\n");
		return -1;
	}

	File file;

	alt_u8 buffer[100];
	check = file_fopen(&file, &(efsl.myFs), "README  TXT", 'r');

	if (check != 0){
		printf("Could not open file\n");
		return -1;
	}

	check = file_read(&file, 100, buffer);
	printf("Check: %d\n", check);
	printf("Contents:\n%s\n", buffer);

	//check = searchDirectory(2, "README  TXT", &(efsl.myFs)).startCluster;
	//printf("README.TXT is at cluster %x\n", check);

	file_fclose(&file);


	return 0;
}

