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
*
***************************************************************************************************/
#include <p33FJ256GP506.h>
#include <board\h\sask.h>
#include <board\inc\ex_sask_generic.h>
#include <board\inc\ex_sask_led.h>
#include <dsp\h\dsp.h>
#include <peripherals\adc\h\ADCChannelDrv.h>
#include <peripherals\pwm\h\OCPWMDrv.h>

#include "..\inc\filter.h"
#include "..\inc\modulate.h"
#include "..\inc\complexmultiply.h"
#include "..\inc\transform.h"
#include <peripherals\timers\inc\ex_timer.h>

//#define __DEBUG_OVERRIDE_INPUT
//#define __DEBUG_FILTERS
//#define __DEBUG_SHIFTERS
//#define __DEBUG_TRANSFORMS

#define FRAME_SIZE 	128
#define PRESSED		1
#define UNPRESSED	0
#define NSamples	10



#define MODE_DO_NOTHING		0

/*
#define UPPER_CARRIER_FREQ 		625	
#define LOWER_CARRIER_FREQ 		62.5
#define CARRIER_INC				62.5
#define CARRIER_DEC				62.5

//Modes are used to change the way the device does things, pressing switch 1 changes the mode
#define MODE_DO_NOTHING			0 //the device passes the audio straight through to the output
#define MODE_BAND_PASS_FILTER	1 //the device uses the band pass filter to remove negative audio frequencies
#define MODE_BAND_PASS_SHIFT	3 //the device band pass filters and shifts the audio frequencies
#define MODE_LOW_PASS_FILTER	2 //the device uses the shifted low pass filter to remove negative audio frequencies
#define MODE_LOW_PASS_SHIFT		4 //the device uses shifted low pass filters and shifts the audio frequencies
#define MODE_FREQ_DOMAIN		5 //the device works on the audio signal in the frequency domain
#define MODE_TOTAL				6



//Allocate memory for input and output buffers
fractional		adcBuffer		[ADC_CHANNEL_DMA_BUFSIZE] 	__attribute__((space(dma)));
fractional		ocPWMBuffer		[OCPWM_DMA_BUFSIZE]		__attribute__((space(dma)));

//variables for FFT
fractcomplex compx[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compX[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compXfiltered[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compXshifted[FRAME_SIZE]__attribute__ ((space(ymemory),far));

//variables for audio processing
fractional		frctAudioIn			[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractional		frctAudioWorkSpace	[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractional		frctAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compCarrierSignal	[FRAME_SIZE]__attribute__ ((space(ymemory),far));

*/



fractional		adcBuffer		[ADC_CHANNEL_DMA_BUFSIZE] 	__attribute__((space(dma)));
fractional		ocPWMBuffer		[OCPWM_DMA_BUFSIZE]		__attribute__((space(dma)));

//Instantiate the drivers
ADCChannelHandle adcChannelHandle;
OCPWMHandle 	ocPWMHandle;


