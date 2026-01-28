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
#ifndef _SPIAUDIODEVICE_H
#define _SPIAUDIODEVICE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;


class SPIAudioDevice
{
public:
	std::map<string, int> global_inputdevicemap;
	std::map<string, int> global_outputdevicemap;

	PaStream* global_stream;
	PaStreamParameters global_inputParameters;
	PaStreamParameters global_outputParameters;
	PaError global_err;
	string global_audioinputdevicename; // = "";
	string global_audiooutputdevicename; // = "";
	//2020dec16, spi, begin
	string global_audioinputhostapi; // = "";
	string global_audiooutputhostapi; // = "";
	//2020dec16, spi, end
	int global_inputAudioChannelSelectors[2];
	int global_outputAudioChannelSelectors[2];
	PaAsioStreamInfo global_asioInputInfo;
	PaAsioStreamInfo global_asioOutputInfo;

	FILE* pFILE;// = NULL;

public:
	SPIAudioDevice();
	~SPIAudioDevice();

	bool SelectAudioInputDevice();
	bool SelectAudioOutputDevice();
};


#endif //_SPIAUDIODEVICE_H
