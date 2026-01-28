/*
 * Copyright (c) 2012-2026 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//
//2013april02, creation for recording a stereo wav file using portaudio.
//			   initially derived from paex_record_file and modified so
//			   the wav file is recording in wav file format instead of
//			   the raw format that was used in paex_record_file.c
//			   also, support for asio and device selection from arguments.
//
//2014may04, added a pause/unpause recording feature attached to key 'P'
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "pa_asio.h"
#include "pa_ringbuffer.h"
#include "pa_util.h"

#include <sndfile.hh>
#include <assert.h>
#include <map>
#include <string>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

#include <conio.h> //for _kbhit()

//2020oct03, spi, begin
#include "defs.h"
#include "spiaudiodevice.h"
#include "spirecordtodisk_ringbufferpausewin32.h"
SPIAudioDevice mySPIAudioDevice;
string global_modestring = ""; //if not empty, this string can be "PAUSEONSTART"
//string global_monitor = ""; //if not empty, will use spitext.exe to display recorded time on given monitor string, "1" for monitor 1, "1.1" for first 1/4 of monitor 1, "1:1" for first 1/16 of monitor 1, etc.
//2020oct03, spi, end

//2020oct05, spi, begin
//relocated in .h
/*
//#define SAMPLE_RATE  (17932) // Test failure to open with this value. 
#define FILE_NAME       "audio_data.raw"
#define NUM_SECONDS     (60)
#define NUM_WRITES_PER_BUFFER   (4)
//#define DITHER_FLAG     (paDitherOff) 
#define DITHER_FLAG     (0) 
*/
//2020oct05, spi, end

//2020oct05, spi, begin
//migrated to top of this file
//migrated data out of the main scope so Terminate() can see it
paTestData          global_paTestData = {0};
PaStream*           stream;
PaError             err = paNoError;
//2020oct05, spi, end


//The event signaled when the app should be terminated.
HANDLE g_hTerminateEvent = NULL;
//Handles events that would normally terminate a console application. 

//2020oct05, spi, begin
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
//int Terminate();
//2020oct05, spi, end

string global_filename;

PaError global_err;

//2020oct05, spi, begin
bool global_prev_pauserecording = true; // = false;
//2020oct05, spi, end
bool global_pauserecording=false;


//2020oct05, spi, begin
//typedef moved in .h
/*
typedef struct
{
    unsigned            frameIndex;
    int                 threadSyncFlag;
    SAMPLE             *ringBufferData;
    PaUtilRingBuffer    ringBuffer;
    FILE               *file;
    void               *threadHandle;
}
 
paTestData;
*/
//2020oct05, spi, end


bool AppendWavFile(const char* filename, const void* pVoid, long sizeelementinbytes, long count)
{
	assert(filename);
	assert(sizeelementinbytes==4);
    const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;  
	//const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_24;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_32;  
	//SndfileHandle outfile(filename, SFM_WRITE, format, numChannels, SampleRate); 

	SndfileHandle outfile(filename, SFM_RDWR, format, NUM_CHANNELS, SAMPLE_RATE); 
	//SndfileHandle outfile(filename, SFM_RDWR, format, 1, SAMPLE_RATE); 
	outfile.seek(outfile.frames(), SEEK_SET);
	/*
	if(frameIndex==0)
	{
		outfile.write(pSamples, numSamples);  
	}
	else
	{
		assert(frameIndex<=totalFrames);
		outfile.write(pSamples, frameIndex*NUM_CHANNELS); 
	}
	*/
	outfile.write((const float*)pVoid, count); 
	return true;
}

// This routine is run in a separate thread to write data from the ring buffer into a wav file (during Recording)
//static int threadFunctionWriteToWavFile(void* ptr)
int threadFunctionWriteToWavFile(void* ptr)
{
    paTestData* pData = (paTestData*)ptr;
 
    // Mark thread started  
    pData->threadSyncFlag = 0;
 
    while (1)
    {
        ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferReadAvailable(&pData->ringBuffer);
        if ( (elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER) ||
             pData->threadSyncFlag )
        {
            void* ptr[2] = {0};
            ring_buffer_size_t sizes[2] = {0};
 
            /* By using PaUtil_GetRingBufferReadRegions, we can read directly from the ring buffer */
            ring_buffer_size_t elementsRead = PaUtil_GetRingBufferReadRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
            if (elementsRead > 0)
            {
                int i;
                for (i = 0; i < 2 && ptr[i] != NULL; ++i)
                {
                    //fwrite(ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i], pData->file);
					AppendWavFile(global_filename.c_str(), ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i]);
                }
                PaUtil_AdvanceRingBufferReadIndex(&pData->ringBuffer, elementsRead);
            }
 
            if (pData->threadSyncFlag)
            {
                break;
            }
        }
 
        /* Sleep a little while... */
        Pa_Sleep(20);
 
    }
 
    pData->threadSyncFlag = 0;
    return 0;
}