//variables for FFT
fractcomplex compx[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compX[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compXfiltered[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractcomplex compXshifted[FRAME_SIZE]__attribute__ ((space(ymemory),far));

//variables for audio processing
fractional		frctAudioIn			[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractional		frctAudioWorkSpace	[FRAME_SIZE]__attribute__ ((space(ymemory),far));
fractional		frctAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compAudioOut		[FRAME_SIZE]__attribute__ ((space(xmemory),far));
fractcomplex	compCarrierSignal	[FRAME_SIZE]__attribute__ ((space(ymemory),far));

//Create the driver handles
ADCChannelHandle *pADCChannelHandle 	= &adcChannelHandle;
OCPWMHandle 	*pOCPWMHandle 		= &ocPWMHandle;


int AudioOut [FRAME_SIZE][NSamples]__attribute__ ((space(ymemory),far));


// Functions devlarations
void PlayRecorded(int sampleRow);
void RecordAudio(int sampleRow);
void ApplyTransformation();
void CountPitch();

int PresentRow;
unsigned long cycles_run;
int countHighPitch, countMedPitch, countLowPitch;

int main(void)
{
	int iMode = MODE_DO_NOTHING;
	int iSwitch1Pressed = UNPRESSED;
	int iSwitch2Pressed = UNPRESSED;
	int iShiftAmount = 1;
	
	// time per cycle
	int clock_frequency, cycle_time, delay_time;
	unsigned long delay_cycles;
	clock_frequency = 40e6;
	cycle_time = 1 / clock_frequency;
	delay_time = 0.2;
	delay_cycles = delay_time / cycle_time;

	//float fCarrierFrequency = 1;
	//createComplexSignal(fCarrierFrequency,FRAME_SIZE,compCarrierSignal);
	
	initFilter();
	ex_sask_init( );
	//SASKInit();

	//Initialise Audio input and output function
	//ADCChannelInit	(pADCChannelHandle,adcBuffer);			
	//OCPWMInit		(pOCPWMHandle,ocPWMBuffer);			

	//Start Audio input and output function
	//ADCChannelStart	(pADCChannelHandle);
	//OCPWMStart		(pOCPWMHandle);	
	
		
		
	int i = 0;
	int n = 0;


	// LEDS REPRESENT STATES
	//RED LED MEANS ERROR
	//YELLOW LED MEANS RECORDING
	//GREEN LED MEANS PLAYING

	//BUTTON 1 -> RECORD
	//BUTTON 2 -> PLAY


	while(1)
	{		
		

		// insert delay
		

		//while(ADCChannelIsBusy(pADCChannelHandle));
			//Read in the Audio Samples from the ADC
		//DCChannelRead	(pADCChannelHandle,frctAudioIn,FRAME_SIZE);
		//VectorCopy(FRAME_SIZE,frctAudioOut,frctAudioIn);
	



	//	#ifndef __DEBUG_OVERRIDE_INPUT//if not in debug mode, read audio in from the ADC
			//Wait till the ADC has a new frame available
	//	#endif

	//	filter the negative part of the Fourier trasform
	//	fourierTransform(FRAME_SIZE,compX,frctAudioIn);
	//	filterNegativeFreq(FRAME_SIZE,compXfiltered,compX);


	//	Initialize the values for the Pitch detection
		//MaxFreqaValue = 0;
		//MaxValue = 0;		
		//i=0;
		//module = 0;
		cycles_run = 0;

	// DIFFERENTS ACTIONS ARE MADE DEPENDING WHICH SWITCH IS PRESSED
		// Record
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
	    	//CountPitch();
	    	// step 3: apply transformation

		}



// Button 2 action: play sound
		else if(CheckSwitchS2() == 1){
			YELLOW_LED = 1;
	    	RED_LED = 1;
	    	GREEN_LED = 0;

	    	for(n = 0; n < NSamples; n++){
	    		PlayRecorded(n);
	    		while(cycles_run < 20){		
	    			//__delay32( delay_cycles);
					//while(OCPWMIsBusy(pOCPWMHandle));	
					OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);
					cycles_run++;	    			
	    		}

	    	}
			OCPWMStop	(pOCPWMHandle);
	    	
	    	YELLOW_LED = 1;
	    	RED_LED = 1;
	    	GREEN_LED = 1;
		}

/*
	//	Extract the highest value in the frequency domain and the index of the belonging step in the interval
		for(i = 1 ; i<64;i++){
			module = pow(compXfiltered[i].real,2) + pow( compXfiltered[i].imag,2);
			if(module > MaxValue){
				MaxFreqaValue = i;
				MaxValue = module;
			}
		}


		//	the frequency spectrum goes from 0 to 4kHz and its represented in a 64 step interval
			LowInterval = 20;
			HighInterval = 40;

		// Find the belonging interval of the Pitch
		if(LowInterval > MaxFreqaValue){
			YELLOW_LED = 1;
	        RED_LED = 1;
	        GREEN_LED = 0;
		}
		else if  (LowInterval <= MaxFreqaValue && HighInterval >= MaxFreqaValue){
			YELLOW_LED = 0;
	        RED_LED = 1;
	        GREEN_LED = 1;
		}
		else if(HighInterval < MaxFreqaValue && 64 >= MaxFreqaValue){
			YELLOW_LED = 1;
	        RED_LED = 0;
	        GREEN_LED = 1;
		}
/*		else{
			YELLOW_LED = 1;
	        RED_LED = 1;
	        GREEN_LED = 1;
		}*/

/*
		while(OCPWMIsBusy(pOCPWMHandle));	
		//Write the real part of the frequency shifted complex audio signal to the output
		OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);
		*/
		
	

		//Wait till the OC is available for a new frame
		//while(OCPWMIsBusy(pOCPWMHandle));	
		//Write the real part of the frequency shifted complex audio signal to the output
		//OCPWMWrite (pOCPWMHandle,frctAudioOut,FRAME_SIZE);

		
	}
}





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
			AudioOut[a][sampleRow] = frctAudioIn[a];
		}
	}else {
		/*Error message*/
			YELLOW_LED = 1;
	        RED_LED = 0;
	        GREEN_LED = 1;
	}
		
}



void PlayRecorded(int sampleRow){


		int a = 0;
	if (sampleRow < NSamples){
		OCPWMInit		(pOCPWMHandle,ocPWMBuffer);			
		OCPWMStart		(pOCPWMHandle);	
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



void CountPitch(){

	countHighPitch = 0;
	countMedPitch = 0;
	countLowPitch = 0;
	int module;
	int MaxFreqaValue ;
	int MaxValue;
	int LowInterval;
	int HighInterval;
	int a = 0;
	int na,in = 0;

	LowInterval = 20;
	HighInterval = 40;
	
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


void ApplyTransformation(){
	//part 1: find the case
	// a) high frequencies 
	if (countHighPitch >=6){

	}
	// b) high frequencies
	else if (countLowPitch >=6){
		
	}
	// c) equally distributed
	else{

	}

}
