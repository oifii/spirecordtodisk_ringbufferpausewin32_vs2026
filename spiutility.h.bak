/*
 * Copyright (c) 2015-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
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
#ifndef _SPIUTILITY_H
#define _SPIUTILITY_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;


float RandomFloat(float a, float b);
int RandomInt(int lowest, int highest);


// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);


PCHAR*
CommandLineToArgvA(
PCHAR CmdLine,
int* _argc
);


struct MonitorRects
//class MonitorRects
{
//public:
	std::vector<RECT>   rcMonitors;
	std::vector<HMONITOR>   hMonitors;
	std::map<HMONITOR, RECT> hmapMonitors;
	//std::map<RECT, HMONITOR> rcmapMonitors;

	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
		pThis->rcMonitors.push_back(*lprcMonitor);
		pThis->hMonitors.push_back(hMon);
		pThis->hmapMonitors.insert(std::pair<HMONITOR, RECT>(hMon, *lprcMonitor));
		//pThis->rcmapMonitors.insert(std::pair<RECT, HMONITOR>(*lprcMonitor, hMon));
		return TRUE;
	}

	MonitorRects()
	{
		EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
	}
};

struct EnumWindowsStruct
{
	std::map<HWND, wstring> hwndstringmap;
	wstring classnameprefix;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct* pThis = reinterpret_cast<EnumWindowsStruct*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap.insert(pair<HWND, wstring>(hwnd, classname));
		}
		return TRUE;
	}

	EnumWindowsStruct()
	{
		ifstream myfile("classname.txt");
		string line;
		if (myfile.is_open())
		{
			getline(myfile, line);
			myfile.close();
			classnameprefix = utf8_decode(line);
		}
		else
		{
			classnameprefix = L"noclassnamespecified";
		}


		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};

#endif
