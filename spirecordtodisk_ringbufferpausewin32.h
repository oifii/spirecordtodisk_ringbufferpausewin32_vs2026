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
//#pragma once
#ifndef _SPIRECORDTODISK_RINGBUFFERPAUSEWIN32_H
#define _SPIRECORDTODISK_RINGBUFFERPAUSEWIN32_H


//#define SAMPLE_RATE  (17932) // Test failure to open with this value. 
#define FILE_NAME       "audio_data.raw"
#define NUM_SECONDS     (60)
#define NUM_WRITES_PER_BUFFER   (4)
//#define DITHER_FLAG     (paDitherOff) 
#define DITHER_FLAG     (0) 

typedef struct
{
    unsigned            frameIndex;
    int                 threadSyncFlag;
    SAMPLE             *ringBufferData;
    PaUtilRingBuffer    ringBuffer;
    FILE               *file;
    void               *threadHandle;
} paTestData;

unsigned int NextPowerOf2(unsigned int val);

int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );

int threadFunctionWriteToWavFile(void* ptr);
int threadFunctionWriteToRawFile(void* ptr);
int threadFunctionReadFromRawFile(void* ptr);

typedef int (*ThreadFunctionType)(void*);
PaError startThread( paTestData* pData, ThreadFunctionType fn ); 
int stopThread( paTestData* pData );

//to terminate recording audio
int Terminate(FILE* pFILE);
#endif //_SPIRECORDTODISK_RINGBUFFERPAUSEWIN32_H