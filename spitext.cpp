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


// spitext.cpp : Defines the entry point for the application.
//

//2020oct05, spi, begin
//#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
//2020oct05, spi, end
#include "spiutility.h"
#include "spitext.h"
#include <stdio.h>
#include <assert.h>
#include <ShellAPI.h>

//#include <string>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

//2020oct05, spi, begin
//for the spirecordtodisk_ringbufferpausewin32
#include "portaudio.h"
#include "pa_asio.h"
#include "pa_ringbuffer.h"
#include "pa_util.h"
#include <sndfile.hh>
#include "defs.h"
#include "spiaudiodevice.h"
#include "spirecordtodisk_ringbufferpausewin32.h"
extern SPIAudioDevice mySPIAudioDevice;
extern string global_modestring; // = ""; //if not empty, this string can be "PAUSEONSTART"
//2020oct05, spi, end

using namespace std;


#define MAX_LOADSTRING 256

/*
#define SPICOUNTERMODE_COUNTUP		0
#define SPICOUNTERMODE_COUNTDOWN	1
#define SPICOUNTERMODE_CLOCK		2
*/
#define ID_TIMER_EVERYSECOND	1
#define ID_TIMER_ONCESTART		2
#define ID_TIMER_ONCEKILL		3

// Global Variables:
HINSTANCE hInst;								// current instance
//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={L"spitexttitle"};					// The title bar text
TCHAR szWindowClass[1024]={L"spitextclass"};			// the main window class name

int global_x=0;
int global_y=0;
string global_spitextstring="some text";
HFONT global_hFont;
int global_fontheight=480;
int global_fontwidth=-1; //will be computed within WM_PAINT handler

int global_starttime_sec=-1; //user specified, -1 for not specified
int global_endtime_sec=-1; //user specified, -1 for not specified

int global_timetodisplay_sec; //calculated
DWORD global_startstamp_ms;
DWORD global_nowstamp_ms;

char charbuffer[1024]={""};
char charbuffer_prev[1024]={""};

BYTE global_alpha=220;

//global_monitor is either empty "" (global_x and global_y will be considered absolute) 
//or "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" specifying monitor (global_x and global_y will be considered relative)
//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
string global_monitor = "";
//global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
string global_hmonitor = "";
//global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
string global_hwnd = "";
//global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
string global_windowclass = "";
//global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
string global_windowtitle = "";
//global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
string global_horizontaljustification = "";
//global_verticaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
string global_verticaljustification = "";

//global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
//int global_horizontalforcefit = 0;
float global_horizontalforcefit = 0.0f;
//global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
//int global_verticalforcefit = 0;
float global_verticalforcefit = 0.0f;

int global_fullmonitorssurface = 1; //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors

//new parameters
#include <atlconv.h>
//string global_fontface="Segoe Script"; //see charmap.exe for fontface (Win+R>charmap)
string global_fontface="Arial"; //see charmap.exe for fontface (Win+R>charmap)
int global_idfontcolor=0;
//string global_classname="spicounterclass";
//string global_title="spicountertitle";
//string global_begin="begin.ahk";
string global_begin="";
//string global_starting="starting.ahk";
string global_starting="";
//string global_finishing="finishing.ahk";
string global_finishing="";
//string global_end="end.ahk";
string global_end="";


//keying color (to be made transparent color)
COLORREF global_keyingcolor = RGB(255, 0, 255);


//these structures initialize themselves automatically with the list of monitors 
//and the list of all top-level windows on the screens
MonitorRects _monitors; 
EnumWindowsStruct_spitext _windows;
//initially presume global_x and global_y coordinate absolute
POINT myoutputPOINT;

bool globalxyabsolute = true;
RECT myoutputRECT;

FILE* pFILE=NULL;
//SIZE mySIZE;

int global_textmode = -1; //defaults to -1 to display straight text
int global_textformat = -1; //defaults to -1 to display straight text otherwise used by the counter modes

vector<string> global_countermode;
vector<string> global_counterformat;

int global_countermodeCOUNTUP = 0;
int global_countermodeCOUNTDOWN = 1;
int global_countermodeCLOCK = 2;

int global_counterformatHHMMSS = 0;
int global_counterformatHHMM = 1;
int global_counterformatMMSS = 2;
int global_counterformatHH = 3;
int global_counterformatMM = 4;
int global_counterformatSS = 5;


//2020oct05, spi, begin
//for the spirecordtodisk_ringbufferpausewin32
extern string global_filename;
extern PaError global_err;
extern bool global_prev_pauserecording; //=false;
extern bool global_pauserecording; //=false;

extern paTestData global_paTestData; // = {0};
extern PaStream* stream;
extern PaError err; //= paNoError;

bool global_pausedisabled = false;
bool global_dontcounttimeonpause = false;

DWORD global_pausestartstamp_ms = 0; //when greater than 0, recording is paused, when 0 it is recording (this logic is ok at the scale of the ms on windows os)
DWORD global_pausetimetotal_ms = 0; //total number of ms elapsed while on pause since app start
//2020oct05, spi, end


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);




