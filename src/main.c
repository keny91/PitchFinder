/**************************************************************************************************
* @file		Projects\PitchFinder\src\main.c
*
* Summary:
*         	main function for the PitchFinder
*
* ToDo:
*     		Find solution to the Flickering Issue
*
* Originator:
*     		Luis Herranz
*
* Version:
* 			Version 1.00	27/03/2016	Luis Herranz	Initial Version, clean
			Version 1.1		16/04/2016	Luis Herranz	Auralization Version, on progress
			Version 2.0		28/04/2016	Luis Herranz	Auralization Version, completed, function up
*
***************************************************************************************************/
#include <p33FJ256GP506.h>
#include <board\h\sask.h>
#include <board\inc\ex_sask_generic.h>
#include <board\inc\ex_sask_led.h>
#include <dsp\h\dsp.h>
#include <peripherals\adc\h\ADCChannelDrv.h>
#include <peripherals\pwm\h\OCPWMDrv.h>
#include <peripherals\timers\inc\ex_timer.h>

#include "..\inc\filter.h"
#include "..\inc\modulate.h"
#include "..\inc\complexmultiply.h"
#include "..\inc\transform.h"



#define FRAME_SIZE 	64
#define PRESSED		1
#define UNPRESSED	0
#define NSamples	20
#define LowInterval	20
#define HighInterval	40




fractional		adcBuffer		[ADC_CHANNEL_DMA_BUFSIZE] 	__attribute__((space(dma)));
fractional		ocPWMBuffer		[OCPWM_DMA_BUFSIZE]		__attribute__((space(dma)));

//Instantiate the drivers
ADCChannelHandle adcChannelHandle;
OCPWMHandle 	ocPWMHandle;