/* This routine is run in a separate thread to write data from the ring buffer into a file (during Recording) */ 
//static int threadFunctionWriteToRawFile(void* ptr)
int threadFunctionWriteToRawFile(void* ptr)
{
    paTestData* pData = (paTestData*)ptr;
 
    /* Mark thread started */ 
    pData->threadSyncFlag = 0;
 
    while (1)
    {
        ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferReadAvailable(&pData->ringBuffer);
        if ( (elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER) ||
             pData->threadSyncFlag )
        {
            void* ptr[2] = {0};
            ring_buffer_size_t sizes[2] = {0};
 
            /* By using PaUtil_GetRingBufferReadRegions, we can read directly from the ring buffer */
            ring_buffer_size_t elementsRead = PaUtil_GetRingBufferReadRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
            if (elementsRead > 0)
            {
                int i;
                for (i = 0; i < 2 && ptr[i] != NULL; ++i)
                {
                    fwrite(ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i], pData->file);
                }
                PaUtil_AdvanceRingBufferReadIndex(&pData->ringBuffer, elementsRead);
            }
 
            if (pData->threadSyncFlag)
            {
                break;
            }
        }
 
        /* Sleep a little while... */
        Pa_Sleep(20);
 
    }
 
    pData->threadSyncFlag = 0;
    return 0;
}
 

 
/* This routine is run in a separate thread to read data from file into the ring buffer (during Playback). When the file
   has reached EOF, a flag is set so that the play PA callback can return paComplete */ 
