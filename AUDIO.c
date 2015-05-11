#include "alt_types.h"
#include <stdbool.h>
#include "altera_up_avalon_audio_regs_dgz.h"
#include "altera_up_avalon_audio_dgz.h"
#include "AUDIO.h"
#include "opencores_i2c.h"



#ifdef DEBUG_AUDIO
    #define AUDIO_DEBUG(x)    DEBUG(x)
#else
    #define AUDIO_DEBUG(x)
#endif

#define WM8731_ADDR_WRITE  0x34


alt_u16 vol_out_l = 0x7f;
alt_u16 vol_out_r = 0x7f;
alt_u16 vol_in_l;
alt_u16 vol_in_r;


/////////////////////////////////////////////////////////////////////////
///////////// Internal function prototype & const definition ////////////
/////////////////////////////////////////////////////////////////////////
bool aduio_RegWrite(alt_u8 reg_index, alt_u16 data16);
static alt_u16  reg_file[10];


/////////////////////////////////////////////////////////////////////////
///////////// External function body ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

bool AUDIO_Init(void)
{
bool bSuccess = true;

    // Set WM8731(Audio codec)
    I2C_init(I2C_MASTER_BASE,50000000u,350000);

    AUDIO_DEBUG(("[AUDIO] AUDIO_Init...\r\n"));

    if (bSuccess)
        bSuccess = aduio_RegWrite(15, 0x0000);  // reset

    if (bSuccess)
        bSuccess = aduio_RegWrite(9, 0x0000);  // inactive interface
       
    usleep(20*1000);
    if (bSuccess)
        bSuccess = aduio_RegWrite(0, 0x001A);  // Left Line In: set left line in volume
        
    if (bSuccess)
        bSuccess = aduio_RegWrite(1, 0x001A);  // Right Line In: set right line in volume
        
    if (bSuccess)
        bSuccess = aduio_RegWrite(2, vol_out_l);  // Left Headphone Out: set left line out volume
        
    if (bSuccess)
        bSuccess = aduio_RegWrite(3, vol_out_r);  // Right Headphone Out: set right line out volume
        
    if (bSuccess)
        bSuccess = aduio_RegWrite(4, 0x00D5);  // Analogue Audio Path Control: set mic as input and boost it, and enable dac
                
    if (bSuccess)
        bSuccess = aduio_RegWrite(5, 0x0000);  // Digital Audio Path Control: disable soft mute, ADC and DAC filter disabled

    if (bSuccess)
        bSuccess = aduio_RegWrite(6, 0);  // power down control: power on all

    if (bSuccess)
        bSuccess = aduio_RegWrite(7, 0x004E);  // I2S mode, left justified, iwl=32-bits, Enable Master Mode

    if (bSuccess)
        bSuccess = aduio_RegWrite(8, 0x0023);  // Sample rate,USB mode 44.1k

    if (bSuccess)
        bSuccess = aduio_RegWrite(9, 0x0001);  // active interface


    AUDIO_DEBUG(("[AUDIO] AUDIO_Init %s\r\n", bSuccess?"success":"fail"));



    return bSuccess;
         
}

bool AUDIO_InterfaceActive(bool bActive){
    bool bSuccess;
    bSuccess = aduio_RegWrite(9, bActive?0x0001:0x0000); 
    return bSuccess;
}

bool AUDIO_MicBoost(bool bBoost){
    bool bSuccess;
    alt_u16 control;
    control = reg_file[4];
    if (bBoost)
        control |= 0x0001;
    else        
        control &= 0xFFFE;
    bSuccess = aduio_RegWrite(4, control);  // Left Line In: set left line in volume
    return bSuccess;
}

bool AUDIO_AdcEnableHighPassFilter(bool bEnable){
    bool bSuccess;
    alt_u16 control;
    control = reg_file[5];
    if (bEnable)
        control &= 0xFFFE;
    else        
        control |= 0x0001;
    bSuccess = aduio_RegWrite(5, control);  // Left Line In: set left line in volume
    return bSuccess;    
}



bool AUDIO_DacDeemphasisControl(alt_u8 deemphasis_type){
    bool bSuccess;
    alt_u16 control;
    control = reg_file[5];
    control &= 0xFFF9;
    switch(deemphasis_type){
        case DEEMPHASIS_48K: control |= ((0x03) << 1); break;
        case DEEMPHASIS_44K1: control |= ((0x02) << 1); break;
        case DEEMPHASIS_32K: control |= ((0x01) << 1); break;
    }
    bSuccess = aduio_RegWrite(5, control);  // Left Line In: set left line in volume
    return bSuccess;       
}

