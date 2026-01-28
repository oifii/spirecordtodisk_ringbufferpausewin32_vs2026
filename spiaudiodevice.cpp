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
 //2020oct05, spi, begin
//#include "stdafx.h"
//2020oct05, spi, end

#include "portaudio.h"

#ifdef WIN32
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include "defs.h"

#include "spiaudiodevice.h"


SPIAudioDevice::SPIAudioDevice()
{
	global_audioinputdevicename = "";
	global_audiooutputdevicename = "";
	//2020dec16, spi, begin
	global_audioinputhostapi = ""; 
	global_audiooutputhostapi = ""; 
	//2020dec16, spi, end
	pFILE = NULL;
}

SPIAudioDevice::~SPIAudioDevice()
{
	
}


bool SPIAudioDevice::SelectAudioInputDevice()
{
	//2020oct05, spi, begin
	global_inputdevicemap.clear();
	//2020oct05, spi, end
	const PaDeviceInfo* deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i<numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		string devicenamestring = deviceInfo->name;
		global_inputdevicemap.insert(pair<string, int>(devicenamestring, i));
		if (pFILE) fprintf(pFILE, "id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	//2020dec16, spi, begin
	std::size_t found = global_audioinputdevicename.find(":");
	//if a semicolon is used to combined the hostapi name with the audio device name
	if (found != std::string::npos)
	{
		//host api may be: "MME", "Windows DirectSound", "ASIO", "Windows WASAPI", "Windows WDM-KS"
		global_audioinputhostapi = global_audioinputdevicename.substr(0,found);
		global_audioinputdevicename = global_audioinputdevicename.substr(found);
	}
	//2020dec16, spi, end


	int deviceid = Pa_GetDefaultInputDevice(); // default input device 
	std::map<string, int>::iterator it;
	it = global_inputdevicemap.find(global_audioinputdevicename);
	//2020dec16, spi, begin
	if (!global_audioinputdevicename.empty())
	{
		if (global_audioinputhostapi.empty())
		{
			if (it != global_inputdevicemap.end())
			{
				deviceid = (*it).second;
				//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
				deviceInfo = Pa_GetDeviceInfo(deviceid);
				//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
				//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
			}
			else
			{
				//Pa_Terminate();
				//return -1;
				//printf("error, audio device not found, will use default\n");
				//MessageBox(win,"error, audio device not found, will use default\n",0,0);
				deviceid = Pa_GetDefaultInputDevice();
			}
		}
		else
		{
			bool match = false;
			for (int i = 0; i < numDevices; i++)
			{
				deviceInfo = Pa_GetDeviceInfo(i);
				string devicenamestring = deviceInfo->name;
				//int indexhostapi = deviceInfo->hostApi;
				const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
				//printf("[ Default %s Input", hostInfo->name);
				string hostapistring = hostInfo->name;
				if ( (devicenamestring== global_audioinputdevicename) && (hostapistring==global_audioinputhostapi) )
				{
					match = true;
					deviceid = i;
					break;
				}
			}
			if (match == false)
			{
				deviceid = Pa_GetDefaultInputDevice();
			}
		}
	}
	//2020dec16, spi, end

	global_inputParameters.device = deviceid;
	if (global_inputParameters.device == paNoDevice)
	{
		//MessageBox(win,"error, no default input device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_inputParameters.channelCount = NUM_CHANNELS;
	global_inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_inputParameters.suggestedLatency = Pa_GetDeviceInfo(global_inputParameters.device)->defaultLowOutputLatency;
	//inputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioInputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioInputInfo.hostApiType = paASIO;
	global_asioInputInfo.version = 1;
	global_asioInputInfo.flags = paAsioUseChannelSelectors;
	global_asioInputInfo.channelSelectors = global_inputAudioChannelSelectors;
	if (deviceid == Pa_GetDefaultInputDevice())
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO)
	{
		global_inputParameters.hostApiSpecificStreamInfo = &global_asioInputInfo;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS)
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}



bool SPIAudioDevice::SelectAudioOutputDevice()
{
	//2020oct05, spi, begin
	global_outputdevicemap.clear();
	//2020oct05, spi, end
	const PaDeviceInfo* deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i<numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		string devicenamestring = deviceInfo->name;
		global_outputdevicemap.insert(pair<string, int>(devicenamestring, i));
		if (pFILE) fprintf(pFILE, "id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	//2020dec16, spi, begin
	std::size_t found = global_audiooutputdevicename.find(":");
	//if a semicolon is used to combined the hostapi name with the audio device name
	if (found != std::string::npos)
	{
		//host api may be: "MME", "Windows DirectSound", "ASIO", "Windows WASAPI", "Windows WDM-KS"
		global_audiooutputhostapi = global_audiooutputdevicename.substr(0, found);
		global_audiooutputdevicename = global_audiooutputdevicename.substr(found);
	}
	//2020dec16, spi, end


	int deviceid = Pa_GetDefaultOutputDevice(); // default output device 
	std::map<string, int>::iterator it;
	it = global_outputdevicemap.find(global_audiooutputdevicename);
	//2020dec16, spi, begin
	if (!global_audiooutputdevicename.empty())
	{
		if (global_audiooutputhostapi.empty())
		{
			if (it != global_outputdevicemap.end())
			{
				deviceid = (*it).second;
				//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
				deviceInfo = Pa_GetDeviceInfo(deviceid);
				//assert(outputAudioChannelSelectors[0]<deviceInfo->maxOutputChannels);
				//assert(outputAudioChannelSelectors[1]<deviceInfo->maxOutputChannels);
			}
			else
			{
				//Pa_Terminate();
				//return -1;
				//printf("error, audio device not found, will use default\n");
				//MessageBox(win,"error, audio device not found, will use default\n",0,0);
				deviceid = Pa_GetDefaultOutputDevice();
			}
		}
		else
		{
			bool match = false;
			for (int i = 0; i < numDevices; i++)
			{
				deviceInfo = Pa_GetDeviceInfo(i);
				string devicenamestring = deviceInfo->name;
				//int indexhostapi = deviceInfo->hostApi;
				const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
				//printf("[ Default %s Input", hostInfo->name);
				string hostapistring = hostInfo->name;
				if ((devicenamestring == global_audiooutputdevicename) && (hostapistring == global_audiooutputhostapi))
				{
					match = true;
					deviceid = i;
					break;
				}
			}
			if (match == false)
			{
				deviceid = Pa_GetDefaultOutputDevice();
			}
		}
	}
	//2020dec16, spi, end


	if (it != global_outputdevicemap.end())
	{
		deviceid = (*it).second;
		//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
		//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
	}
	else
	{
		//Pa_Terminate();
		//return -1;
		//printf("error, audio device not found, will use default\n");
		//MessageBox(win,"error, audio device not found, will use default\n",0,0);
		deviceid = Pa_GetDefaultOutputDevice();
	}


	global_outputParameters.device = deviceid;
	if (global_outputParameters.device == paNoDevice)
	{
		//MessageBox(win,"error, no default output device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_outputParameters.channelCount = NUM_CHANNELS;
	global_outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_outputParameters.suggestedLatency = Pa_GetDeviceInfo(global_outputParameters.device)->defaultLowOutputLatency;
	//outputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioOutputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioOutputInfo.hostApiType = paASIO;
	global_asioOutputInfo.version = 1;
	global_asioOutputInfo.flags = paAsioUseChannelSelectors;
	global_asioOutputInfo.channelSelectors = global_outputAudioChannelSelectors;
	if (deviceid == Pa_GetDefaultOutputDevice())
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO)
	{
		global_outputParameters.hostApiSpecificStreamInfo = &global_asioOutputInfo;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS)
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}
