/* Tests FATKINS.h */
#ifndef EFS_H
#define EFS_H

#include "FATKINS.h"
#include "alt_types.h"
#include "spi_sd.h"
#include <stdio.h>
#include <stdlib.h>
#include "altstring.h"
#include "conversions.h"
//#include "SDDir.h"


#define SECTOR_SIZE 512
#define CLUSTERS_IN_FATSECTOR 128
#define BYTES_PER_ENTRY 4
#define DIRECTORY_ENTRIES_PER_SECTOR 16
#define MAX_FILE_NAME_SIZE 12


void printSector(alt_u8* buffer);
EmbeddedBootSector getBootSector(alt_32 bootSectorAddress);

alt_32 efs_init_partitions(EmbeddedFileSystem* fileSystem);
alt_32 getFATentry(alt_32 clusterNum, alt_32 startingSectorOfFAT);
void printEFS(EmbeddedFileSystem* fileSystem);
File searchDirectory(alt_32 directoryCluster, alt_8 rawfilename[], FileSystemInfo* myFs);
File findFile(FileSystemInfo* myFs, alt_8 filepath[]);
alt_32 file_fopen(File* file, FileSystemInfo* myFs, alt_8 filenameLiteral[], alt_u8 mode);
alt_32 file_read(File* file, alt_32 length, alt_u8 buffer[]);

alt_32 file_fclose(File* file);
alt_32 ls_getNext(DirList* list);
alt_32 ls_openDir(DirList* list,FileSystemInfo* myFs, alt_8* path);

