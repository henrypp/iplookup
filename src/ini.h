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

#ifndef __INI_H__
#define __INI_H__

#include <windows.h>
#include <atlstr.h> // cstring

class INI
{
	protected:
		CString ini_path, buffer;

		BOOL make_write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPCWSTR lpcszValue);
		DWORD make_read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPWSTR lpcszReturned, DWORD dwSize, LPCWSTR lpcszDefault);

	public:
		// Load File
		void load(LPCWSTR lpszPath);

		// Get Ini Path
		CString get_ini_path();

		// Write Int
		BOOL write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, DWORD dwValue);

		// Write String
		BOOL write(LPCWSTR lpcszSection, LPCWSTR lpcszKey, LPCWSTR lpcszValue);

		// Read Int
		UINT read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, INT iDefault);

		// Read String
		CString read(LPCWSTR lpcszSection, LPCWSTR lpcszKey, DWORD dwLenght, LPCWSTR lpcszDefault);
};

#endif // __INI_H__