int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	global_startstamp_ms = GetTickCount();

	LPSTR *szArgList;
	int nArgs;
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvA failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	//2020oct05, spi, begin
	///////////////////
	//read in arguments
	///////////////////
	global_filename = "testrecording.wav"; //usage: spirecord testrecording.wav 10 "E-MU ASIO" 0 1
	float fSecondsRecord = NUM_SECONDS; 
	if(nArgs>1)
	{
		//first argument is the filename
		global_filename = szArgList[1];
	}
	if(nArgs>2)
	{
		//second argument is the time it will record
		//will record until task is killed if fSecondsRecord is negative and both global_starttime_sec and global_endtime_sec are also negative
		fSecondsRecord = atof(szArgList[2]); //fSecondsRecord greater than 0 is only used if both global_starttime_sec and global_endtime_sec are not used
	}
	//use audio_spi\spidevicesselect.exe to find the name of your devices, only exact name will be matched (name as detected by spidevicesselect.exe)  
	//2020oct03, spi, begin
	//global_audiodevicename="E-MU ASIO"; //"Wave (2- E-MU E-DSP Audio Proce"
	mySPIAudioDevice.global_audioinputdevicename="E-MU ASIO";
	//string audiodevicename="Wave (2- E-MU E-DSP Audio Proce"; //"E-MU ASIO"
	if(nArgs>3)
	{
		//global_audiodevicename = szArgList[3]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
		mySPIAudioDevice.global_audioinputdevicename = szArgList[3];
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
	if(nArgs>4)
	{
		//global_inputAudioChannelSelectors[0]=atoi(szArgList[4]); //0 for first asio channel (left) or 2, 4, 6, etc.
		mySPIAudioDevice.global_inputAudioChannelSelectors[0]=atoi(szArgList[4]); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(nArgs>5)
	{
		//global_inputAudioChannelSelectors[1]=atoi(szArgList[5]); //1 for second asio channel (right) or 3, 5, 7, etc.
		mySPIAudioDevice.global_inputAudioChannelSelectors[1]=atoi(szArgList[5]); //1 for second asio channel (right) or 3, 5, 7, etc.
	}
	//use audio_spi\spidevicesselect.exe to find the name of your devices, only exact name will be matched (name as detected by spidevicesselect.exe)  
	mySPIAudioDevice.global_audiooutputdevicename=""; //defaults to empty for no audio output
	//mySPIAudioDevice.global_audiooutputdevicename=="E-MU ASIO";
	//string audiodevicename="Wave (2- E-MU E-DSP Audio Proce"; //"E-MU ASIO"
	if(nArgs>6)
	{
		//global_audiodevicename = szArgList[3]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
		mySPIAudioDevice.global_audiooutputdevicename = szArgList[6];
	}
	mySPIAudioDevice.global_outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	mySPIAudioDevice.global_outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	if(nArgs>7)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[0]=atoi(szArgList[7]); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(nArgs>8)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[1]=atoi(szArgList[8]); //1 for second asio channel (right) or 3, 5, 7, etc.
	}
	if(nArgs>9)
	{
		global_modestring = szArgList[9];
		if(!global_modestring.empty()) // && global_modestring=="PAUSEONSTART") global_pauserecording = true;
		{
			if (global_modestring.find("DISABLEPAUSE") != std::string::npos) 
			{
				//found!
				global_pausedisabled = true;
			}
			if (!global_pausedisabled && (global_modestring.find("PAUSEONSTART") != std::string::npos)) 
			{
				//found!
				global_pauserecording = true;
			}
			if (!global_pausedisabled && (global_modestring.find("DONTCOUNTTIMEONPAUSE") != std::string::npos)) 
			{
				//found!
				global_dontcounttimeonpause = true;
			}
		}
	}
	int nargs_recordtodisk = 9;
	//2020oct05, spi, end
	if(nArgs>(1+nargs_recordtodisk))
	{
		global_spitextstring = szArgList[(1+nargs_recordtodisk)]; //any text to be displayed or "COUNTUP", "COUNTDOWN", "CLOCK", or a combinaison of "COUNTUP", "COUNTDOWN", "CLOCK" along with "HH", "MM", "SS", 
											 //"HH:MM:SS", "HH:MM", "MM:SS", "COUNTUP HH:MM:SS", "COUNTDOWN MM:SS", "CLOCK HH:MM:SS"
	}
	if(nArgs>(2+nargs_recordtodisk))
	{
		global_starttime_sec = atoi(szArgList[(2+nargs_recordtodisk)]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK" 
													//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before initiating global_spitextstring display
	}
	if(nArgs>(3+nargs_recordtodisk))
	{
		global_endtime_sec = atoi(szArgList[(3+nargs_recordtodisk)]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK"
												//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before killing global_spitextstring display
	}
	if(nArgs>(4+nargs_recordtodisk))
	{
		global_x = atoi(szArgList[(4+nargs_recordtodisk)]);
	}
	if(nArgs>(5+nargs_recordtodisk))
	{
		global_y = atoi(szArgList[(5+nargs_recordtodisk)]);
	}
	if(nArgs>(6+nargs_recordtodisk))
	{
		global_fontheight = atoi(szArgList[(6+nargs_recordtodisk)]);
	}
	//new parameters
	if(nArgs>(7+nargs_recordtodisk))
	{
		global_fontface = szArgList[(7+nargs_recordtodisk)]; 
	}
	if(nArgs>(8+nargs_recordtodisk))
	{
		global_idfontcolor = atoi(szArgList[(8+nargs_recordtodisk)]); 
	}
	if(nArgs>(9+nargs_recordtodisk))
	{
		wcscpy(szWindowClass, szArgListW[(9+nargs_recordtodisk)]); 
	}
	if(nArgs>(10+nargs_recordtodisk))
	{
		wcscpy(szTitle, szArgListW[(10+nargs_recordtodisk)]); 
	}
	if(nArgs>(11+nargs_recordtodisk))
	{
		global_begin = szArgList[(11+nargs_recordtodisk)]; 
	}
	if(nArgs>(12+nargs_recordtodisk))
	{
		global_starting = szArgList[(12+nargs_recordtodisk)]; 
	}
	if(nArgs>(13+nargs_recordtodisk))
	{
		global_finishing = szArgList[(13+nargs_recordtodisk)]; 
	}
	if(nArgs>(14+nargs_recordtodisk))
	{
		global_end = szArgList[(14+nargs_recordtodisk)]; 
	}
	if(nArgs>(15+nargs_recordtodisk))
	{
		global_alpha = atoi(szArgList[(15+nargs_recordtodisk)]); 
	}
	if(nArgs>(16+nargs_recordtodisk))
	{
		global_monitor = szArgList[(16+nargs_recordtodisk)]; //global_monitor is either empty "" (global_x and global_y will be considered absolute) or i.e. "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" etc. specifying monitor (global_x and global_y will be considered relative)
										//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
	}
	if(nArgs>(17+nargs_recordtodisk))
	{
		global_hmonitor = szArgList[(17+nargs_recordtodisk)]; //global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
	}
	if(nArgs>(18+nargs_recordtodisk))
	{
		global_hwnd = szArgList[(18+nargs_recordtodisk)]; //global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
	}
	if(nArgs>(19+nargs_recordtodisk))
	{
		global_windowclass = szArgList[(19+nargs_recordtodisk)]; //global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
	}
	if(nArgs>(20+nargs_recordtodisk))
	{
		global_windowtitle = szArgList[(20+nargs_recordtodisk)]; //global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
	}
	if(nArgs>(21+nargs_recordtodisk))
	{
		global_horizontaljustification = szArgList[(21+nargs_recordtodisk)]; //global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
	}
	if(nArgs>(22+nargs_recordtodisk))
	{
		global_verticaljustification = szArgList[(22+nargs_recordtodisk)]; //global_verticaljustification is either an empty string "" (global_y and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
	}
	if(nArgs>(23+nargs_recordtodisk))
	{
		//global_horizontalforcefit = atoi(szArgList[23]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		global_horizontalforcefit = atof(szArgList[(23+nargs_recordtodisk)]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		if(global_horizontalforcefit<0.0f) global_horizontalforcefit=0.0f;
		if(global_horizontalforcefit>1.0f) global_horizontalforcefit=1.0f;
	}
	if(nArgs>(24+nargs_recordtodisk))
	{
		//global_vecticalforcefit = atoi(szArgList[24]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		global_verticalforcefit = atof(szArgList[(24+nargs_recordtodisk)]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		if(global_verticalforcefit<0.0f) global_verticalforcefit=0.0f;
		if(global_verticalforcefit>1.0f) global_verticalforcefit=1.0f;
	}
	if(nArgs>(25+nargs_recordtodisk))
	{
		global_fullmonitorssurface = atoi(szArgList[(25+nargs_recordtodisk)]); //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors
	}

	LocalFree(szArgList);
	LocalFree(szArgListW);

	//2020oct05, spi, begin
	mySPIAudioDevice.pFILE = fopen("devices.txt", "w");	
	//2020oct05, spi, end

	pFILE = fopen("debug.txt", "w");
	if(pFILE)
	{
		fprintf(pFILE, "global_horizontalforcefit is %f\n", global_horizontalforcefit);
		fprintf(pFILE, "global_verticalforcefit is %f\n", global_verticalforcefit);
		fprintf(pFILE, "\n\n");
	}
	//global_startstamp_ms = GetTickCount();

	//init possible clock modes and formats
	global_countermode.push_back("COUNTUP");
	global_countermode.push_back("COUNTDOWN");
	global_countermode.push_back("CLOCK");
	global_counterformat.push_back("HH:MM:SS");
	global_counterformat.push_back("HH:MM");
	global_counterformat.push_back("MM:SS");
	global_counterformat.push_back("HH");
	global_counterformat.push_back("MM");
	global_counterformat.push_back("SS");

	/*
	int global_countermodeCOUNTUP = 0;
	int global_countermodeCOUNTDOWN = 1;
	int global_countermodeCLOCK = 2;
	*/
	//global_textmode = -1; //default to display straight text
	if ( (global_spitextstring.find(global_countermode[global_countermodeCOUNTUP]) != string::npos) )
	{
		global_textmode = global_countermodeCOUNTUP; 
	}
	else if ( (global_spitextstring.find(global_countermode[global_countermodeCOUNTDOWN]) != string::npos) )
	{
		global_textmode = global_countermodeCOUNTDOWN; 
	}
	else if ( (global_spitextstring.find(global_countermode[global_countermodeCLOCK]) != string::npos) )
	{
		global_textmode = global_countermodeCLOCK; 
	}


	/*
	int global_counterformatHHMMSS = 0;
	int global_counterformatHHMM = 1;
	int global_counterformatMMSS = 2;
	int global_counterformatHH = 3;
	int global_counterformatMM = 4;
	int global_counterformatSS = 5;
	*/
	if(global_textmode>-1)
	{
		//some counter mode, set counter initial time to be displayed
		if(global_starttime_sec>-1)
		{
			global_timetodisplay_sec = global_starttime_sec;
		}
		else
		{
			global_timetodisplay_sec = 0;
		}

		global_textformat = global_counterformatHHMMSS; //default when unspecified by user
		//some counter mode, check if counter format specified
		if( (global_spitextstring.find(global_counterformat[global_counterformatHHMMSS]) != string::npos) )
		{
			global_textformat = global_counterformatHHMMSS;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatHHMM]) != string::npos) )
		{
			global_textformat = global_counterformatHHMM;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatMMSS]) != string::npos) )
		{
			global_textformat = global_counterformatMMSS;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatHH]) != string::npos) )
		{
			global_textformat = global_counterformatHH;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatMM]) != string::npos) )
		{
			global_textformat = global_counterformatMM;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatSS]) != string::npos) )
		{
			global_textformat = global_counterformatSS;
		}
	}
	else
	{
		//string display mode, use start and end time as delay and kill time
		if(global_starttime_sec<=0) global_starttime_sec = 0; //no delay start timer will be set
		if(global_endtime_sec<=0) global_endtime_sec = -1; //no ending timer will be set
		if( (global_endtime_sec-global_starttime_sec)<=0 ) global_endtime_sec = -1; //no ending timer will be set
	}

	//2020oct05, spi, begin
	if(fSecondsRecord>0 && ((global_starttime_sec<0 && global_endtime_sec<0)
							|| ((global_textmode == global_countermodeCLOCK) && (global_endtime_sec <= global_starttime_sec))
							|| ((global_textmode < 0) && (global_endtime_sec <= global_starttime_sec))
							) )
	{
		if (global_textmode < 0)
		{
			if (global_starttime_sec < 0) global_starttime_sec = 0;
			global_endtime_sec = global_starttime_sec + fSecondsRecord;
		}
		else if(global_textmode == global_countermodeCLOCK)
		{
			if(global_starttime_sec<0) global_starttime_sec = 0;
			global_endtime_sec = global_starttime_sec+fSecondsRecord;
		}
		else if(global_textmode == global_countermodeCOUNTUP)
		{
			global_starttime_sec = 0;
			global_endtime_sec = fSecondsRecord;
		}
		else if(global_textmode == global_countermodeCOUNTDOWN)
		{
			global_starttime_sec = fSecondsRecord;
			global_endtime_sec = 0;
		}
		global_timetodisplay_sec = global_starttime_sec;
	}
	//2020oct05, spi, end


	//these structures initialize themselves automatically with the list of monitors 
	//and the list of all top-level windows on the screens
	//MonitorRects _monitors; 
	//EnumWindowsStruct_spitext _windows;
	if(pFILE)
	{
		int numberofmonitor=_monitors.rcMonitors.size();
		if(numberofmonitor>0)
		{
			//idmonitor is valid
			for(int indexmonitor=0; indexmonitor<numberofmonitor; indexmonitor++)
			{
				RECT myRECT = _monitors.rcMonitors[indexmonitor];
				HMONITOR hmonitor = _monitors.hMonitors[indexmonitor];
				fprintf(pFILE, "detecting monitors id %d (left %d, right %d, top %d, bottom %d)\n", indexmonitor+1, myRECT.left, myRECT.right, myRECT.top, myRECT.bottom);
				stringstream ss;
				ss << std::hex << hmonitor;
				string hexstring(ss.str());
				fprintf(pFILE, "detecting monitors id %d (hmonitor: dec %d, hex 0x%s)\n", indexmonitor+1, hmonitor, hexstring.c_str());
			}
			fprintf(pFILE, "combined global rectangle from all monitors (left %d, right %d, top %d, bottom %d)\n", _monitors.rcCombined.left, _monitors.rcCombined.right, _monitors.rcCombined.top, _monitors.rcCombined.bottom);
		}
		else
		{
			fprintf(pFILE, "error, not detecting any monitors!\n");
			fclose(pFILE);
			return(FALSE);
		}
	}