/* Only when not using NIOS board */
/*alt_32 sd_readSector(alt_32 address, alt_u8* buffer){
	FILE* fp = fopen(OUR_SD_DIR_LOCATION, "r");
	//FILE* fp = fopen("/Users/nicholashobbs/Downloads/Torrent/2015-05-05-raspbian-wheezy.img", "r");
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
	alt_32 FATSector = startingSectorOfFAT + (clusterNum) / CLUSTERS_IN_FATSECTOR;
	alt_32 clusterByteOffset = BYTES_PER_ENTRY * ((clusterNum) % CLUSTERS_IN_FATSECTOR); // 4 bytes for FAT32 entries
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
	printf("bytesPerSector:                 %d\n", fileSystem->myFs.bootSector.bytesPerSector);

	printf("\n");
}

alt_32 efs_init(EmbeddedFileSystem* fileSystem, alt_8 deviceName[]){
	printf("%x\n", if_initInterface());   // if on NIOS board uncomment this line
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

File searchDirectory(alt_32 directoryCluster, alt_8 rawfilename[], FileSystemInfo* myFs){
	//printf("searchDirectory(%x,%s,%x)\n", directoryCluster, rawfilename, myFs);
	alt_8 filename[12];
	formatStringForFAT(rawfilename, filename);
	//printf("Looking for: %s from %s\n", filename, rawfilename);

	File foundFile;
	alt_u8 buffer[SECTOR_SIZE];
	alt_32 entry = 0; // current entry in the directory
	alt_32 startingSector = (directoryCluster  - 2) * myFs->bootSector.sectorsPerCluster + myFs->bootSector.firstClusterStart;
	alt_32 clusterSector = startingSector;
	alt_32 FAT_result, currentSector, currentCluster;
	currentCluster = directoryCluster;
	do {
		entry = 0;
		do {
			currentSector = clusterSector + entry / DIRECTORY_ENTRIES_PER_SECTOR;
			sd_readSector(currentSector, buffer);
			//printSector(buffer);
			//printf("%x\n", currentSector);
			foundFile = newFile(buffer, entry % DIRECTORY_ENTRIES_PER_SECTOR, myFs->bootSector.sectorsPerCluster, myFs->bootSector.firstClusterStart);
			foundFile.startSector = (foundFile.startCluster  - 2) * myFs->bootSector.sectorsPerCluster + myFs->bootSector.firstClusterStart;
			foundFile.startSectorOfFAT = myFs->bootSector.startingSectorOfFAT;
			if (altstrcmp(foundFile.FileName, filename) == 0){
				//printf("Found file's filename %s\n", foundFile.fileName);
				//printf("File name is %s\n", filename);
				return foundFile;
			}
			//printf("%s\n", foundFile.fileName);
			entry++;
		} while (foundFile.FileName[0] != END_OF_DIR && (currentSector-clusterSector < myFs->bootSector.sectorsPerCluster) );
		FAT_result = getFATentry(currentCluster, myFs->bootSector.startingSectorOfFAT);
		currentCluster = FAT_result;
		clusterSector = (FAT_result  - 2) * myFs->bootSector.sectorsPerCluster + myFs->bootSector.firstClusterStart;
		//printf("FAT result: %x\n", FAT_result);
		//printf("clusterSector: %d\n",clusterSector);
		//printf("currentSector: %d\n",currentSector);
	} while (foundFile.FileName[0] != END_OF_DIR && currentCluster < 0xffffff7);
	foundFile.FileName[0] = END_OF_DIR;
	printf("Filesize: %d\n", foundFile.FileSize);
	return foundFile;
}

File findFile(FileSystemInfo* myFs, alt_8 filepath[]){
	if(filepath[0] == '/'){
		filepath++;
	}
	alt_32 fileNamesCount = altstrcount(filepath, '/') + 1;
	alt_8 formattedName[MAX_FILE_NAME_SIZE];
	alt_8 numberOfWords;
	alt_32 clusterNumber = myFs->bootSector.rootClusterStart;
	File file;

	
	alt_8 **filenameArray = malloc(MAX_FILE_NAME_SIZE*fileNamesCount);
	numberOfWords = altsplitstring(filepath, filenameArray, '/');
	alt_8 *findFileName = filenameArray[numberOfWords-1];
	formatStringForFAT(findFileName, formattedName);


	alt_32 wordCount;
	for(wordCount = 0; wordCount < numberOfWords; ++wordCount){
		printf("%s\n", filenameArray[wordCount]);
		
		file = searchDirectory(clusterNumber, filenameArray[wordCount], myFs);

		if(file.FileName[0] == END_OF_DIR){
			printf("Error: unable to find file/folder: %s\n", filenameArray[wordCount]);
			printf("Error: File name in path is not correct \n");
			printf("file.FileName: %s\n", file.FileName);
			//printf("findFileName: %s\n", findFileName);
			printf("filenameArray[%d]: %s\n", wordCount ,filenameArray[wordCount]);
			printf("clusterNumber: %d\n" ,clusterNumber);
			printf("numberOfWords: %d\n" ,numberOfWords);
			break;
		} else if (file.Attribute & SUBDIRECTORY){
			clusterNumber = file.startCluster;
		} else if(altstrcmp(file.FileName, formattedName) != 0){
			printf("Error: File name in path is not correct \n");
			printf("file.FileName: %s\n", file.FileName);
			//printf("findFileName: %s\n", findFileName);
			printf("filenameArray[%d]: %s\n", wordCount ,filenameArray[wordCount]);
			printf("clusterNumber: %d\n" ,clusterNumber);
			printf("numberOfWords: %d\n" ,numberOfWords);
			break;
		}
	}
	free(filenameArray);
	printf("Filesize: %d\n", file.FileSize);
	return file; 
}

alt_32 file_fopen(File* file, FileSystemInfo* myFs, alt_8 filenameLiteral[], alt_u8 mode){
	*file = findFile(myFs, filenameLiteral);
	file->currentCluster = file->startCluster;
	printf("Filesize: %d\n", file->FileSize);
	return 0;
}

alt_32 file_read(File* file, alt_32 length, alt_u8 buffer[]){
	alt_32 i, bytesRead=0;
	alt_u8 sectorBuffer[SECTOR_SIZE];
	alt_32 currentSectorPosition;
	sd_readSector(file->currentSector, sectorBuffer);
	for (i=0; i<length; i++){
		if (file->currentCluster >= 0x0ffffff8){
			break;
		}
		file->currentClusterStartSector = file->firstClusterStart + file->sectorsPerCluster * (file->currentCluster - 2);
		//printf("currentclusterstartsec %d, first cluster start %d, sectors per cluster %d, current cluster %d", file->currentClusterStartSector, file->firstClusterStart , file->sectorsPerCluster , file->currentCluster);
		if (file->currentPosition >= file->FileSize){
			break;
		}
		file->currentSector = file->currentPosition/SECTOR_SIZE + file->currentClusterStartSector;
		if (file->currentPosition % SECTOR_SIZE == 0){
			sd_readSector(file->currentSector, sectorBuffer);
		}
		currentSectorPosition = file->currentPosition % SECTOR_SIZE;
		buffer[i] = sectorBuffer[currentSectorPosition];
		bytesRead++;
		file->currentPosition++;

		if (file->currentPosition % (file->sectorsPerCluster) == 0 && file->currentPosition != 0){
			file->currentCluster = getFATentry(file->currentCluster, file->startSectorOfFAT);
			printf("\t\t\t%d\n", file->currentCluster);
		}
	}
	return bytesRead;
}
alt_32 file_fclose(File* file){
	file->currentPosition = 0;
	file->currentCluster = file->firstClusterStart;
	return 0;
}

alt_32 ls_getNext(DirList* list){
	alt_u8 buffer[SECTOR_SIZE];
	alt_32 currentSector = (list->startingCluster - 2) * list->sectorsPerCluster + list->firstClusterStart;
	sd_readSector(currentSector, buffer);
	do {
		list->currentEntry = newFile(buffer, list->currentEntryNum, list->sectorsPerCluster, list->firstClusterStart);
	} while (list->currentEntry.FileName != END_OF_DIR && list->currentEntry.FileName == ENTRY_IS_ERASED);
	if (list->currentEntry.FileName[0] == END_OF_DIR){
		return -1;
	}
	list->currentEntryNum++;
	return 0;
}

alt_32 ls_openDir(DirList* list,FileSystemInfo* myFs, alt_8* path){
	File directoryObject;
	alt_32 check;
	check = file_fopen(&directoryObject, myFs, path, 'r');
	if (check!=0) return -1;
	list->currentEntryNum = 0;
	list->startingCluster = directoryObject.startCluster;
	list->currentPosition = directoryObject.currentPosition;
	list->startSectorOfFAT = directoryObject.startSectorOfFAT;
	list->startSector = directoryObject.startSector;
	list->sectorsPerCluster = directoryObject.sectorsPerCluster;
	list->firstClusterStart = directoryObject.firstClusterStart;
	return 0;
}
alt_32 file_write(File* file, alt_32 size, alt_8* buffer){
	return -1;
}

alt_32 fs_umount(FileSystemInfo* myFs){
	return -1;
}

alt_32 makedir(FileSystemInfo* myFs, alt_8* path){
	return -1;
}

alt_32 rmfile(FileSystemInfo* myFs, alt_8* path){
	return -1;
}


#endif


