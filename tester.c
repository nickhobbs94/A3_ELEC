/* Tests FATKINS.h */

#include "FATKINS.h"
#include "alt_types.h"
//#include "spi_sd.h"
#include <stdio.h>
#include <stdlib.h>
#include "altstring.h"
#include "SDDir.h"


#define SECTOR_SIZE 512
#define CLUSTERS_IN_FATSECTOR 128
#define BYTES_PER_ENTRY 4
#define DIRECTORY_ENTRIES_PER_SECTOR 16
#define MAX_FILE_NAME_SIZE 12



/* Only when not using NIOS board */
alt_32 sd_readSector(alt_32 address, alt_u8* buffer){
	FILE* fp = fopen(OUR_SD_DIR_LOCATION, "r");
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
	//printEFS(fileSystem);

	return 0;
}

File searchDirectory(alt_32 directoryCluster, alt_8 rawfilename[], FileSystemInfo* myFs){
	alt_8 filename[12];
	formatStringForFAT(rawfilename, filename);

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
			//printf("File name is %s\n", filename);
			return foundFile;
		}
		//printf("%s\n", foundFile.fileName);

		FAT_result = getFATentry(directoryCluster, myFs->bootSector.startingSectorOfFAT);
		entry++;
	} while (foundFile.fileName[0] != END_OF_DIR);

	return foundFile;
}

//TODO find cluster number from filename
File findFile(FileSystemInfo* myFs, alt_8 filepath[]){
	alt_32 fileNamesCount = altstrcount(filepath, '/');
	alt_8 numberOfWords;
	alt_32 clusterNumber = myFs->bootSector.rootClusterStart;
	File file;

	if(filepath[0] != '/'){
		fileNamesCount++;
	}

	alt_8 **filenameArray = malloc(MAX_FILE_NAME_SIZE*fileNamesCount);
	numberOfWords = altsplitstring(filepath, filenameArray, '/');
	alt_8 *findFileName = filenameArray[numberOfWords-1];

	// int i = 0;
	// for(; i < numberOfWords; ++i)
	// {
	// 	printf("%s\n", filenameArray[i]);
	// }

	alt_32 wordCount;
	for(wordCount = 0; wordCount < numberOfWords; ++wordCount){
		printf("%s\n", filenameArray[wordCount]);
		
		file = searchDirectory(clusterNumber, filenameArray[wordCount], myFs);
<<<<<<< HEAD


=======
		printf("%s\n", file.fileName);
>>>>>>> 61c9dcfd5991ef9b2efc53b9ec271fa374733018
		if(file.fileName[0] & END_OF_DIR){
			printf("Error: End of Directory\n");
			break;
		} else if (file.attributes & SUBDIRECTORY){
			clusterNumber = file.startCluster;
		} else if(altstrcmp(file.fileName, findFileName) != 0){
			printf("Error: File name in path is not correct \n");
			printf("file.fileName: %s\n", file.fileName);
			printf("findFileName: %s\n", findFileName);
			printf("filenameArray[%d]: %s\n", wordCount ,filenameArray[wordCount]);
			printf("clusterNumber: %d\n" ,clusterNumber);
			printf("numberOfWords: %d\n" ,numberOfWords);
			break;
		}
	}
	free(filenameArray);
	return file; 
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
	//printf("FAT ENTRY OF CLUSTER %x: %x\n", file->startCluster, getFATentry(file->startCluster, file->startSectorOfFAT));
	alt_u8 buf[SECTOR_SIZE];
	//sd_readSector(file->startSector, buf);
	//printSector(buf);
	
	alt_32 i;
	for (i=0; i+file->currentPosition<length && i+file->currentPosition<file->fileSize; i++){
		if (i%SECTOR_SIZE == 0){
			sd_readSector(file->startSector + i/SECTOR_SIZE + file->currentPosition, buf);
			//printf("reading %d\n", i);
		}
		buffer[i] = buf[i%SECTOR_SIZE];
	}
	file->currentPosition += i;
	//printf("%d\n", file->fileSize);
	return i;
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
<<<<<<< HEAD
	printf("\n~~WITHOUT SLASH~~\n");
	check = file_fopen(&file, &(efsl.myFs), "FOLDER/FILE    TXT", 'r');
	printf("\n~~WITH SLASH~~\n");
	check = file_fopen(&file, &(efsl.myFs), "/FOLDER/FILE    TXT", 'r');
=======
	check = file_fopen(&file, &(efsl.myFs), "/readme.txt", 'r');
>>>>>>> 61c9dcfd5991ef9b2efc53b9ec271fa374733018

	if (check != 0){
		printf("Could not open file\n");
		return -1;
	}
	do {
		check = file_read(&file, 100, buffer);
		alt_32 i;
		for (i=0; i<check; i++){
			printf("%c", buffer[i]);
		}
	} while (check != 0);

	//check = searchDirectory(2, "README  TXT", &(efsl.myFs)).startCluster;
	//printf("README.TXT is at cluster %x\n", check);

	file_fclose(&file);


	return 0;
}

