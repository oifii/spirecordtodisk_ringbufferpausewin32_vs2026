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
	//2020sept13, spi, begin
	RECT rcCombined;
	//2020sept13, spi, end
	std::vector<HMONITOR>   hMonitors;
	std::map<HMONITOR, RECT> hmapMonitors;
	//std::map<RECT, HMONITOR> rcmapMonitors;

	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
		pThis->rcMonitors.push_back(*lprcMonitor);
		//2020sept13, spi, begin
		UnionRect(&pThis->rcCombined, &pThis->rcCombined, lprcMonitor);
		//2020sept13, spi, end
		pThis->hMonitors.push_back(hMon);
		pThis->hmapMonitors.insert(std::pair<HMONITOR, RECT>(hMon, *lprcMonitor));
		//pThis->rcmapMonitors.insert(std::pair<RECT, HMONITOR>(*lprcMonitor, hMon));
		return TRUE;
	}

	MonitorRects()
	{
		//2020sept13, spi, begin
		SetRectEmpty(&rcCombined);
		//2020sept13, spi, end
		EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
	}
};

//original struct EnumWindowsStruct version as used in cwindowmultimonitor.h/.cpp
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

//alternate struct EnumWindowsStruct version as used in spitext.h/.cpp
struct EnumWindowsStruct_spitext
{
	std::map<HWND, wstring> hwndstringmap_class;
	wstring classnameprefix;
	std::map<HWND, wstring> hwndstringmap_title;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct_spitext* pThis = reinterpret_cast<EnumWindowsStruct_spitext*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		WCHAR windowtitle[1024];
		GetWindowText(hwnd, windowtitle, sizeof(windowtitle));
		if(pThis->classnameprefix.empty() || pThis->classnameprefix.c_str()==L"noclassnamespecified")
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		else if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		return TRUE;
	}

	EnumWindowsStruct_spitext()
	{
		ifstream myfile("classname.txt");
		string line;
		if (myfile.is_open())
		{
			getline(myfile, line);
			myfile.close();
			if(!line.empty()) classnameprefix = utf8_decode(line);
		}
		else
		{
			classnameprefix = L"noclassnamespecified";
		}


		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap_class.clear();
		hwndstringmap_title.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};

#endif
