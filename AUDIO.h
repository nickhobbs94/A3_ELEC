#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdbool.h>
#include "alt_types.h"

#define VOL_OUT_MAX  0x7F
#define VOL_OUT_MIN  0x00

#define USB  0x01
#define BSOR 0x02

//#define AUDIO_DACFIFO_ISFULL()     IORD_ALTERA_AVALON_PIO_DATA(AUDIO_BASE)
//#define AUDIO_DACFIFO_WRITE(data)  IOWR_ALTERA_AVALON_PIO_DATA(AUDIO_BASE, data)

typedef enum{
    SOURCE_MIC = 0,
    SOURCE_LINEIN
}INPUT_SOURCE;

typedef enum{
        // MCLK = 18.324
    RATE_ADC48K_DAC48K,
    RATE_ADC48K_DAC8K,
    RATE_ADC8K_DAC48K,
    RATE_ADC8K_DAC8K,
    RATE_ADC32K_DAC32K,
    RATE_ADC96K_DAC96K,

    RATE_ADC48K_DAC48K_USB,
    RATE_ADC44K_DAC44K_USB,
    RATE_ADC48K_DAC8K_USB,
    RATE_ADC44K_DAC8K_USB,
    RATE_ADC8K_DAC48K_USB,
    RATE_ADC8K_DAC44K_USB,
    RATE_ADC8K_DAC8K_USB,
    RATE_ADC32K_DAC32K_USB,
    RATE_ADC96K_DAC96K_USB,
    RATE_ADC88K_DAC88K_USB,
} AUDIO_SAMPLE_RATE;

typedef enum{
    DEEMPHASIS_NONE,
    DEEMPHASIS_48K,
    DEEMPHASIS_44K1,
    DEEMPHASIS_32K
}DEEMPHASIS_TYPE;

bool AUDIO_Init(void);
bool AUDIO_MicBoost(bool bBoost);
bool AUDIO_SetLineInVol(alt_u16 l_vol, alt_u16 r_vol);
bool AUDIO_SetLineOutVol(alt_u16 l_vol, alt_u16 r_vol);
bool AUDIO_AdcEnableHighPassFilter(bool bEnable);
bool AUDIO_LineInMute(bool bMute);
bool AUDIO_MicMute(bool bMute);
bool AUDIO_SetInputSource(alt_u8 InputSource);
bool AUDIO_SetSampleRate(alt_u8 SampleRate);
bool AUDIO_SetBits(alt_u8 bits);
// dac control
bool AUDIO_DacDeemphasisControl(alt_u8 deemphasis_type);
bool AUDIO_DacEnableSoftMute(bool bEnable);
bool AUDIO_DacEnableZeroCross(bool bEnable);


bool AUDIO_Reset(void);
bool AUDIO_InterfaceActive(bool bActive);
bool AUDIO_Play1K(void);

typedef void audio_pcmfunc_t(alt_u8 *, alt_u32 ,alt_u8,alt_u32 *, alt_u32 *,alt_u8 );
void audio_play_u8(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode );
void audio_play_u16(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode );
void audio_play_u24(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode );
void audio_play_u32(alt_u8 *data, alt_u32 nsamples, alt_u8 channels, alt_u32 *left, alt_u32 *right,alt_u8 mode );

extern alt_u16 vol_out_l;
extern alt_u16 vol_out_r;
extern alt_u16 vol_in_l;
extern alt_u16 vol_in_r;

#endif /*AUDIO_H_*/
