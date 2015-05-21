/* FATKINS: Walk along a FAT32 file system */
#ifndef FATKINS_H
#define FATKINS_H

#include <stdio.h>
#include "alt_types.h"

struct masterBootRecord{
	alt_8 bootCode[446];
	alt_8 partitionTable1[16];
	alt_8 partitionTable2[16];
	alt_8 partitionTable3[16];
	alt_8 partitionTable4[16];
	alt_8 signature[2];
};

struct partitionTable{
	alt_8 bootFlag;
	alt_8 CHS_begin[3];
	alt_8 typeCode;
	alt_8 CHS_end[3];
	alt_8 LBA_begin[4];
	alt_8 Num_sectors[4];
};

struct bootSector{
	alt_8 jumpInstruction[3];
	alt_8 OEM_Name[8];
	alt_8 bytesPerSector[2];
	alt_8 sectorsPerCluster;
	alt_8 numReservedSectors[2];
	alt_8 numFileAllocTables;
	alt_8 unused0[2];
	alt_8 numSectors[2];  // Nonzero if < 2^16
	alt_8 mediaDescriptor;
	alt_8 logicalSectorsPerFAT[2]; // zero for fat32
	alt_8 unused1[8];
	alt_8 numSectorsExtra[4];  // num sectors > 2^16
	alt_8 sectorsPerFAT[4];
	alt_8 unused2[4];
	alt_8 clusterNumOfRootDir[4]; // mostly 2
	alt_8 unused3[42];
	alt_8 OS_bootCode[420]; // blazeit
	alt_8 signature[2];
};

struct FAT_infoSector{
	alt_8 signature0[4];
	alt_8 unused0[480];
	alt_8 signature1[4];
	alt_8 numFreeClusters[4];
	alt_8 numMostRecentAllocCluster[4];
	alt_8 unused1[14];
	alt_8 signature2[2];
};

struct directory{
	//alt_8
};

#endif

