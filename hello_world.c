#include <stdio.h>
#include <stdlib.h>
#include "altera_up_avalon_audio_dgz.h"
#include "AUDIO.h"
#include "altera_up_avalon_audio_regs_dgz.h"

#include <math.h>

#define VOL 28
#define SAMP 8000
#define SIZ 2048
//#define M_PI 3.141593

alt_32 wav_play_old(alt_32 argc, alt_8* argv[]){
    /* MUST HAVE THE FOLDER audio in the project
     *  GO TO BLACKBOARD AND DOWNLOAD THE FOLDER */

    /* ----- Configure the audio codec: typically we do this
     * only once at the beginning --------------------------     */
    if ( !AUDIO_Init() ) {
    	printf("Unable to initialise audio codec\n");
    	return -1;
    }

    /* ----- Open the audio device: Typically we do this
     * only once at the beginning --------------------------     */
    alt_up_audio_dev*  audio_dev;
    audio_dev = alt_up_audio_open_dev(AUDIO_NAME);



    /* -----  Reset the FIFO in audio codec: typically we do this
     * everytime a new audio stream is opened --------------     */
    alt_up_audio_reset_audio_core(audio_dev);


    /* -----  Set the sampling frequency in the codec: typically we
     * do this everytime a new audio stream is opened -------    */
    switch(SAMP) {
      case 8000:  AUDIO_SetSampleRate(RATE_ADC8K_DAC8K_USB); break;
      case 32000: AUDIO_SetSampleRate(RATE_ADC32K_DAC32K_USB); break;
      case 44100: AUDIO_SetSampleRate(RATE_ADC44K_DAC44K_USB); break;
      case 48000: AUDIO_SetSampleRate(RATE_ADC48K_DAC48K_USB); break;
      case 96000: AUDIO_SetSampleRate(RATE_ADC96K_DAC96K_USB); break;
      default:  printf("Non-standard sampling rate\n"); return -1;
   }



    /* ----- Simple sample code for playing audio -------------- */
    unsigned int buf[SIZ];
    //alt_u32 buf[SIZ];
    alt_32 i, freq;
    for(freq=200; freq<=200; freq+=100) {
    	float omega = ((2.0*M_PI)*freq)/SAMP;

    	// fill buffer: Each sample MUST be a signed 32 bit int
    	for(i=0;i<SIZ;++i){
    		//buf[i] = pow(2,VOL)*sin(omega*i);
    	}

        // Check if we have enough space in the audio codec FIFO
        // if not then wait
        while(alt_up_audio_write_fifo_space(audio_dev,0) < SIZ);

        // Write data into left and right channels  of audio codec FIFO
        alt_up_audio_write_fifo(audio_dev,buf,SIZ,ALT_UP_AUDIO_RIGHT);
	    alt_up_audio_write_fifo(audio_dev,buf,SIZ,ALT_UP_AUDIO_LEFT);
    }

    return 0;
}

#undef VOL
#undef SIZ
#undef SAMP

