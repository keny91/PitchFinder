 /*****h*        use\apps\ADC\Audio_Pipe\inc\ex_audio_process.h
  * Summary:
  *         Library of functions to process audio buffers
  *
  * Variables:
  *     none
  * Functions:

  * Notes:
  *		none

  * ToDo:
  *     none

  * Originator:
  *     Chris Bore, BORES Signal processing, chris@bores.com, www.bores.com

  * History:
  *      Version 1.00     13/03/2013
  *****/

#ifndef __EX_AUDIO_PROCESS_H__
#define __EX_AUDIO_PROCESS_H__

void ex_audio_process( int frame_size, int *ptrAudioIn, int *ptrAudioWorkSpace, int *ptrAudioOut );

#endif