bool AUDIO_DacEnableZeroCross(bool bEnable){
    bool bSuccess;
    alt_u16 control_l, control_r;
    alt_u16 mask;
    control_l = reg_file[2];
    control_r = reg_file[3];
    mask = 0x01 << 7;
    if (bEnable){
        control_l |= mask;
        control_r |= mask;
    }else{        
        control_l &= ~mask;
        control_r &= ~mask;
    }        
    bSuccess = aduio_RegWrite(2, control_l);  // Left Line In: set left line in volume
    if (bSuccess)
        bSuccess = aduio_RegWrite(3, control_r);  // Left Line In: set left line in volume
    return bSuccess;      
}

bool AUDIO_DacEnableSoftMute(bool bEnable){
    bool bSuccess;
    alt_u16 control;
    alt_u16 mask;
    control = reg_file[5];
    mask = 0x01 << 3;
    if (bEnable)
        control |= mask;
    else        
        control &= ~mask;
    bSuccess = aduio_RegWrite(5, control);  // Left Line In: set left line in volume
    return bSuccess;      
}

bool AUDIO_MicMute(bool bMute){
    bool bSuccess;
    alt_u16 control;
    alt_u16 mask;
    control = reg_file[4];
    mask = 0x01 << 1;
    if (bMute)
        control |= mask;
    else        
        control &= ~mask;
    bSuccess = aduio_RegWrite(4, control);  // Left Line In: set left line in volume
    return bSuccess;        
}

bool AUDIO_LineInMute(bool bMute){
    bool bSuccess;
    alt_u16 control_l, control_r;
    alt_u16 mask;
    control_l = reg_file[0];
    control_r = reg_file[1];
    mask = 0x01 << 7;
    if (bMute){
        control_l |= mask;
        control_r |= mask;
    }else{        
        control_l &= ~mask;
        control_r &= ~mask;
    }        
    bSuccess = aduio_RegWrite(0, control_l);  // Left Line In: set left line in volume
    if (bSuccess)
        bSuccess = aduio_RegWrite(1, control_r);  // Left Line In: set left line in volume
    return bSuccess;        
}



bool AUDIO_SetInputSource(alt_u8 InputSource){
    bool bSuccess;
    alt_u16 control;
    alt_u16 mask;
    control = reg_file[4];
    mask = 0x01 << 2;
    if (InputSource == SOURCE_MIC)
        control |= mask;
    else        
        control &= ~mask;
    bSuccess = aduio_RegWrite(4, control);  // Left Line In: set left line in volume
    return bSuccess;       
}


// See datasheet page 39
bool AUDIO_SetSampleRate(alt_u8 SampleRate){
    bool bSuccess;
    alt_u16 control;

    control = 0;
    switch(SampleRate){
         // MCLK = 18.323
        case RATE_ADC48K_DAC48K: control = (0x0) << 2; break;
        case RATE_ADC48K_DAC8K: control = (0x1) << 2; break;
        case RATE_ADC8K_DAC48K: control = (0x2) << 2; break;
        case RATE_ADC8K_DAC8K: control = (0x3) << 2; break;
        case RATE_ADC32K_DAC32K: control = (0x6) << 2; break;
        case RATE_ADC96K_DAC96K: control = (0x7) << 2; break;
        //USB mode
        case RATE_ADC48K_DAC48K_USB: control = (0x00 << 2)|USB; break;
        case RATE_ADC44K_DAC44K_USB: control = (0x08 << 2) |BSOR|USB; break;
        case RATE_ADC48K_DAC8K_USB:  control = (0x01 << 2)|USB; break;
        case RATE_ADC44K_DAC8K_USB:  control = (0x09 << 2)|BSOR|USB; break;
        case RATE_ADC8K_DAC48K_USB:  control = (0x02 << 2)|USB; break;
        case RATE_ADC8K_DAC44K_USB:  control = (0x0A << 2)|BSOR|USB; break;
        case RATE_ADC8K_DAC8K_USB:   control = (0x03 << 2)|USB; break;
        case RATE_ADC32K_DAC32K_USB: control = (0x06 << 2)|USB; break;
        case RATE_ADC96K_DAC96K_USB: control = (0x07 << 7)|USB; break;
        case RATE_ADC88K_DAC88K_USB: control = (0x0F << 7)|BSOR|USB; break;
        // MCLK = 11.2896
      //  case RATE_ADC44K1_DAC44K1: control = (0x8) << 2; break;
      //  case RATE_ADC44K1_DAC8K: control = (0x9) << 2; break;
      //  case RATE_ADC8K_DAC44K1: control = (0xA) << 2; break;
    }
        
    bSuccess = aduio_RegWrite(8, control);  // Left Line In: set left line in volume
    return bSuccess;      
}

