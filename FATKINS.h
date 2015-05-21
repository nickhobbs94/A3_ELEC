/* FATKINS: Walk along a FAT32 file system */
#ifndef FATKINS_H
#define FATKINS_H

#include <stdio.h>
#include "alt_types.h"

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

typedef struct masterBootRecord{
	alt_u8 bootCode[446];
	alt_u8 partitionTable1[16];
	alt_u8 partitionTable2[16];
	alt_u8 partitionTable3[16];
	alt_u8 partitionTable4[16];
	alt_u8 signature[2];
} masterBootRecord;

typedef struct partitionTable{
	alt_u8 bootFlag;
	alt_u8 CHS_begin[3];
	alt_u8 typeCode;
	alt_u8 CHS_end[3];
	alt_u8 LBA_begin[4];
	alt_u8 Num_sectors[4];
} partitionTable;

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


/* --------------------------- extracted structures --------------------------- */


/* Boot sector important info structure */
typedef struct EmbeddedBootSector{
	alt_32 num_reserved_sectors;
	alt_32 startingSectorOfFAT;
	alt_32 numOfFATs;
	alt_32 sectorsPerFAT;
	alt_32 firstClusterStart;
	alt_32 sectorsPerCluster;
	alt_32 rootClusterStart;
} EmbeddedBootSector;

/* Struct with everything important about a partition */
typedef struct EmbeddedPartition{
	alt_u8 typeCode; // is it FAT32?
	alt_32 bootSectorAddress;
	EmbeddedBootSector bootSector;
} EmbeddedPartition;

typedef struct FileSystemInfo{
	EmbeddedPartition partition; // Sector address of current partition
} FileSystemInfo;

/* Struct with everything important about the file system */
typedef struct EmbeddedFileSystem{
	alt_u8 numberOfPartitions;
	alt_32 addressesOfPartitions;
	FileSystemInfo myFs;  // current partition
} EmbeddedFileSystem;

typedef struct File{
	alt_u8 FileName[12]; // 8 (name) + 3 (extension) + 1 (null terminate)
	alt_32 FileSize;
	alt_u8 Attribute;
} File;

typedef struct DirList{
	File currentEntry;
} DirList;


#endif