//static int threadFunctionReadFromRawFile(void* ptr)
int threadFunctionReadFromRawFile(void* ptr)
{
    paTestData* pData = (paTestData*)ptr;

    while (1)
    {
        ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferWriteAvailable(&pData->ringBuffer);
 
        if (elementsInBuffer >= pData->ringBuffer.bufferSize / NUM_WRITES_PER_BUFFER)
        {
            void* ptr[2] = {0};
            ring_buffer_size_t sizes[2] = {0};
 
            /* By using PaUtil_GetRingBufferWriteRegions, we can write directly into the ring buffer */
            PaUtil_GetRingBufferWriteRegions(&pData->ringBuffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
 
            if (!feof(pData->file))
            {
                ring_buffer_size_t itemsReadFromFile = 0;
                int i;
                for (i = 0; i < 2 && ptr[i] != NULL; ++i)
                {
                    itemsReadFromFile += (ring_buffer_size_t)fread(ptr[i], pData->ringBuffer.elementSizeBytes, sizes[i], pData->file);
                }
                PaUtil_AdvanceRingBufferWriteIndex(&pData->ringBuffer, itemsReadFromFile);
 
                /* Mark thread started here, that way we "prime" the ring buffer before playback */
                pData->threadSyncFlag = 0;
            }
            else
            {
                /* No more data to read */
                pData->threadSyncFlag = 1;
                break;
            }
        }
 
        /* Sleep a little while... */
        Pa_Sleep(20);
    }
    return 0;
}
 

//2020oct05, spi, begin
//relocated in .h
/*
typedef int (*ThreadFunctionType)(void*);
*/
//2020oct05, spi, end

/* Start up a new thread in the given function, at the moment only Windows, but should be very easy to extend
 
   to posix type OSs (Linux/Mac) */
 
//static PaError startThread( paTestData* pData, ThreadFunctionType fn ) 
PaError startThread( paTestData* pData, ThreadFunctionType fn ) 
{
#ifdef _WIN32
    typedef unsigned (__stdcall* WinThreadFunctionType)(void*);
    pData->threadHandle = (void*)_beginthreadex(NULL, 0, (WinThreadFunctionType)fn, pData, CREATE_SUSPENDED, NULL);
    if (pData->threadHandle == NULL) return paUnanticipatedHostError;
 
    /* Set file thread to a little higher prio than normal */
    SetThreadPriority(pData->threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
 
    /* Start it up */
    pData->threadSyncFlag = 1;
    ResumeThread(pData->threadHandle);
#endif
 
    /* Wait for thread to startup */
    while (pData->threadSyncFlag) {
        Pa_Sleep(10);
    }
 
    return paNoError;
}
 

 
//static int stopThread( paTestData* pData )
int stopThread( paTestData* pData )
{
    pData->threadSyncFlag = 1;
    /* Wait for thread to stop */
    while (pData->threadSyncFlag) {
        Pa_Sleep(10);
    }
 
#ifdef _WIN32
    CloseHandle(pData->threadHandle);
    pData->threadHandle = 0;
#endif

    return paNoError;
}
 

 

 
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
//static int recordCallback( const void *inputBuffer, void *outputBuffer,
int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    
	//2020oct03, spi, begin
	//now, need to record AND hear audio out even when recording is paused
	//1) always playback
	const SAMPLE* in = (const SAMPLE*)inputBuffer;
	SAMPLE* out = (SAMPLE*)outputBuffer;
    //2020dec19, spi, begin
    if (out != NULL)
    {
    //2020dec19, spi, end
        if (global_prev_pauserecording)
        {
            global_prev_pauserecording = false;
            memset(out, 0, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
        }
        else
        {
            memcpy(out, in, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
        }
    //2020dec19, spi, begin
    }
    //2020dec19, spi, end
    
    //2) as before, record unless required to pause recording
	//2020oct03, spi, end

	if(global_pauserecording) return paContinue;

    paTestData *data = (paTestData*)userData;
    ring_buffer_size_t elementsWriteable = PaUtil_GetRingBufferWriteAvailable(&data->ringBuffer);
    ring_buffer_size_t elementsToWrite = min(elementsWriteable, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
 
    (void) outputBuffer; //Prevent unused variable warnings.
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
 
    data->frameIndex += PaUtil_WriteRingBuffer(&data->ringBuffer, rptr, elementsToWrite);
 
    return paContinue;
}
 

 
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
//static int playCallback( const void *inputBuffer, void *outputBuffer, 
int playCallback( const void *inputBuffer, void *outputBuffer, 
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paTestData *data = (paTestData*)userData;
    ring_buffer_size_t elementsToPlay = PaUtil_GetRingBufferReadAvailable(&data->ringBuffer);
    ring_buffer_size_t elementsToRead = min(elementsToPlay, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
    SAMPLE* wptr = (SAMPLE*)outputBuffer;
 
    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
 
    data->frameIndex += PaUtil_ReadRingBuffer(&data->ringBuffer, wptr, elementsToRead);
    return data->threadSyncFlag ? paComplete : paContinue;
}
 

 
//static unsigned NextPowerOf2(unsigned val)
unsigned int NextPowerOf2(unsigned int val)
{
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    return ++val;
}
 
//2020oct05, spi, begin
//migrated to top of this file
/*
//migrated data out of the main scope so Terminate() can see it
paTestData          data = {0};
PaStream*           stream;
PaError             err = paNoError;
*/
//2020oct05, spi, end
 
//2020oct05, spi, begin
/*
//WARNING, main(int argc, char *argv[]) is NOT the entry point anymore
//WARNING, it was the console app entry point.
//WARNING, the new (windows app) entry point is:
//WARNING, _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
//WARNING, this new entry point is defined in file spitext.cpp
*/
//2020oct05, spi, end

////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]);
int main(int argc, char *argv[])
{
	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);

	///////////////////
	//read in arguments
	///////////////////
	global_filename = "testrecording.wav"; //usage: spirecord testrecording.wav 10 "E-MU ASIO" 0 1
	float fSecondsRecord = NUM_SECONDS; 
	if(argc>1)
	{
		//first argument is the filename
		global_filename = argv[1];
	}
	if(argc>2)
	{
		//second argument is the time it will record
		fSecondsRecord = atof(argv[2]);
	}
	//use audio_spi\spidevicesselect.exe to find the name of your devices, only exact name will be matched (name as detected by spidevicesselect.exe)  
	//2020oct03, spi, begin
	//global_audiodevicename="E-MU ASIO"; //"Wave (2- E-MU E-DSP Audio Proce"
	mySPIAudioDevice.global_audioinputdevicename="E-MU ASIO";
	//string audiodevicename="Wave (2- E-MU E-DSP Audio Proce"; //"E-MU ASIO"
	if(argc>3)
	{
		//global_audiodevicename = argv[3]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
		mySPIAudioDevice.global_audioinputdevicename = argv[3];
	}
	//global_inputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	//global_inputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	mySPIAudioDevice.global_inputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	mySPIAudioDevice.global_inputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	//global_inputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	//global_inputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	//global_inputAudioChannelSelectors[0] = 8; // on emu patchmix ASIO device channel 9 (left)
	//global_inputAudioChannelSelectors[1] = 9; // on emu patchmix ASIO device channel 10 (right)
	//global_inputAudioChannelSelectors[0] = 10; // on emu patchmix ASIO device channel 11 (left)
	//global_inputAudioChannelSelectors[1] = 11; // on emu patchmix ASIO device channel 12 (right)
	if(argc>4)
	{
		//global_inputAudioChannelSelectors[0]=atoi(argv[4]); //0 for first asio channel (left) or 2, 4, 6, etc.
		mySPIAudioDevice.global_inputAudioChannelSelectors[0]=atoi(argv[4]); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(argc>5)
	{
		//global_inputAudioChannelSelectors[1]=atoi(argv[5]); //1 for second asio channel (right) or 3, 5, 7, etc.
		mySPIAudioDevice.global_inputAudioChannelSelectors[1]=atoi(argv[5]); //1 for second asio channel (right) or 3, 5, 7, etc.
	}
	//use audio_spi\spidevicesselect.exe to find the name of your devices, only exact name will be matched (name as detected by spidevicesselect.exe)  
	mySPIAudioDevice.global_audiooutputdevicename=""; //defaults to empty for no audio output
	//mySPIAudioDevice.global_audiooutputdevicename=="E-MU ASIO";
	//string audiodevicename="Wave (2- E-MU E-DSP Audio Proce"; //"E-MU ASIO"
	if(argc>6)
	{
		//global_audiodevicename = argv[3]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
		mySPIAudioDevice.global_audiooutputdevicename = argv[6];
	}
	mySPIAudioDevice.global_outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	mySPIAudioDevice.global_outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	if(argc>7)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[0]=atoi(argv[7]); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(argc>8)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[1]=atoi(argv[8]); //1 for second asio channel (right) or 3, 5, 7, etc.
	}
	if(argc>9)
	{
		global_modestring = argv[9];
		if(!global_modestring.empty() && global_modestring=="PAUSEONSTART") global_pauserecording = true;
	}
	//2020oct05, spi, begin
	//if(argc>10)
	//{
	//	global_monitor = argv[10];
	//}
	//2020oct05, spi, end
	mySPIAudioDevice.pFILE = fopen("devices.txt", "w");
	//2020oct03, spi, end

    //Auto-reset, initially non-signaled event 
    g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    //Add the break handler
    ::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    //PaStreamParameters  inputParameters;
    //PaStreamParameters  outputParameters;
    //PaStream*           stream;
    //PaError             err = paNoError;
    //paTestData          data = {0};
    //unsigned            delayCntr;
    float            delayCntr;
    unsigned            numSamples;
    unsigned            numBytes;
 
    printf("patest_record.c\n"); fflush(stdout);
 
    // We set the ring buffer size to about 500 ms
    numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.5 * NUM_CHANNELS));
    numBytes = numSamples * sizeof(SAMPLE);
    global_paTestData.ringBufferData = (SAMPLE *) PaUtil_AllocateMemory( numBytes );
    if(global_paTestData.ringBufferData == NULL )
    {
        printf("Could not allocate ring buffer data.\n");
        goto done;
    }
 
    if (PaUtil_InitializeRingBuffer(&global_paTestData.ringBuffer, sizeof(SAMPLE), numSamples, global_paTestData.ringBufferData) < 0)
    {
        printf("Failed to initialize ring buffer. Size is not power of 2 ??\n");
        goto done;
    }
 
    err = Pa_Initialize();
    if( err != paNoError ) goto done;
 
	if(0)
	{
		mySPIAudioDevice.global_inputParameters.device = Pa_GetDefaultInputDevice(); //default input device 
		
		if (mySPIAudioDevice.global_inputParameters.device == paNoDevice) 
		{
			fprintf(stderr,"Error: No default input device.\n");
			goto done;
		}
		mySPIAudioDevice.global_inputParameters.channelCount = 2;                    // stereo input
		mySPIAudioDevice.global_inputParameters.sampleFormat = PA_SAMPLE_TYPE;
		mySPIAudioDevice.global_inputParameters.suggestedLatency = Pa_GetDeviceInfo( mySPIAudioDevice.global_inputParameters.device )->defaultLowInputLatency;
		mySPIAudioDevice.global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//////////////////////////////
		//audio input device selection
		//////////////////////////////
		mySPIAudioDevice.SelectAudioInputDevice();
	}

	///////////////////////////////
	//audio output device selection
	///////////////////////////////
	mySPIAudioDevice.SelectAudioOutputDevice();
	PaStreamParameters* p_outputParameters = NULL;
	if(mySPIAudioDevice.global_audiooutputdevicename!="")
	{
		p_outputParameters = &mySPIAudioDevice.global_outputParameters;
	}
	//2020oct03, spi, end

    //Record some audio. -------------------------------------------- 
    err = Pa_OpenStream(
              &stream,
			  //2020oct03, spi, begin
              //&global_inputParameters,
              &mySPIAudioDevice.global_inputParameters,
              //NULL,                  // &outputParameters, 
			  p_outputParameters,
			  //2020oct03, spi, end
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      // we won't output out of range samples so don't bother clipping them 
              recordCallback,
              &global_paTestData);
 
    if( err != paNoError ) goto done;
 
	if(0)
	{
		// Open the raw audio 'cache' file...
        global_paTestData.file = fopen(FILE_NAME, "wb");
		if (global_paTestData.file == 0) goto done;
	}
	else
	{
		// Open the wav audio 'cache' file...
        global_paTestData.file = fopen(global_filename.c_str(), "wb");
		if (global_paTestData.file == 0) goto done;
	}

    //Start the file writing thread 
    if(0)
	{
		err = startThread(&global_paTestData, threadFunctionWriteToRawFile);
	}
	else
	{
		err = startThread(&global_paTestData, threadFunctionWriteToWavFile);
	}
	if( err != paNoError ) goto done;
 
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording to '" FILE_NAME "' for %f seconds!! Press P to pause/unpause recording. ===\n", fSecondsRecord); fflush(stdout);
 
    // Note that the RECORDING part is limited with TIME, not size of the file and/or buffer, so you can
    //   increase NUM_SECONDS until you run out of disk 
    delayCntr = 0;
    //while( delayCntr++ < fSecondsRecord )
    while( delayCntr < fSecondsRecord )
    {
        //printf("index = %d\n", data.frameIndex ); fflush(stdout);
        printf("rec time = %f\n", delayCntr ); fflush(stdout);
		if(_kbhit() && _getch()=='p')
		{
			if(global_pauserecording==false)
			{
				global_pauserecording=true;
				printf("pause pressed\n"); fflush(stdout);
			}
			else
			{
				global_pauserecording=false;
				printf("unpause pressed\n"); fflush(stdout);
			}
		}
        Pa_Sleep(1000);
		if(!global_pauserecording) delayCntr++;
    }
    if( err < 0 ) goto done;
 

    printf("Done.\n"); fflush(stdout);

 
done:
	Terminate(NULL);
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          // Always return 0 or 1, but no other return codes. 
    }

	//2020oct03, spi, begin
	if(mySPIAudioDevice.pFILE) fclose(mySPIAudioDevice.pFILE);
	//2020oct03, spi, end                                                                                                                                                                                                             
    return err;
}
//2020oct05, spi, begin
/**/
//2020oct05, spi, end


int Terminate(FILE* pFILE)
{
	//2020oct05, spi, begin
	if(pFILE==NULL) pFILE=stderr;
	//2020oct05, spi, end

    err = Pa_CloseStream( stream );
    if( err != paNoError ) 
	{
		if(pFILE)
		{
			fprintf( pFILE, "An error occured while using the portaudio stream\n" );
			fprintf( pFILE, "Error number: %d\n", err );
			fprintf( pFILE, "Error message: %s\n", Pa_GetErrorText( err ) );
		}
		err = 1;          // Always return 0 or 1, but no other return codes. 
		return err;
	}
    // Stop the thread 
    err = stopThread(&global_paTestData);
    if( err != paNoError )
	{
		if(pFILE)
		{
			fprintf( pFILE, "An error occured while using the portaudio stream\n" );
			fprintf( pFILE, "Error number: %d\n", err );
			fprintf( pFILE, "Error message: %s\n", Pa_GetErrorText( err ) );
		}
		err = 1;          // Always return 0 or 1, but no other return codes. 
		return err;
	}
 
    // Close file 
    fclose(global_paTestData.file);
    global_paTestData.file = 0;

    Pa_Terminate();
    if(global_paTestData.ringBufferData )       // Sure it is NULL or valid. 
        PaUtil_FreeMemory(global_paTestData.ringBufferData );

	if(pFILE)
	{
		fprintf(pFILE, "Exiting!\n"); fflush(pFILE);
	}
	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
	return 0;
}
 
//Called by the operating system in a separate thread to handle an app-terminating event. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        // CTRL_C_EVENT - Ctrl+C was pressed 
        // CTRL_BREAK_EVENT - Ctrl+Break was pressed 
        // CTRL_CLOSE_EVENT - Console window was closed 
		Terminate(NULL);
        // Tell the main thread to exit the app 
        ::SetEvent(g_hTerminateEvent);
        return TRUE;
    }

    //Not an event handled by this function.
    //The only events that should be able to
	//reach this line of code are events that
    //should only be sent to services. 
    return FALSE;
}
