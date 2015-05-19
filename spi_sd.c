#include "spi_sd.h"
/**************************************************************************/
static void if_spiInit(void) {
	IOWR_ALTERA_AVALON_SPI_SLAVE_SEL(SPI_0_BASE, 1 );
	printf("\nspi ini\n");
}

/**************************************************************************/
static euint8 if_spiSend(euint8 outgoing) {
	euint8  SD_Data=0,status;
	
	/* Set the SSO bit (force chipselect) */
	IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0x400);

	do{
        status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);//get status
	} while (((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0 ) &&
            (status & ALTERA_AVALON_SPI_STATUS_RRDY_MSK) == 0);
	/* wait till transmit and ready ok */

	IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, outgoing);
    
	/* Wait until the interface has finished transmitting */
	do{status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);}
	while ((status & ALTERA_AVALON_SPI_STATUS_TMT_MSK) == 0);

	/* reciver ready */
	if (((status & 0x80) != 0) ){
    	    SD_Data = IORD_ALTERA_AVALON_SPI_RXDATA(SPI_0_BASE);
	}
	else{
		printf("\n no recive after transmit");
	}
 
	IOWR_ALTERA_AVALON_SPI_SLAVE_SEL(SPI_0_BASE, 1);
	IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0);
 
	if( (status & 0x100) !=0)
    	printf("\n error in spi error in spi");
  
	return (SD_Data);
}
/**************************************************************************/
static euint8 if_spi_sendbuf(euint8 *buf, euint16 len) {
    euint8  status;
    euint16 i;
    /* Set the SSO bit (force chipselect) */
    IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0x400);
    for(i=0;i<len;i++){
        do{
            status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);//get status
        }while ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0 );
        /* wait till transmit and ready ok */
        IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, buf[i]);
    }

    do{
        status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);//get status
    }while ((status & ALTERA_AVALON_SPI_STATUS_TMT_MSK) == 0 );
    IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0);
    return (0);
}

/*****************************************************************************/
static euint8 if_spi_readbuf(euint8 *buf, euint16 len){
    euint8  status;
    euint16 i;
    /* Set the SSO bit (force chipselect) */
    IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0x400);
    /* Clear RX reg*/
    buf[0] = IORD_ALTERA_AVALON_SPI_RXDATA(SPI_0_BASE);
    /* wait till transmit and ready ok */
    do{
        status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);//get status
    }while ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0 );
    
    for(i=0;i<len;i++){
        IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, 0xFF);
        /* Wait until the interface has finished transmitting */
        while ((IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE) & ALTERA_AVALON_SPI_STATUS_RRDY_MSK) == 0);
        /* reciver ready */
        buf[i] = IORD_ALTERA_AVALON_SPI_RXDATA(SPI_0_BASE);
    }
    IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_0_BASE, 0);
    return (0);
}
/**************************************************************************/
static void sd_Command(euint8 cmd, euint16 paramx, euint16 paramy) {
	if_spiSend(0xff);
	if_spiSend(0x40 | cmd);
	if_spiSend((euint8) (paramx >> 8)); /* MSB of parameter x */
	if_spiSend((euint8) (paramx)); /* LSB of parameter x */
	if_spiSend((euint8) (paramy >> 8)); /* MSB of parameter y */
	if_spiSend((euint8) (paramy)); /* LSB of parameter y */
	if_spiSend(0x95); /* Checksum (should be only valid for first command (0) */
	if_spiSend(0xff); /* eat empty command - response */
}
/*****************************************************************************/
static euint8 sd_Resp8b(void){
	euint8 i, resp;
	/* Respone will come after 1 - 8 pings */
	for(i=0;i<8;i++){
		resp = if_spiSend(0xff);
		if(resp != 0xff)
			return(resp);
	}
	return(resp);
}
/*****************************************************************************/
static euint16 sd_Resp16b(void){
	euint16 resp = ( sd_Resp8b() << 8 ) & 0xff00;
	resp |= if_spiSend(0xff);
	return(resp);
}
/*****************************************************************************/
static void sd_Resp8bError(euint8 value){
	switch(value){
		case 0x40:
			printf("Argument out of bounds.\n");
			break;
		case 0x20:
			printf("Address out of bounds.\n");
			break;
		case 0x10:
			printf("Error during erase sequence.\n");
			break;
		case 0x08:
			printf("CRC failed.\n");
			break;
		case 0x04:
			printf("Illegal command.\n");
			break;
		case 0x02:
			printf("Erase reset (see SanDisk docs p5-13).\n");
			break;
		case 0x01:
			printf("Card is initialising.\n");
			break;
		default:
			printf("Unknown error 0x%x (see SanDisk docs p5-13).\n",value);
			break;
	}
}
/*****************************************************************************/
esint8 sd_Init(void){
	esint16 i=100;
	euint8 resp;
	/* Try to send reset command up to 100 times */
	do{
		sd_Command(0, 0, 0);
		resp=sd_Resp8b();
	} while(resp!=1 && i--);
	
	if(resp!=1){
		if(resp==0xff){
			return(-1);
		}
		else {
			sd_Resp8bError(resp);
			return(-2);
		}
	}
	/* Wait till card is ready initialising (returns 0 on CMD1) */
	/* Try up to 32000 times. */
	i=32000;
	do{
		sd_Command(1, 0, 0);
		resp=sd_Resp8b();
		if(resp!=0)
			sd_Resp8bError(resp);
	} while(resp==1 && i--);
	
	if(resp!=0){
		sd_Resp8bError(resp);
		return(-3);
	}
	printf("Card initialized successfully\n");
	return(0);
}

