#ifndef SPI_SD_H_
#define SPI_SD_H_
#include "altera_avalon_spi_regs.h"
#include "altera_avalon_spi.h"
#include "system.h"
#include "newtypes.h"
#include <stdio.h>

/*
typedef struct  {
    eint32      sectorCount;
}hwInterface;

esint8 if_initInterface(hwInterface* file);
*/

esint8 if_initInterface(void);
esint8 sd_readSector(euint32 address, euint8* buf);
esint8 sd_writeSector(euint32 address, euint8* buf);

#endif /*SPI_SD_H_*/