#ifdef _DEBUG
	if(pFILE)
	{
		fprintf(pFILE, "\ndetecting the following windows (window classname listing):\n\n");
		if(_windows.hwndstringmap_class.size()>1)
		{
			std::map<HWND, wstring>::iterator it;
			for (it = _windows.hwndstringmap_class.begin(); it != _windows.hwndstringmap_class.end(); it++)
			{
				HWND myHWND = it->first;
				wstring mywindowclassw = it->second;
				string mywindowclass = "";
				if(!mywindowclassw.empty()) mywindowclass = utf8_encode(mywindowclassw);
				fprintf(pFILE, "hwnd,windowclassname %d,%40s\n", myHWND, mywindowclass.c_str());
			}
		}
		fprintf(pFILE, "\n");
		fprintf(pFILE, "\ndetecting the following windows (window title listing):\n\n");
		if(_windows.hwndstringmap_title.size()>1)
		{
			std::map<HWND, wstring>::iterator it;
			for (it = _windows.hwndstringmap_title.begin(); it != _windows.hwndstringmap_title.end(); it++)
			{
				HWND myHWND = it->first;
				wstring mywindowtitlew = it->second;
				string mywindowtitle = "";
				if(!mywindowtitlew.empty()) mywindowtitle = utf8_encode(mywindowtitlew);
				fprintf(pFILE, "hwnd,windowtitle %d,%40s\n", myHWND, mywindowtitle.c_str());
			}
		}
		fflush(pFILE);
	}
