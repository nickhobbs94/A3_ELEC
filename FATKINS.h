/* FATKINS: Walk along a FAT32 file system */
#ifndef FATKINS_H
#define FATKINS_H

#include <stdio.h>
#include "alt_types.h"
#include "conversions.h"
#include "altstring.h"

#define   NUM_FAT_PARTITIONS  4 

/* Special codes for file entries in directory sector */
#define   END_OF_DIR         0x00
#define   INIT_BYTE_IS_E5    0x05
#define   DOT_ENTRY          0x2e
#define   ENTRY_IS_ERASED    0xe5

/* File attributes bitmask */
/* example usage: if (attribute & HIDDEN); */
#define   READONLY       0b00000001
#define   HIDDEN         0b00000010
#define   SYSTEM         0b00000100
#define   VOLUME_LABEL   0b00001000
#define   SUBDIRECTORY   0b00010000
#define   ARCHIVE        0b00100000
#define   DEVICE         0b01000000
#define   UNUSED         0b10000000
#define   LONGFILENAME   0x0f


/* Long file name first byte (sequence number) */
#define  DELETED_ENTRY     8
#define  LAST_PART_LFN     4
#define  OTHER_PART_LFN    0

/* --------------------------- raw buffer structures --------------------------- */

typedef struct partitionTable{
	alt_u8 bootFlag;
	alt_u8 CHS_begin[3];
	alt_u8 typeCode;
	alt_u8 CHS_end[3];
	alt_u8 LBA_begin[4];
	alt_u8 Num_sectors[4];
} partitionTable;

typedef struct masterBootRecord{
	alt_u8 bootCode[446];
	partitionTable partition_tables[4];
	alt_u8 signature[2];
} masterBootRecord;

typedef struct bootSector{
	alt_u8 jumpInstruction[3];
	alt_u8 OEM_Name[8];
	alt_u8 bytesPerSector[2];
	alt_u8 sectorsPerCluster;
	alt_u8 numReservedSectors[2];
	alt_u8 numFileAllocTables;
	alt_u8 unused0[2];
	alt_u8 numSectors[2];  // Nonzero if < 2^16
	alt_u8 mediaDescriptor;
	alt_u8 logicalSectorsPerFAT[2]; // zero for fat32
	alt_u8 unused1[8];
	alt_u8 numSectorsExtra[4];  // num sectors > 2^16
	alt_u8 sectorsPerFAT[4];
	alt_u8 unused2[4];
	alt_u8 clusterNumOfRootDir[4]; // mostly 2
	alt_u8 unused3[42];
	alt_u8 OS_bootCode[420]; // blazeit
	alt_u8 signature[2];
} bootSector;

typedef struct FAT_infoSector{
	alt_u8 signature0[4];
	alt_u8 unused0[480];
	alt_u8 signature1[4];
	alt_u8 numFreeClusters[4];
	alt_u8 numMostRecentAllocCluster[4];
	alt_u8 unused1[14];
	alt_u8 signature2[2];
} FAT_infoSector;

typedef struct directoryEntry{      // File entry in the sector of a directory
	alt_u8 fileName[8];             // First byte can be special codes
	alt_u8 fileExtenstion[3];
	alt_u8 fileAttributes;
	alt_u8 reserved;
	alt_u8 ms_timeCreation;         // millisecond resolution of time creation
	alt_u8 hms_timeCreation[2];     // hours minutes seconds of creation time
	alt_u8 ymd_timeCreation[2];     // year month day of creation time
	alt_u8 ymd_lastAccessDate[2];   // year month day of last access date
	alt_u8 startCluster_msb[2];     // most significant bits of the start cluster
	alt_u8 hms_lastModifiedDate[2]; // hours minutes seconds of last modify time
	alt_u8 ymd_lastModifiedDate[2]; // year month day last modified date
	alt_u8 startCluster_lsb[2];     // least significant bits of start cluster
	alt_u8 fileSizeInBytes[4];
} directoryEntry;


