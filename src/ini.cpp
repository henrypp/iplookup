/************************************
*	Ini Wrapper
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

// lastmod: 13/10/12

#include "ini.h"

BOOL INI::make_write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPCWSTR lpcszValue)
{
	return WritePrivateProfileString(lpcszSection, lpcszKey, lpcszValue, ini_path);
}
		
DWORD INI::make_read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPWSTR lpcszReturned, DWORD dwSize, LPCWSTR lpcszDefault)
{
	return GetPrivateProfileString(lpcszSection, lpcszKey, lpcszDefault, lpcszReturned, dwSize, ini_path);
}

// Load File
void INI::load(LPCWSTR lpcszPath)
{
	ini_path = lpcszPath;
}

// Get Ini Path
CString INI::get_ini_path()
{
	return ini_path;
}

// Write Int
BOOL INI::write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, DWORD dwValue)
{
	buffer.Format(L"%d\0", dwValue);

	return make_write(lpcszSection, lpcszKey, buffer);
}

// Write String
BOOL INI::write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPCWSTR lpcszValue)
{
	return make_write(lpcszSection, lpcszKey, lpcszValue);
}

// Read Int
UINT INI::read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, INT iDefault)
{
	return GetPrivateProfileInt(lpcszSection, lpcszKey, iDefault, ini_path);
}

// Read String
CString INI::read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, DWORD dwLenght, LPCWSTR lpcszDefault)
{
	make_read(lpcszSection, lpcszKey, buffer.GetBuffer(dwLenght), dwLenght, lpcszDefault);
	buffer.ReleaseBuffer();

	return buffer;
}