#endif

	//initially presume global_x and global_y coordinate absolute
	//POINT myoutputPOINT;
	myoutputPOINT.x = global_x;
	myoutputPOINT.y= global_y;
	//bool globalxyabsolute = true;
	//RECT myoutputRECT;
	
	HMONITOR myHMONITOR = NULL;
	std::map<HMONITOR, RECT>::iterator it;
	if(global_fullmonitorssurface) 
	{
		//default to the rectangle made from all monitors combined
		myoutputRECT = _monitors.rcCombined;
	}
	else
	{
		//fit to one monitor, rectrict from displaying string extending over multiple monitors
		//HMONITOR myHMONITOR;
		myHMONITOR = MonitorFromPoint(myoutputPOINT, MONITOR_DEFAULTTONEAREST);
		//std::map<HMONITOR, RECT>::iterator it;
		it = _monitors.hmapMonitors.find(myHMONITOR);
		if (it != _monitors.hmapMonitors.end())
		{
			myoutputRECT = (*it).second;
		}
		else
		{
			assert(false);
			return(FALSE);
		}
	}
	if(!global_monitor.empty())
	{
		int idmonitor = atoi(global_monitor.substr(0,1).c_str());
		int idsubmonitor = 0;
		int numsubmonitors = 1;
		if(idmonitor==0) idmonitor=1;
		if(idmonitor>0 && idmonitor<10)
		{
			if(idmonitor <= _monitors.rcMonitors.size())
			{
				//idmonitor is valid
				globalxyabsolute = false;
				myoutputRECT = _monitors.rcMonitors[idmonitor-1];
			}
			if( (global_monitor.size()>2) && ((global_monitor.substr(1,1)==".")||(global_monitor.substr(1,1)==":")) )
			{
				if(global_monitor.substr(1,1)==".")
				{
					idsubmonitor = atoi(global_monitor.substr(2,1).c_str());
					if(idsubmonitor<1) idsubmonitor=1;
					if(idsubmonitor>4) idsubmonitor=4;
					numsubmonitors = 4;
				}
				else if(global_monitor.substr(1,1)==":")
				{
					idsubmonitor = atoi(global_monitor.substr(2,1).c_str());
					if(idsubmonitor==1 && (global_monitor.size()>3))
					{
						//use second digit if present and valid
						int seconddigit = atoi(global_monitor.substr(3,1).c_str());
						if(seconddigit>-1 && seconddigit<7) idsubmonitor=10+seconddigit;
					}
					if(idsubmonitor<1) idsubmonitor=1;
					if(idsubmonitor>16) idsubmonitor=16;
					numsubmonitors = 16;
				}
				if(idsubmonitor>0 && idsubmonitor<17)
				{
					//idsubmonitor is valid
					//divbyfour=true;
					//now, let's divide myoutputRECT by 4 same size rectangles
					//based on https://stackoverflow.com/questions/6190019/split-a-rectangle-into-equal-sized-rectangles
					RECT original = myoutputRECT;
					//int numsubmonitors = 1;
					//if (global_monitor.substr(1,1)==".") numsubmonitors = 4;
					//else if (global_monitor.substr(1,1)==":") numsubmonitors = 16;
					int columns = ceil(sqrt((float)numsubmonitors));
					int fullrows = numsubmonitors / columns;
					int orphans = numsubmonitors % columns;   // how many 'odd-sized' ones on our bottom row.
					//calculate output width and height
					int width =  (original.right-original.left)/columns; //original.width/ columns;
					int height = (original.bottom-original.top)/(orphans == 0 ? fullrows : (fullrows+1)); //original.height / (orphans == 0 ? fullrows : (fullrows+1)); // reduce height if there are orphans
					//calculate output rectangles
					RECT output[16];
					assert(numsubmonitors<=16);
					int i=-1;
					//to be similar to displayfusion software installed on remotedroide
					//i.e. second monitor's submonitors are specified like this
					//2.1 2.3
					//2.2 2.4
					for (int x = 0; x < columns; ++x) //for (int y = 0; y < fullrows; ++y)
					{
						for (int y = 0; y < fullrows; ++y) //for (int x = 0; x < columns; ++x)
						{
							i++;
							//submonitor "X.1"
							output[i].left = original.left + x * width; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.2"
							output[i].top = original.top + y * height; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.3"
							output[i].right = output[i].left + width; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.4"
							output[i].bottom = output[i].top + height; //output.push_back(CRect(x * width, y * height, width, height));
						}
					}
					myoutputRECT = output[idsubmonitor-1];
				}
			}
		}
		if(pFILE)
		{
			fprintf(pFILE, "\nmonitor specified by %s\n", global_monitor.c_str());
			fprintf(pFILE, "idmonitor is %d, idsubmonitor is %d, number of submonitors is %d\n", idmonitor, idsubmonitor, numsubmonitors);
			fprintf(pFILE, "output rect is now, left %d right %d top %d bottom %d\n", myoutputRECT.left, myoutputRECT.right, myoutputRECT.top, myoutputRECT.bottom);
		}
	}
	else if(!global_hmonitor.empty())
	{
		//todo: for hex string, use strtol https://en.cppreference.com/w/c/string/byte/strtol
		HMONITOR suppliedhmonitor;
		if(global_hmonitor.size()>3 && global_hmonitor.substr(0,2)=="0x") 
		{
			global_hmonitor = global_hmonitor.substr(2);
			suppliedhmonitor = (HMONITOR) strtol(global_hmonitor.c_str(), NULL, 16);
		}
		else 
		{
			suppliedhmonitor = (HMONITOR) atol(global_hmonitor.c_str());
		}
		it = _monitors.hmapMonitors.find(suppliedhmonitor);
		if (it != _monitors.hmapMonitors.end())
		{
			myHMONITOR = (*it).first;
			myoutputRECT = (*it).second;
		}
		else
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_hmonitor not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		globalxyabsolute = false;
	}
	else if(!global_hwnd.empty())
	{
		//todo: for hex string, use strtol https://en.cppreference.com/w/c/string/byte/strtol
		HWND suppliedhwnd;
		if(global_hwnd.size()>3 && global_hwnd.substr(0,2)=="0x") 
		{
			global_hwnd = global_hwnd.substr(2);
			suppliedhwnd = (HWND) strtol(global_hwnd.c_str(), NULL, 16);
		}
		else 
		{
			suppliedhwnd = (HWND) atol(global_hwnd.c_str());
		}

		BOOL bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_hwnd not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}
	else if(!global_windowclass.empty())
	{
		BOOL bresult = false;
		HWND suppliedhwnd;
		if(!global_windowtitle.empty())
		{
			suppliedhwnd = (HWND) FindWindowA(global_windowclass.c_str(), global_windowtitle.c_str());
		}
		else
		{
			suppliedhwnd = (HWND) FindWindowA(global_windowclass.c_str(), NULL);
		}
		if(suppliedhwnd) bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_windowclass and window title not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}
	else if(!global_windowtitle.empty())
	{
		BOOL bresult = false;
		HWND suppliedhwnd = FindWindowA(NULL, global_windowtitle.c_str());
		if(suppliedhwnd) bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_windowtitle not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}

	/*
	//moved into InitInstance()
	if( (global_textmode==-1) && (global_starttime_sec>0) )
	{
		//todo, launch a timer to wait for global_starttime_sec before display;
	}
	if( (global_textmode==-1) && (global_endtime_sec>0) && ((global_endtime_sec-global_starttime_sec)>0) )
	{
		//todo, launch a timer to wait for global_timetodisplay_sec before killing app;
	}
	*/


	//2020oct05, spi, begin
    float delayCntr;
    unsigned int numSamples;
    unsigned int numBytes;	
    // We set the ring buffer size to about 500 ms
    numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.5 * NUM_CHANNELS));
    numBytes = numSamples * sizeof(SAMPLE);
	global_paTestData.ringBufferData = (SAMPLE *) PaUtil_AllocateMemory( numBytes );
    if(global_paTestData.ringBufferData == NULL )
    {
        if(pFILE) 
		{
			fprintf(pFILE, "Error, could not allocate ring buffer data.\n");
			fflush(pFILE);
		}
		return FALSE;
    }
    if (PaUtil_InitializeRingBuffer(&global_paTestData.ringBuffer, sizeof(SAMPLE), numSamples, global_paTestData.ringBufferData) < 0)
    {
        if(pFILE) 
		{
			fprintf(pFILE, "Error, failed to initialize ring buffer. Size is not power of 2 ??\n");
			fflush(pFILE);
		}        
		return FALSE;
    }
    err = Pa_Initialize();
    if( err != paNoError )
	{
        if(pFILE) 
		{
			fprintf(pFILE, "Error, Pa_Initialize() failed.\n");
			fflush(pFILE);
		}
		return FALSE;
	}
	if(0)
	{
		mySPIAudioDevice.global_inputParameters.device = Pa_GetDefaultInputDevice(); //default input device 
		if (mySPIAudioDevice.global_inputParameters.device == paNoDevice) 
		{
			if(pFILE) 
			{
				fprintf(pFILE,"Error: No default input device.\n");
				fflush(pFILE);
			}
			return FALSE;
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
    if( err != paNoError )
	{
		if(pFILE) 
		{
			fprintf(pFILE,"Error, Pa_OpenStream() failed.\n");
			fflush(pFILE);
		}
		return FALSE;
	}
	if(0)
	{
		// Open the raw audio 'cache' file...
		global_paTestData.file = fopen(FILE_NAME, "wb");
		if (global_paTestData.file == 0)
		{
			if(pFILE) 
			{
				fprintf(pFILE,"Error, cannot open filename %s.\n", FILE_NAME);
				fflush(pFILE);
			}
			return FALSE;
		}
	}
	else
	{
		// Open the wav audio 'cache' file...
		global_paTestData.file = fopen(global_filename.c_str(), "wb");
		if (global_paTestData.file == 0)
		{
			if(pFILE) 
			{
				fprintf(pFILE,"Error, cannot open filename %s.\n", global_filename.c_str());
				fflush(pFILE);
			}
			return FALSE;
		}
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
	if( err != paNoError )
	{
		if(pFILE) 
		{
			fprintf(pFILE,"Error, cannot start the writing thread.\n");
			fflush(pFILE);
		}
		return FALSE;
	}
    err = Pa_StartStream( stream );
    if( err != paNoError )
	{
		if(pFILE) 
		{
			fprintf(pFILE,"Error, Pa_StartStream() failed.\n");
			fflush(pFILE);
		}
		return FALSE;
	}

    //printf("\n=== Now recording to '" FILE_NAME "' for %f seconds!! Press P to pause/unpause recording. ===\n", fSecondsRecord); fflush(stdout);
	if(global_pauserecording)
	{
		global_pausestartstamp_ms  = GetTickCount(); //timestamp pause mode start
	}
	//2020oct05, spi, end


	if(!global_begin.empty()) ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);
	if(!global_starting.empty()) ShellExecuteA(NULL, "open", global_starting.c_str(), "", NULL, nCmdShow);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_CPPMFC_TRANSPARENTTXT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		if(pFILE) fclose(pFILE);
		return FALSE;
	}

	//hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPPMFC_TRANSPARENTTXT));
	//hAccelTable = NULL;
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPPMFC_TRANSPARENTTXT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= L""; //MAKEINTRESOURCE(IDC_CPPMFC_TRANSPARENTTXT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

bool justification(SIZE mySIZE)
{
	bool didjustify = false;

	//horizontal justification
	if(pFILE && !global_horizontaljustification.empty()) 
	{
		fprintf(pFILE, "\nbefore horizontal justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	if(global_horizontaljustification=="left")
	{
	}
	else if(global_horizontaljustification=="right")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) 
		{
			global_x = global_x - mySIZE.cx;
		}
		else 
		{
			global_x = myoutputRECT.right - mySIZE.cx;
		}
		didjustify=true;
	}
	else if(global_horizontaljustification=="center")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_x = global_x - mySIZE.cx/2;
		else global_x = myoutputRECT.left + (myoutputRECT.right-myoutputRECT.left)/2 - mySIZE.cx/2;
		didjustify=true;
	}
	if(pFILE && !global_horizontaljustification.empty()) 
	{
		fprintf(pFILE, "\nafter horizontal justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	//vertical justification
	if(pFILE && !global_verticaljustification.empty()) 
	{
		fprintf(pFILE, "\nbefore vertical justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	if(global_verticaljustification=="top")
	{
	}
	else if(global_verticaljustification=="bottom")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_y = global_y - mySIZE.cy;
		else global_y = myoutputRECT.bottom - mySIZE.cy;
		didjustify=true;
	}
	else if(global_verticaljustification=="center")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_y = global_y - mySIZE.cy/2;
		else global_y = myoutputRECT.top + (myoutputRECT.bottom-myoutputRECT.top)/2 - mySIZE.cy/2;
		didjustify=true;
	}
	if(pFILE && !global_verticaljustification.empty()) 
	{
		fprintf(pFILE, "\nafter vertical justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	return didjustify;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	/*
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	*/
	DWORD Flags1 = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
	//2020oct05, spi, begin
	if(!global_pausedisabled)
	{
		Flags1 = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT; //kicking out WS_EX_NOACTIVATE |, to allow user to see app icon in windows task bar and activate app window so it gets keydown, i.e. P key to pause and unpause recording
	}
	//2020oct05, spi, end
	DWORD Flags2 = WS_POPUP;

	//hWnd = CreateWindowEx(Flags1, szWindowClass, szTitle, Flags2, global_x, global_y, 1920, 1200, 0, 0, hInstance, 0);
	hWnd = CreateWindowEx(Flags1, szWindowClass, szTitle, Flags2, 0, 0, 100, 100, 0, 0, hInstance, 0);
	//hWnd = CreateWindowEx(Flags1, utf8_decode(global_classname).c_str(), szTitle, Flags2, 0, 0, 100, 100, 0, 0, hInstance, 0);
	if (!hWnd)
	{
		return FALSE;
	}
	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"Segoe Script");
	//global_hFont=CreateFontA(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,global_fontface.c_str());
	global_hFont=CreateFontA(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,PROOF_QUALITY,0,global_fontface.c_str());
	

	SIZE mySIZE;
	HDC myHDC = GetDC(hWnd);
	HGDIOBJ prevHGDIOBJ = SelectObject(myHDC, global_hFont);

	if(global_textmode < 0)
	{
		//GetTextExtentPoint32A(myHDC, "88:88:88", strlen("88:88:88"), &mySIZE);
		GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
	}
	else
	{
		/*
		//if global_spitextstring specifies a counter mode and a counter format, fetch it properly
		if(global_textformat<0) GetTextExtentPoint32A(myHDC, "88:88:88", strlen("88:88:88"), &mySIZE);
		else if(global_textformat==global_counterformatHHMMSS) GetTextExtentPoint32A(myHDC, "88:88:88", strlen("88:88:88"), &mySIZE);
		else if(global_textformat==global_counterformatHHMM) GetTextExtentPoint32A(myHDC, "88:88", strlen("88:88"), &mySIZE);
		else if(global_textformat==global_counterformatMMSS) GetTextExtentPoint32A(myHDC, "88:88", strlen("88:88"), &mySIZE);
		else if(global_textformat==global_counterformatHH) GetTextExtentPoint32A(myHDC, "88", strlen("88"), &mySIZE);
		else if(global_textformat==global_counterformatMM) GetTextExtentPoint32A(myHDC, "88", strlen("88"), &mySIZE);
		else if(global_textformat==global_counterformatSS) GetTextExtentPoint32A(myHDC, "88", strlen("88"), &mySIZE);
		//GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
		*/
		//if global_spitextstring specifies a counter mode and a counter format, fetch it properly
		global_spitextstring = "88:88:88";
		if(global_textformat<0) global_spitextstring = "88:88:88";
		else if(global_textformat==global_counterformatHHMMSS) global_spitextstring = "88:88:88";
		else if(global_textformat==global_counterformatHHMM) global_spitextstring = "88:88";
		else if(global_textformat==global_counterformatMMSS) global_spitextstring = "88:88";
		else if(global_textformat==global_counterformatHH) global_spitextstring = "88";
		else if(global_textformat==global_counterformatMM) global_spitextstring = "88";
		else if(global_textformat==global_counterformatSS) global_spitextstring = "88";
		GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
	}

	if(globalxyabsolute==false)
	{
		//relative coordinates
		global_x = myoutputRECT.left + global_x;
		global_y = myoutputRECT.top + global_y;
		if(pFILE) 
		{
			if(global_x > myoutputRECT.right) fprintf(pFILE, "\nwarning, global_x is considered relative and exceeds targeted monitor or window rect\n");
			if(global_y > myoutputRECT.bottom) fprintf(pFILE, "\nwarning, global_y is considered relative and exceeds targeted monitor or window rect\n");			
		}
	}
	else
	{
		//absolute coordinates
		myoutputPOINT.x = global_x;
		myoutputPOINT.y = global_y;
		if(global_fullmonitorssurface)
		{
			bool found=false;
			found = PtInRect(&(_monitors.rcCombined),myoutputPOINT);
			if(!found)
			{
				assert(false);
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, global_x and global_y are considered absolute but do not point inside the combined rect of all monitors\n");
				}
			}
		}
		else
		{
			bool found=false;
			for (int i = 0; i < _monitors.rcMonitors.size(); ++i)
			{
				if (PtInRect(&(_monitors.rcMonitors[i]),myoutputPOINT))
				{
					found=true;
					break;
				}
			}
			if(!found)
			{
				assert(false);
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, global_x and global_y are considered absolute but do not point inside any monitor rects\n");
				}
			}
		}		
	}

	if(!global_horizontaljustification.empty() || !global_verticaljustification.empty())
	{
		justification(mySIZE);
	}
	//if(!globalxyabsolute && (global_horizontalforcefit || global_verticalforcefit) && (global_spitextstring.length()>0) && (mySIZE.cx>0))
	if( (!globalxyabsolute) && ((global_horizontalforcefit>0.0f) || (global_verticalforcefit>0.0f)) && (global_spitextstring.length()>0) && (mySIZE.cx>0))
	{
		if(pFILE) 
		{
			fprintf(pFILE, "\n");
			if((global_horizontalforcefit>0.0f)) fprintf(pFILE, "before horizontal force fit\n");
			if((global_verticalforcefit>0.0f)) fprintf(pFILE, "before vertical force fit\n");
			fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
			fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
		}
		POINT topleftPOINT;
		topleftPOINT.x = global_x;
		topleftPOINT.y = global_y;
		POINT bottomrightPOINT;
		bottomrightPOINT.x = global_x + mySIZE.cx;
		bottomrightPOINT.y = global_y + mySIZE.cy;
		if (!PtInRect(&myoutputRECT,topleftPOINT) || !PtInRect(&myoutputRECT,bottomrightPOINT))
		{
			//if(global_horizontalforcefit)
			if(global_horizontalforcefit>0.0f)
			{
				if(global_x<myoutputRECT.left) global_x = myoutputRECT.left;
				if(global_x>myoutputRECT.right) global_x = myoutputRECT.right;
			}
			//if(global_verticalforcefit)
			if(global_verticalforcefit>0.0f)
			{
				if(global_y<myoutputRECT.top) global_y = myoutputRECT.top;
				if(global_y>myoutputRECT.bottom) global_y = myoutputRECT.bottom;
			}

			int maxwidth = myoutputRECT.right - myoutputRECT.left;
			assert(maxwidth>0);
			int maxheight = myoutputRECT.bottom - myoutputRECT.top;
			assert(maxheight>0);

			//2020sept23, spi, begin
			//global_horizontalforcefit and global_verticalforcefit can now be between 0.0 and 1.0, specifying a percentage (a scale) when greater than 0
			int newmaxwidth = maxwidth;
			int newmaxheight = maxheight;
			if(global_horizontalforcefit>0.0f)
			{
				newmaxwidth = maxwidth * global_horizontalforcefit;
				if(global_x<(myoutputRECT.left+(maxwidth-newmaxwidth)/2)) global_x = myoutputRECT.left+(maxwidth-newmaxwidth)/2;
				if(global_x>(myoutputRECT.right-(maxwidth-newmaxwidth)/2)) global_x = myoutputRECT.right-(maxwidth-newmaxwidth)/2;
			}
			//if(global_verticalforcefit)
			if(global_verticalforcefit>0.0f)
			{
				newmaxheight = maxheight * global_verticalforcefit;
				if(global_y<(myoutputRECT.top+(maxheight-newmaxheight)/2)) global_y = myoutputRECT.top+(maxheight-newmaxheight)/2;
				if(global_y>(myoutputRECT.bottom-(maxheight-newmaxheight)/2)) global_y = myoutputRECT.bottom-(maxheight-newmaxheight)/2;
			}
			//2020sept23, spi, end

			//regle de 3
			//global_fontheight -> mySIZE.cx/global_spitextstring.length()
			//newfontheight -> maxwidth/global_spitextstring.length()
			//int newfontheight = maxwidth * global_fontheight / mySIZE.cx;
			int newfontheight = newmaxwidth * global_fontheight / mySIZE.cx;
			//if( (global_verticalforcefit>0.0f) && newfontheight>maxheight) newfontheight = maxheight;
			//if( (global_verticalforcefit>0.0f) && newfontheight>maxheight) newfontheight = maxheight*global_verticalforcefit;
			if( (global_verticalforcefit>0.0f) && newfontheight>newmaxheight) newfontheight = maxheight;
			//create new font using newfontheight
			HFONT newhfont = CreateFontA(newfontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,PROOF_QUALITY,0,global_fontface.c_str());
			if(newhfont)
			{
				//deselect current font
				SelectObject(myHDC, prevHGDIOBJ);
				//delete current font
				DeleteObject(global_hFont);
				//select new font
				global_hFont = newhfont;
				global_fontheight = newfontheight;
				prevHGDIOBJ = SelectObject(myHDC, global_hFont);
				//recompute new string extent size
				GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
				//if enabled, justify once again
				if(!global_horizontaljustification.empty() || !global_verticaljustification.empty())
				{
					justification(mySIZE);
				}
			}
			else
			{
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, create font of new size %d did not succeed\n", newfontheight);
					fprintf(pFILE, "warning, force fit may not be successful\n");
				}
			}
		}

		if(pFILE) 
		{
			fprintf(pFILE, "\n");
			if((global_horizontalforcefit>0.0f)) fprintf(pFILE, "after horizontal force fit\n");
			if((global_verticalforcefit>0.0f)) fprintf(pFILE, "after vertical force fit\n");
			fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n",global_x, global_y, mySIZE.cx, mySIZE.cy);
			fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
		}
	}

	if(pFILE) 
	{
		fprintf(pFILE, "\ndisplaying string %s at:\n", global_spitextstring.c_str());
		fprintf(pFILE, "\nglobal_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n",global_x, global_y, mySIZE.cx, mySIZE.cy);
		fflush(pFILE);
	}
	//2020oct05, spi, begin
	if(global_pausedisabled)
	{
		//no app icon is shown in windows task bar (that is what we want when no keydown responses are required)
		SetWindowPos(hWnd, NULL, global_x, global_y, mySIZE.cx, mySIZE.cy, SWP_NOZORDER); 
	}
	else
	{
		//kicking out , SWP_NOZORDER, to experiment, app icon is now shown in windows task bar (that is what we want when keydown responses are required)
		SetWindowPos(hWnd, NULL, global_x, global_y, mySIZE.cx, mySIZE.cy, SWP_SHOWWINDOW); 
	}
	//2020oct05, spi, end
	SelectObject(myHDC, prevHGDIOBJ);
	ReleaseDC(hWnd, myHDC);
	/*
	HRGN GGG = CreateRectRgn(0, 0, 1920, 1200);
	InvertRgn(GetDC(hWnd), GGG);
	SetWindowRgn(hWnd, GGG, false);
	*/
	//COLORREF RRR = RGB(255, 0, 255);
	//COLORREF global_keyingcolor = RGB(255, 0, 255);

	//2020sept22, spi, begin
	//SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)0, LWA_COLORKEY);
	//SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)global_alpha, LWA_COLORKEY);
	SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)global_alpha, LWA_COLORKEY|LWA_ALPHA);
	if(pFILE)
	{
		fprintf(pFILE, "\n\nSetLayeredWindowAttributes() with global_alpha %d\n", global_alpha);
	}
	//2020sept22, spi, end
	/*
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);
	*/
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	/*
	DeleteObject(GGG);
	*/

	//SetTimer(hWnd, 1, 1000, NULL);

	//moved into InitInstance()
	if( (global_textmode>-1) ) //|| ( (global_starttime_sec>=0)||(global_endtime_sec>=0) ) )
	{
		//when in counter modes
		//set timer to update display or to keep track of start time and kill time
		SetTimer(hWnd, ID_TIMER_EVERYSECOND, 1000, NULL); //every seconds timer
	}
	if( (global_textmode<0) && (global_starttime_sec>0) )
	{
		//todo, launch a timer to wait for global_starttime_sec before display;
		//set timer to start to display
		SetTimer(hWnd, ID_TIMER_ONCESTART, global_starttime_sec*1000, NULL); 
		if(pFILE)
		{
			fprintf(pFILE, "SetTimer() ID_TIMER_ONCESTART with global_starttime_sec %d\n", global_starttime_sec);
		}
	}
	if( (global_textmode<0) && (global_endtime_sec>0) && ((global_endtime_sec-global_starttime_sec)>0) )
	{
		//todo, launch a timer to wait for global_timetodisplay_sec before killing app;
		//set timer to kill display
		//SetTimer(hWnd, ID_TIMER_ONCEKILL, (global_endtime_sec-global_starttime_sec)*1000, NULL); 
		SetTimer(hWnd, ID_TIMER_ONCEKILL, global_endtime_sec*1000, NULL); 
		if(pFILE)
		{
			fprintf(pFILE, "SetTimer() ID_TIMER_ONCEKILL with global_endtime_sec %d\n", global_endtime_sec);
		}
	}

	return TRUE;
}

