  /*****f*        user\apps\audio\Audio_Pipe\src\ex_audio_process.c
  * Summary:
  *         Function to do something to audio
  *
  * Inputs:
  *       AudioIn					int - input audio samples
  *       AudioWorkSpace            int - working buffer (history)
  * Outputs:
  *		  AudioOut                  int - output audio samples

  * Dependencies:
  *      none
  * Notes:
  *		none

  * ToDo:
  *     none

  * Originator:
  *     Chris Bore, BORES Signal processing, chris@bores.com, www.bores.com

  * History:
  *      Version 1.00     13/03/2013
  *****/

void ex_audio_process( int frame_size, int *AudioIn, int *AudioWorkSpace, int *AudioOut )
{
	int i;

	for ( i = 0; i < frame_size; i++ )
	{
		AudioOut[ i ] = AudioWorkSpace[ i ] = ( AudioWorkSpace[ i ] * 3 + AudioIn[ i ] ) >> 2;
	}
}