/* --------------------------- extracted structures and ther init functions --------------------------- */





/* --------------------------- Boot sector important info structure --------------------------- */

typedef struct EmbeddedBootSector{
	alt_32 address;
	alt_16 num_reserved_sectors;
	alt_32 startingSectorOfFAT;
	alt_16 numOfFATs;
	alt_32 sectorsPerFAT;
	alt_32 firstClusterStart;
	alt_32 sectorsPerCluster;
	alt_32 rootClusterStart;
} EmbeddedBootSector;

EmbeddedBootSector newEmbeddedBootSector(alt_u8* buffer, alt_32 bootSectorAddress){
	EmbeddedBootSector newBootSector;
	bootSector* rawBootSector = (bootSector*) buffer;

	newBootSector.address = bootSectorAddress;
	newBootSector.num_reserved_sectors = extract_little(rawBootSector->numReservedSectors, 2);
	newBootSector.startingSectorOfFAT = bootSectorAddress + newBootSector.num_reserved_sectors; 
	newBootSector.numOfFATs = rawBootSector->numFileAllocTables;
	newBootSector.sectorsPerFAT = extract_little(rawBootSector->sectorsPerFAT, 4);
	newBootSector.firstClusterStart = newBootSector.startingSectorOfFAT + newBootSector.numOfFATs * newBootSector.sectorsPerFAT;
	newBootSector.sectorsPerCluster = rawBootSector->sectorsPerCluster;
	newBootSector.rootClusterStart = extract_little(rawBootSector->clusterNumOfRootDir, 4);
	return newBootSector;
}

/* --------------------------- Struct with everything important about a partition --------------------------- */
typedef struct EmbeddedPartition{
	alt_u8 typeCode; // is it FAT32?
	alt_32 bootSectorAddress;
	alt_32 numSectors;
} EmbeddedPartition;

EmbeddedPartition newEmbeddedPartition(alt_u8* buffer){
	EmbeddedPartition newPartition;
	partitionTable* rawTable = (partitionTable*) buffer;
	newPartition.typeCode = rawTable->typeCode;
	newPartition.bootSectorAddress = extract_little(rawTable->LBA_begin, 4);
	newPartition.numSectors = extract_little(rawTable->Num_sectors, 4);

	return newPartition;
}

/* --------------------------- File System Info struct (used for reading, etc) --------------------------- */
typedef struct FileSystemInfo{
	alt_u8 partition_number; // offset in EmbeddedFileSystem->partitions[]
	EmbeddedPartition partition; // current partition
	EmbeddedBootSector bootSector; // boot sector of current partition
} FileSystemInfo;

/* --------------------------- Embedded File System --------------------------- */
typedef struct EmbeddedFileSystem{
	alt_u8 populatedPartitionCount;
	EmbeddedPartition partitions[NUM_FAT_PARTITIONS];
	FileSystemInfo myFs;  // current partition's boot codes etc.
} EmbeddedFileSystem;

/* -------------- Other structs ---------------- */

typedef struct File{
	alt_8 fileName[12]; // 8 (name) + 3 (extension) + 1 (null terminate)
	alt_32 startCluster;
	alt_32 startSector;
	alt_32 startSectorOfFAT;
	alt_32 fileSize;
	alt_32 currentPosition; // how much of the file has been read?
	alt_u8 attributes;
} File;

File newFile(alt_u8* buffer, alt_32 offset){
	File file;
	directoryEntry* rawEntry = (directoryEntry*) buffer + offset;

	file.attributes = rawEntry->fileAttributes;
	/* Null terminate by nulling file attributes */
	rawEntry->fileAttributes = '\0';
	altstrcpy((alt_8*)file.fileName, (alt_8*)rawEntry->fileName);
	file.startCluster = (extract_little(rawEntry->startCluster_msb, 2)<<2) + (extract_little(rawEntry->startCluster_lsb, 2));

	return file;
}

typedef struct DirList{
	File currentEntry;
	alt_32 startingCluster;
} DirList;


#endif

