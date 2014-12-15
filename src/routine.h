/************************************
*  	Routine Functions
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

// lastmod: 22/11/12

#ifndef __ROUTINE_H__
#define __ROUTINE_H__

#include <windows.h>
#include <uxtheme.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <lm.h>
#include <atlstr.h>
#include <strsafe.h>

// Color Shader
#ifndef COLOR_SHADE
#define COLOR_SHADE(clr, percent) RGB((BYTE)(GetRValue(clr) * percent / 100), (BYTE)(GetGValue(clr) * percent / 100), (BYTE)(GetBValue(clr) * percent / 100))
#endif // COLOR_SHADE

void Lv_SetStyleEx(HWND hWnd, INT iDlgItem, DWORD dwExStyle, BOOL bExplorerStyle);
CString Lv_GetItemText(HWND hWnd, INT iDlgItem, INT cchTextMax, INT iItem, INT iSubItem);
void Lv_InsertGroup(HWND hWnd, INT iDlgItem, CString lpszText, INT iGroupId, UINT uAlign = 0, UINT uState = 0);
void Lv_InsertColumn(HWND hWnd, INT iDlgItem, CString lpszText, INT iWidth, INT iItem, INT iFmt);
void Lv_InsertItem(HWND hWnd, INT iDlgItem, CString lpszText, INT iItem, INT iSubItem, INT iImage = -1, INT iGroupId = -1, LPARAM lParam = -1);

BOOL ShowEditBalloonTip(HWND hWnd, INT iDlgItem, LPCTSTR lpcszTitle, LPCTSTR lpcszText, INT iIcon);
BOOL IsAdmin();
BOOL FileExists(LPCTSTR lpcszPath);
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpcszPrivilege, BOOL bEnablePrivilege);
BOOL ValidWindowsVersion(DWORD dwMajorVersion, DWORD dwMinorVersion);

CString number_format(LONGLONG lNumber, LPCWSTR lpszAppend = 0, CONST WCHAR cSeparator = L',');

VOID SetAlwaysOnTop(HWND hWnd, BOOL bEnable);
VOID CenterDialog(HWND hWnd);
VOID ToggleVisible(HWND hWnd);
VOID CreateAutorunEntry(LPCWSTR lpszName, BOOL bRemove);

CString ls(HINSTANCE hInstance, UINT uID);

CString ClipboardGet();
BOOL ClipboardPut(CString buffer);

BOOL UnixTimeToSystemTime(time_t t, SYSTEMTIME* pst);
time_t SystemTimeToUnixTime(SYSTEMTIME* pst);

INT MessageBox(HWND hWnd, UINT uType, LPCWSTR lpcszCaption, LPCWSTR lpcszFormat, ...);
HWND SetDlgItemTooltip(HWND hWnd, INT iDlgItem, LPWSTR lpszText);
INT WmMutexWrapper(HWND hwndDlg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK AboutBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT AboutBoxCreate(HWND hParent, LPWSTR lpszIcon, LPCWSTR lpcszTitle, LPCWSTR lpszAppName, LPCWSTR lpcszCopyright);

#endif // __ROUTINE_H__