//variables for FFT
fractcomplex compx[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compX[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compXfiltered[FRAME_SIZE]__attribute__ ((space(ymemory),far));
int compXshifted[FRAME_SIZE]__attribute__ ((space(ymemory),far));

//variables for audio processing
fractional		frctAudioIn			[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractional		frctAudioWorkSpace	[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractional		frctAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compCarrierSignal	[FRAME_SIZE]__attribute__ ((space(xmemory),far));

//Create the driver handles
ADCChannelHandle *pADCChannelHandle 	= &adcChannelHandle;
OCPWMHandle 	*pOCPWMHandle 		= &ocPWMHandle;

// ROWS -> COLS
int AudioOut [FRAME_SIZE][NSamples]__attribute__ ((space(ymemory),far));
int filter [FRAME_SIZE]__attribute__ ((space(ymemory),far));

// Functions devlarations
void PlayRecorded(int sampleRow);
void RecordAudio(int sampleRow);
void CreateFilter();
//void ApplyFilter();
void CountPitch();

int PresentRow;
unsigned long cycles_run;
int countHighPitch, countMedPitch, countLowPitch;

int main(void)
{

	// time per cycleÇ
	float clock_frequency, cycle_time, delay_time;
	float delay_time2;
	unsigned long delay_cycles, delay_cycles2;
	clock_frequency = 40e6;
	cycle_time = 1 / clock_frequency;
	delay_time = 0.1;
	delay_cycles = delay_time / cycle_time;
	delay_time2 = 0.02;
	delay_cycles2 = delay_time2 / cycle_time;

	//float fCarrierFrequency = 1;
	//createComplexSignal(fCarrierFrequency,FRAME_SIZE,compCarrierSignal);
	
	initFilter();
	ex_sask_init( );
	//SASKInit();

		
	//int i = 0;
	int n = 0;


	// LEDS REPRESENT STATES
	//RED LED MEANS ERROR
	//YELLOW LED MEANS RECORDING
	//GREEN LED MEANS PLAYING

	//BUTTON 1 -> RECORD
	//BUTTON 2 -> PLAY


	// Do forever
	while(1)
	{		
		

	// DIFFERENTS ACTIONS ARE MADE DEPENDING WHICH SWITCH IS PRESSED
		// Record
		//(SWITCH_S2 == 0 && SWITCH_S1 ==1)
		if (CheckSwitchS1() == 1){

			// Step 1: record the sound
			YELLOW_LED = 0;
	    	RED_LED = 1;
	    	GREEN_LED = 1;
	    	int n = 0;
	    	for(n = 0; n < NSamples; n++){
	    		RecordAudio(n);
	    		__delay32( delay_cycles);
	    	}
	    	
	    	YELLOW_LED = 1;
	    	RED_LED = 1;
	    	GREEN_LED = 1;

	    	// step 2: determine the interval
	    	CountPitch();
	    	// step 3: create a filter and apply transformation in Frequency Domain
	    	CreateFilter();
	    	//ApplyFilter();
		}



// Button 2 action: play sound
		if(CheckSwitchS2() == 1){
			YELLOW_LED = 1;
	    	RED_LED = 1;
	    	GREEN_LED = 0;

	    	OCPWMInit		(pOCPWMHandle,ocPWMBuffer);			
			OCPWMStart		(pOCPWMHandle);	

	    	for(n = 0; n < NSamples; n++){
	    	/*	PlayRecorded(n);
	 			//while(cycles_run < 20){		
	    			__delay32( delay_cycles);
					while(OCPWMIsBusy(pOCPWMHandle));	
					OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);
					cycles_run++;
					*/	  
				cycles_run=0;
	    		PlayRecorded(n);
	 			while(cycles_run < 20){		
	    		//	__delay32( delay_cycles2);
					while(OCPWMIsBusy(pOCPWMHandle));	
					OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);
					cycles_run++;	    			
	    		}
	    	}

			OCPWMStop	(pOCPWMHandle);
	    	
	    	YELLOW_LED = 1;
	    	RED_LED = 1;
	    	GREEN_LED = 1;
		}


		
	}
}




/***********************************
RecordAudio: passes the present captured frame in the buffer into a concrete position in the
storage array.

Red light will activate if we receive unexpected parameters.
************************************/
void RecordAudio(int sampleRow){

	int a = 0;
	if (sampleRow < NSamples){
		ADCChannelInit	(pADCChannelHandle,adcBuffer);			
		ADCChannelStart	(pADCChannelHandle);	

		while(ADCChannelIsBusy(pADCChannelHandle));
			//Read in the Audio Samples from the ADC
		ADCChannelRead(pADCChannelHandle,frctAudioIn,FRAME_SIZE);
		for(a = 0 ; a <FRAME_SIZE; a++){

		//VectorCopy(FRAME_SIZE,frctAudioOut,frctAudioIn);
			AudioOut[a][sampleRow] = frctAudioIn[a]; // MEMCOPY
		}
	}else {
		/*Error message*/
			YELLOW_LED = 1;
	        RED_LED = 0;
	        GREEN_LED = 1;
	}
		
}



/***********************************
PlayRecorded: plays through the audio source output, a single frame in the stored array.
We will be playing that frame for various executions since we don´t want a tone but a constant 
time sound

Red light will activate if we receive unexpected parameters.
************************************/

void PlayRecorded(int sampleRow){


		int a = 0;
	if (sampleRow < NSamples){

		for(a = 0 ; a <FRAME_SIZE; a++){
			frctAudioOut[a]= AudioOut[a][sampleRow];
		}
		//while(OCPWMIsBusy(pOCPWMHandle));	
		//OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);
		//OCPWMStop	(pOCPWMHandle);
	}else {
		/*Error message*/
			YELLOW_LED = 1;
	        RED_LED = 0;
	        GREEN_LED = 1;
	}
}





/***********************************
CountPitch(): After we have a full frame storage. We will extract where 
can their Pitches be found which will determine the kind of signal processing 
that will go through.
************************************/

void CountPitch(){

	countHighPitch = 0;
	countMedPitch = 0;
	countLowPitch = 0;
	int module;
	int MaxFreqaValue ;
	int MaxValue;

	int a = 0;
	int na,in = 0;


	
	for(na = 0; na < NSamples; na++){
	    

	    for(a = 0 ; a <FRAME_SIZE; a++){
			frctAudioIn[a]= AudioOut[a][na];
		}
//  APPLY FFT

		fourierTransform(FRAME_SIZE,compX,frctAudioIn);
		filterNegativeFreq(FRAME_SIZE,compXfiltered,compX);

		// Find highest value
		MaxValue = -1;
		MaxFreqaValue = -1; 
	    for(in = 1 ; in<FRAME_SIZE;in++){
			module = pow(compXfiltered[in].real,2) + pow( compXfiltered[in].imag,2);
			if(module > MaxValue){
				MaxFreqaValue = in;
				MaxValue = module;
			}
		}

				//	the frequency spectrum goes from 0 to 4kHz and its represented in a 64 step interval


		// Find the belonging interval of the Pitch
		if(LowInterval > MaxFreqaValue){
			countLowPitch ++;
		}
		else if  (LowInterval <= MaxFreqaValue && HighInterval >= MaxFreqaValue){
			countMedPitch ++;
		}
		else if(HighInterval < MaxFreqaValue && FRAME_SIZE >= MaxFreqaValue){
			countHighPitch++;
		}

	}

}


/***********************************
ApplyTransformation(): Apply transformation will modify each of the stored frames applying 
a frequency filter over all of them.
The kind of filter used depends on the picth distributions. The process requires that each frame is
converted with the FFT and the iFFT afterwards.

************************************/


void CreateFilter(){
	//part 1: find the case
	// a) high frequencies 
	int a = 0;
	int n = 0;
	for (n = 0;n<NSamples;n++){
		for(a=0;a<FRAME_SIZE;a++){
			frctAudioIn[a]=AudioOut[a][n];
		}
		VectorCopy(FRAME_SIZE,frctAudioOut,frctAudioIn);

	if (countLowPitch >=6){
		shiftedLowPassFilter(FRAME_SIZE,frctAudioOut,frctAudioIn);
		/*
		for(a = 0; a<FRAME_SIZE; a++){
			if(a<LowInterval){
				filter[a] = 1;
			}
			else{
				filter[a] = 0;
			}
		}
		*/
			
	}
	// b) high frequencies
	else if (countHighPitch >=6){
		bandPassFilter(FRAME_SIZE,frctAudioOut,frctAudioIn);
		/*
		for(a = 0; a<FRAME_SIZE; a++){
			if(a>=HighInterval){
				filter[a] = 1;
			}
			else{
				filter[a] = 0;
			}
			
		}
		*/
	}
	// c) equally distributed
	else{
		shiftedLowPassFilter(FRAME_SIZE,frctAudioOut,frctAudioIn);
		/*
		for(a = 0; a<FRAME_SIZE; a++){
			
			
			if(a>=HighInterval){
				filter[a] = 2;
			}
			else if(a<LowInterval){
				filter[a] = 0;
			}
			else{
				filter[a] = 1;
				
			}
		}
		*/
	}
			for(a=0;a<FRAME_SIZE;a++){
			AudioOut[a][n]=frctAudioOut[a];
		}

}
}

/*
void ApplyFilter(){
int n = 0;
int a = 0;
for (n = 0;n<NSamples;n++){

	for(a=0;a<FRAME_SIZE;a++){
		frctAudioIn[a]=AudioOut[a][n];
	}

	fourierTransform(FRAME_SIZE,compX,frctAudioIn);
	filterNegativeFreq(FRAME_SIZE,frctAudioIn,compX);
	//shiftFreqSpectrum(FRAME_SIZE,iShiftAmount,compXshifted,compXfiltered);
	
	for(a=0;a<FRAME_SIZE;a++){
		if(filter[a] == 0)
		//compXshifted[a] = compXfiltered[a]*filter[a];
			compXshifted[a] = filter[a];
		else
			compXshifted[a] = frctAudioIn[a];
		}
	inverseFourierTransform(FRAME_SIZE,frctAudioOut,compXshifted);

	for(a=0;a<FRAME_SIZE;a++){
		AudioOut[a][n]=frctAudioOut[a];
	}
	}
}
*/