/****************************************************************************/
esint8 sd_State(void){
	eint16 value;	
	sd_Command(13, 0, 0);
	value=sd_Resp16b();
	switch(value) {
		case 0x000:
			return(1);
			break;
		case 0x0001:
			printf("Card is Locked.\n");
			break;
		case 0x0002:
			printf("WP Erase Skip, Lock/Unlock Cmd Failed.\n");
			break;
		case 0x0004:
			printf("General / Unknown error -- card broken?.\n");
			break;
		case 0x0008:
			printf("Internal card controller error.\n");
			break;
		case 0x0010:
			printf("Card internal ECC was applied, but failed to correct the data.\n");
			break;
		case 0x0020:
			printf("Write protect violation.\n");
			break;
		case 0x0040:
			printf("An invalid selection, sectors for erase.\n");
			break;
		case 0x0080:
			printf("Out of Range, CSD_Overwrite.\n");
			break;
		default:
			if(value>0x00FF)
				sd_Resp8bError((euint8) (value>>8));
			else
				printf("Unknown error: 0x%x (see SanDisk docs p5-14).\n",value);
			break;
	}
	return(-1);
}
/****************************************************************************/

/* ****************************************************************************/
/* WAIT ?? -- FIXME
 * CMDWRITE
 * WAIT
 * CARD RESP
 * WAIT
 * DATA BLOCK OUT
 *      START BLOCK
 *      DATA
 *      CHKS (2B)
 * BUSY...
*/

esint8 sd_writeSector(euint32 address, euint8* buf){
	euint32 place;
	euint16 t=0;	
	//DBG((TXT("Trying to write %u to sector %u.\n"),(void *)&buf,address));
	place=512*address;
	sd_Command(24,(euint16) (place >> 16), (euint16) place);
	sd_Resp8b(); // Card response
	if_spiSend(0xfe); // Start block 
    if_spi_sendbuf(buf,512);
	if_spiSend(0xff); // Checksum part 1 
	if_spiSend(0xff); // Checksum part 2 
	if_spiSend(0xff);
	while(if_spiSend(0xff)!=0xff){
		t++;
	}
	//DBG((TXT("Nopp'ed %u times.\n"),t));
	return(0);
}
/*****************************************************************************/

/* ****************************************************************************
 * WAIT ?? -- FIXME
 * CMDCMD
 * WAIT
 * CARD RESP
 * WAIT
 * DATA BLOCK IN
 * 		START BLOCK
 * 		DATA
 * 		CHKS (2B)
 */
esint8 sd_readSector(euint32 address, euint8* buf){
	euint8 cardresp, firstblock;
	euint16 fb_timeout=0xffff;
	euint32 place;
	//DBG((TXT("sd_readSector::Trying to read sector %u and store it at %p.\n"),address,&buf[0]));
	place=address<<9;
	sd_Command(17, (euint16) (place >> 16), (euint16) place);
	cardresp=sd_Resp8b(); // Card response  
	// Wait for startblock 
	do {
		firstblock=sd_Resp8b(); 
    } while(firstblock==0xff && fb_timeout--);

	if(cardresp!=0x00 || firstblock!=0xfe){
		sd_Resp8bError(firstblock);
		return(-1);
	}
    if_spi_readbuf(buf,512); 
	// Checksum (2 byte) - ignore for now 
	if_spiSend(0xff);
	if_spiSend(0xff);
	return(0);
}
/*****************************************************************************/

/* ***************************************************************************
 calculates size of card from CSD 
 (extension by Martin Thomas, inspired by code from Holger Klabunde)
*/
static euint32 sd_getDriveSize(void){
	euint8 cardresp, i, by, iob[16];
	euint16 c_size, c_size_mult, read_bl_len; 
	
	sd_Command(9,0,0);
	do {
		cardresp = sd_Resp8b();
	} while ( cardresp != 0xFE );

	printf("CSD:");
	for( i=0; i<16; i++) {
		iob[i] = sd_Resp8b();
		printf(" %02x", iob[i]);
	}
	printf("\n");

	if_spiSend(0xff);
	if_spiSend(0xff);

	c_size = iob[6] & 0x03; // bits 1..0
	c_size <<= 10;
	c_size += (euint16)iob[7]<<2;
	c_size += iob[8]>>6;

	by = iob[5] & 0x0F;
	read_bl_len = 1;
	read_bl_len <<= by;

	by = iob[9] & 0x03;
	by <<= 1;
	by += iob[10] >> 7;
	
	c_size_mult = 1;
	c_size_mult <<= (2+by);
	
	return (euint32)(c_size+1) * (euint32)c_size_mult * (euint32)read_bl_len;
}
/* ***********************************************************************/
//esint8 if_initInterface(hwInterface* file) {
esint8 if_initInterface(void) {
    euint32 size;
    if_spiInit();
    if(sd_Init()<0) {
        printf("Card failed to initialize.\n");
        return(-1);
    }
    if(sd_State()<0){
        printf("Card didn't return the ready state.\n");
        return(-2);
    }    
    //file->sectorCount= sd_getDriveSize()>>9; /* FIXME ASAP!! */
    //printf("Card size:%d\n",size);
    printf("Init done...\n");
    return(0);
}