void DrawTextXOR(HDC hdc, const char* charbuffer, int charbufferlength)
{

		//spi, begin
		HDC myMemHDC = CreateCompatibleDC(hdc);
		HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
		SIZE mySIZE2;
		GetTextExtentPoint32A(myMemHDC, charbuffer, charbufferlength, &mySIZE2);

		HBITMAP myHBITMAP = CreateCompatibleBitmap(hdc, mySIZE2.cx, mySIZE2.cy);
		HGDIOBJ prevHBITMAP = SelectObject(myMemHDC, myHBITMAP);
		//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0xFF, 0xFF, 0xFF));
		//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0xFF));
		//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0x00));
		//COLORREF crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0xFF, 0xFF)); //not visible
		COLORREF crOldTextColor_memhdc;
		if(global_idfontcolor==0)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0x00, 0xFF)); //white
		}
		else if(global_idfontcolor==1)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0xFF, 0xFF)); //blue
		}
		else if(global_idfontcolor==2)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0xFF, 0x00)); //red
		}
		else if(global_idfontcolor==3)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0xFF, 0x00)); //black
		}
		else if(global_idfontcolor==4)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0x00, 0x00)); //yellow
		}
		else if(global_idfontcolor==5)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xA0, 0x00, 0x20)); //green lime
		}
		else if(global_idfontcolor==6)
		{
			crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0x00, 0x00)); //green
		}

		//int nOldDrawingMode_memhdc = SetROP2(myMemHDC, R2_NOTXORPEN); //XOR mode, always have to erase what's drawn.
		//int iOldBkMode_memhdc = SetBkMode(myMemHDC, TRANSPARENT);
		//HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
		//TextOutA(myMemHDC, 1, 1, "test string", 11);
		TextOutA(myMemHDC, 0, 0, charbuffer, charbufferlength);
		strcpy(charbuffer_prev, charbuffer);
		//Rectangle(myMemHDC, 0, 0, 1000, 800);
		//BitBlt(hdc, 0, 0, 1000, 800, myMemHDC, 0, 0, SRCCOPY); 
		BitBlt(hdc, 0, 0, mySIZE2.cx, mySIZE2.cy, myMemHDC, 0, 0, 0x00990066); //XOR mode, always have to erase what's drawn.
		//BitBlt(hdc, global_x, global_y, mySIZE2.cx, mySIZE2.cy, myMemHDC, 0, 0, 0x00990066); //XOR mode, always have to erase what's drawn.
		SelectObject(myMemHDC, prevHBITMAP);
		DeleteDC(myMemHDC);
		DeleteObject(myHBITMAP);
		//DeleteDC(myMemHDC2);
		
		//spi, end
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
    case WM_ERASEBKGND:
		{
			
			RECT rect;
			GetClientRect(hWnd, &rect);
			//FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(0, 0, 0)));
			//FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(255, 0, 255))); //keying color
			FillRect((HDC)wParam, &rect, CreateSolidBrush(global_keyingcolor)); //keying color
			
		}
		break;
		
	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER_EVERYSECOND:
		case ID_TIMER_ONCESTART:
		case ID_TIMER_ONCEKILL:

			///////////////////
			//update time stamp
			///////////////////
			global_nowstamp_ms = GetTickCount();

			////////////////
			//for all modes
			///////////////
			//1) check for start condition
			if(wParam==ID_TIMER_ONCESTART)
			{
				//InvalidateRect(hWnd, NULL, FALSE);
				KillTimer(hWnd, ID_TIMER_ONCESTART);
			}
			//2) check for end condition
			else if(wParam==ID_TIMER_ONCEKILL)
			{
				int nShowCmd = false;
				//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
				//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
				if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
				KillTimer(hWnd, ID_TIMER_ONCEKILL);
				PostMessage(hWnd, WM_DESTROY, 0, 0);
			}

			////////////////////////
			//text string mode only
			///////////////////////
			if(global_textmode<0 && ((wParam==ID_TIMER_ONCESTART)||(wParam==ID_TIMER_ONCEKILL)) )
			{
				//nothing to do in here evrery second
				//only redraw when start and kill
				InvalidateRect(hWnd, NULL, FALSE);
			}

			////////////////////
			//counter modes only
			////////////////////
			//2020oct05, spi, begin
			/*
			if(global_textmode==global_countermodeCLOCK)
			{
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if(global_textmode==global_countermodeCOUNTUP)
			*/
			if((global_textmode==global_countermodeCOUNTUP) || (global_textmode==global_countermodeCLOCK))
			//2020oct05, spi, end
			{
				//1) calculate time to display
				if(global_starttime_sec<0) global_starttime_sec=0;
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				//2020oct05, spi, begin
				if(global_dontcounttimeonpause)
				{
					int currenttimeonpause_ms = global_pausestartstamp_ms; 
					if(currenttimeonpause_ms) currenttimeonpause_ms = (global_nowstamp_ms - currenttimeonpause_ms);
					int totaltimeonpause_ms = global_pausetimetotal_ms + currenttimeonpause_ms;
					elapsed_sec -= (totaltimeonpause_ms/1000); //todo: rounding reverif
				}
				//2020oct05, spi, end
				global_timetodisplay_sec = global_starttime_sec + elapsed_sec;
				InvalidateRect(hWnd, NULL, FALSE);

				//2) check for end condition
				//2020oct05, spi, begin
				//was ending 1 sec too early
				//if(global_endtime_sec>-1 && ((global_endtime_sec-global_starttime_sec)-elapsed_sec)<1)
				if(global_endtime_sec>-1 && ((global_endtime_sec-global_starttime_sec)-elapsed_sec)<0)
				//2020oct05, spi, end
				{
					int nShowCmd = false;
					//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
					//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
					if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
					KillTimer(hWnd, ID_TIMER_ONCEKILL);
					PostMessage(hWnd, WM_DESTROY, 0, 0);
				}
			}
			else if(global_textmode==global_countermodeCOUNTDOWN)
			{
				//calculate time to display
				if(global_starttime_sec<0) global_starttime_sec=0;
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				//2020oct05, spi, begin
				if(global_dontcounttimeonpause)
				{
					int currenttimeonpause_ms = global_pausestartstamp_ms; 
					if(currenttimeonpause_ms) currenttimeonpause_ms = (global_nowstamp_ms - currenttimeonpause_ms);
					int totaltimeonpause_ms = global_pausetimetotal_ms + currenttimeonpause_ms;
					elapsed_sec -= (totaltimeonpause_ms/1000); //todo: rounding reverif
				}
				//2020oct05, spi, end
				global_timetodisplay_sec = global_starttime_sec - elapsed_sec;
				InvalidateRect(hWnd, NULL, FALSE);

				//2) check for end condition
				//2020oct05, spi, begin
				//was ending 1 sec too early
				//if(global_endtime_sec>-1 && ((global_starttime_sec-global_endtime_sec)-elapsed_sec)<1)
				if(global_endtime_sec>-1 && ((global_starttime_sec-global_endtime_sec)-elapsed_sec)<0)
				//2020oct05, spi, end
				{
					int nShowCmd = false;
					//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
					//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
					if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
					KillTimer(hWnd, ID_TIMER_ONCEKILL);
					PostMessage(hWnd, WM_DESTROY, 0, 0);
				}
			}
			else
			{
				assert(false);
			}
			return 0;
		}
		break;
	case WM_KEYDOWN:
		if (wParam == 0x50) //P key
		{
			if(global_pauserecording==false) 
			{
				global_pauserecording=true;
				global_pausestartstamp_ms  = GetTickCount(); //timestamp pause mode start
			}
			else 
			{
				global_pauserecording=false;
				global_pausetimetotal_ms += (GetTickCount()-global_pausestartstamp_ms);
				global_pausestartstamp_ms  = 0; //reset
			}
		}
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			//SaveDC(hdc);
			//int nOldDrawingMode = SetROP2(hdc, R2_NOTXORPEN); //XOR mode, always have to erase what's drawn.

			//int iOldBkMode = SetBkMode(hdc, TRANSPARENT);
			COLORREF crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
			HGDIOBJ hOldFont=(HFONT)SelectObject(hdc,global_hFont);
			
			
			if(global_textmode<0)
			{
				global_nowstamp_ms = GetTickCount();
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				//2020oct05, spi, begin
				if(global_dontcounttimeonpause)
				{
					int currenttimeonpause_ms = global_pausestartstamp_ms; 
					if(currenttimeonpause_ms) currenttimeonpause_ms = (global_nowstamp_ms - currenttimeonpause_ms);
					int totaltimeonpause_ms = global_pausetimetotal_ms + currenttimeonpause_ms;
					elapsed_sec -= (totaltimeonpause_ms/1000); //todo: rounding reverif
				}
				//2020oct05, spi, end
				if(global_starttime_sec>0 && (global_starttime_sec-elapsed_sec)>0)
				{
					//string text mode
					//sprintf(charbuffer, "%s", "delayed"); //empty display
					sprintf(charbuffer, "%s", ""); //empty display
				}
				else
				{
					//string text mode
					sprintf(charbuffer, "%s", global_spitextstring.c_str());
				}
			}
			else
			{
				SYSTEMTIME st;
				//GetSystemTime(&st);
				GetLocalTime(&st);
				int hh = st.wHour;
				int mm = st.wMinute;
				int ss = st.wSecond;
				if(global_textmode==global_countermodeCLOCK)
				{
					//sprintf(charbuffer, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
					//sprintf(charbuffer, "%02d.%02d:%02d", st.wHour, st.wMinute, st.wSecond);
					//sprintf(charbuffer, mytimeformat.c_str(), st.wHour, st.wMinute, st.wSecond);
				}
				else if(global_textmode==global_countermodeCOUNTUP)
				{
					hh = global_timetodisplay_sec / 3600;
					mm = (global_timetodisplay_sec % 3600) / 60;
					ss = global_timetodisplay_sec % 60;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				else if(global_textmode==global_countermodeCOUNTDOWN)
				{
					hh = global_timetodisplay_sec / 3600;
					mm = (global_timetodisplay_sec % 3600) / 60;
					ss = global_timetodisplay_sec % 60;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
					
				}
				else
				{
					//ERROR
					assert(false);
					hh=99;
					mm=99;
					ss=99;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				string mytimeformat = "%02d.%02d:%02d";
				if(global_textformat==global_counterformatHHMMSS)
				{
					mytimeformat = "%02d.%02d:%02d";
					sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				else if(global_textformat==global_counterformatHHMM)
				{
					mytimeformat = "%02d.%02d";
					sprintf(charbuffer, mytimeformat.c_str(), hh, mm);
				}
				else if(global_textformat==global_counterformatMMSS)
				{
					mytimeformat = "%02d:%02d";
					sprintf(charbuffer, mytimeformat.c_str(), mm, ss);
				}
				else if(global_textformat==global_counterformatHH ||
						global_textformat==global_counterformatMM ||
						global_textformat==global_counterformatSS)
				{
					mytimeformat = "%02d";
					if(global_textformat==global_counterformatHH) sprintf(charbuffer, mytimeformat.c_str(), hh);
					else if(global_textformat==global_counterformatMM) sprintf(charbuffer, mytimeformat.c_str(), mm);
					else if(global_textformat==global_counterformatSS) sprintf(charbuffer, mytimeformat.c_str(), ss);
				}

			}

			//TextOutA(hdc, 50, 50, charbuffer, charbufferlength);
			int charbufferlength = strlen(charbuffer);
			if(strcmp(charbuffer_prev, "")) DrawTextXOR(hdc, charbuffer_prev, strlen(charbuffer));
			DrawTextXOR(hdc, charbuffer, charbufferlength);
			
			SetTextColor(hdc, crOldTextColor);
			//SetBkMode(hdc, iOldBkMode);
			SelectObject(hdc,hOldFont);

			//SetROP2(hdc, nOldDrawingMode); 
			//RestoreDC(hdc, -1);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		DeleteObject(global_hFont);
		KillTimer(hWnd, ID_TIMER_EVERYSECOND);
		KillTimer(hWnd, ID_TIMER_ONCESTART);
		KillTimer(hWnd, ID_TIMER_ONCEKILL);
		if(!global_end.empty()) ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
		Terminate(pFILE);
		if(mySPIAudioDevice.pFILE) fclose(mySPIAudioDevice.pFILE); //devices.txt
		if(pFILE) fclose(pFILE); //debug.txt
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