bool AUDIO_SetBits(alt_u8 bits)
{
    bool bSuccess;
    alt_u16 control;
    control = 0;
    switch(bits){
         // MCLK = 18.323
        case 32: control = 0x4E; break;
        case 24: control = 0x4A; break;
        case 20: control = 0x46; break;
        case 16: control = 0x42; break;
        default: return false;
    }

    bSuccess = aduio_RegWrite(7, control);  // Left Line In: set left line in volume
    return bSuccess;
}




bool AUDIO_SetLineInVol(alt_u16 l_vol, alt_u16 r_vol){
    bool bSuccess;
    alt_u16 control;
    
    // left
    control = reg_file[0];
    control &= 0xFFE0;
    control += l_vol;
    bSuccess = aduio_RegWrite(0, control);
    
    if (bSuccess){
        // right
        control = reg_file[1];
        control &= 0xFFE0;
        control += r_vol;
        bSuccess = aduio_RegWrite(1, control);        
    }
    
    AUDIO_DEBUG(("[AUDIO] set Line-In vol(%d,%d) %s\r\n", l_vol, r_vol, bSuccess?"success":"fail"));        
    return bSuccess;
}

bool AUDIO_SetLineOutVol(alt_u16 l_vol, alt_u16 r_vol){
    bool bSuccess;
    alt_u16 control;
    
    // left
    control = reg_file[2];
    control &= 0xFF80;
    control += l_vol;
    bSuccess = aduio_RegWrite(2, control);
    
    if (bSuccess){
        // right
        control = reg_file[3];
        control &= 0xFF80;
        control += r_vol;
        bSuccess = aduio_RegWrite(3, control);        
    }    
    
    AUDIO_DEBUG(("[AUDIO] set Line-Out vol(%d,%d) %s\r\n", l_vol, r_vol, bSuccess?"success":"fail"));        
    
    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////
///////////// Interla function body ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

bool aduio_RegWrite(alt_u8 reg_index, alt_u16 data16){
alt_u8 buf[2];
alt_u32 result;
    alt_u8 data, control;
    if (reg_index <= 10)
        reg_file[reg_index] = data16;
    data = data16 & 0xFF;
    control = (reg_index << 1) | ((data16 >> 8) & 0x01);
    AUDIO_DEBUG(("[AUDIO] set audio reg[%02d] = %04Xh\r\n", reg_index, data16));
    buf[0] = control;
    buf[1] = data;
    result = I2C_write_ex(I2C_MASTER_BASE,WM8731_ADDR_WRITE,buf,2);
    if (result!=I2C_ACK)
    {
        AUDIO_DEBUG(("[AUDIO] write reg fail!!!!\r\n"));
        return false;
    }
    else
    	return true;
    usleep(50000);

}

void audio_play_u8(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode )
{
alt_u32 i;
alt_u8 *mydata = data;
    for(i=0;i<nsamples;i++)
    {
        right[i] = (*mydata)<<24;
        mydata++;
		if(channels==1)
		{
			left[i] = right[i];
		}
		else
		{
			left[i] = (*mydata)<<24;
			mydata++;
			if(channels>2)
				mydata+=(channels-2);
		}
    }
}

void audio_play_u16(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode )
{
	alt_u32 i;
	alt_u16 *mydata = data;
    for(i=0;i<nsamples;i++)
    {
        right[i] = (*mydata)<<16;
        mydata++;
		if(channels==1)
		{
			left[i] = right[i];
		}
		else
		{
			left[i] = (*mydata)<<16;
			mydata++;
			if(channels>2)
				mydata+=(channels-2);
		}
    }
}

void audio_play_u24(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode )
{
	alt_u32 i;
	alt_u8 *mydata = data;
    for(i=0;i<nsamples;i++)
    {
        right[i] = ((*mydata)<<8)|((*(mydata+1))<<16)|((*(mydata+2))<<24);
        mydata+=3;
		if(channels==1)
		{
			left[i] = right[i];
		}
		else
		{
			left[i] = ((*mydata)<<8)|((*(mydata+1))<<16)|((*(mydata+2))<<24);
			mydata+=3;
			if(channels>2)
				mydata+=(channels-2)*3;
		}
    }
}

void audio_play_u32(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode )
{
	alt_u32 i;
	alt_u32 *mydata = (alt_u32*)data;
    for(i=0;i<nsamples;i++)
    {
        right[i] = (*mydata);
        mydata++;
		if(channels==1)
		{
			left[i] = right[i];
		}
		else
		{
			left[i] = (*mydata);
			mydata++;
			if(channels>2)
				mydata+=(channels-2);
		}
    }
}
