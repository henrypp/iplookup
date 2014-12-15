/************************************
*  	Ip Lookup
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

#ifndef __IPLOOKUP_H__
#define __IPLOOKUP_H__

// Define
#define APP_NAME L"Ip Lookup"
#define APP_NAME_SHORT L"iplookup"
#define APP_VERSION L"1.3"
#define APP_VERSION_RES 1,3
#define APP_HOST L"www.henrypp.org"
#define APP_WEBSITE L"http://" APP_HOST
#define APP_USERAGENT APP_NAME L"/" APP_VERSION L" (+" APP_WEBSITE L")"

// Settings Structure
struct CONFIG
{
	HWND hWnd; // main window handle
	HINSTANCE hLocale; // language module handle

	WCHAR szCurrentDir[MAX_PATH]; // current directory
};

#endif // __IPLOOKUP_H__
