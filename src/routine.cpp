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

#include "routine.h"

// Set Extended Style (ListView)
void Lv_SetStyleEx(HWND hWnd, INT iDlgItem, DWORD dwExStyle, BOOL bExplorerStyle)
{
	if(bExplorerStyle)
		SetWindowTheme(GetDlgItem(hWnd, iDlgItem), L"explorer", 0);

	SendDlgItemMessage(hWnd, iDlgItem, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);
}

// Get Item Text (ListView)
CString Lv_GetItemText(HWND hWnd, INT iDlgItem, INT cchTextMax, INT iItem, INT iSubItem)
{
	LVITEM lvi = {0};
	CString buffer;

	lvi.iSubItem = iSubItem;
	lvi.pszText = buffer.GetBuffer(cchTextMax);
	lvi.cchTextMax = cchTextMax;

	SendDlgItemMessage(hWnd, iDlgItem, LVM_GETITEMTEXT, iItem, (LPARAM)&lvi);
	buffer.ReleaseBuffer();

	return buffer;
}

// Insert Group (ListView)
void Lv_InsertGroup(HWND hWnd, INT iDlgItem, CString lpszText, INT iGroupId, UINT uAlign, UINT uState)
{
	LVGROUP lvg = {0};

	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_GROUPID | LVGF_HEADER;
	lvg.pszHeader = lpszText.GetBuffer();
	lvg.cchHeader = lpszText.GetLength();
	lvg.iGroupId = iGroupId;
	
	if(uAlign)
	{
		lvg.mask |= LVGF_ALIGN;
		lvg.uAlign = uAlign;
	}

	if(uState)
	{
		lvg.mask |= LVGF_STATE;
		lvg.state = uState;
	}

	SendDlgItemMessage(hWnd, iDlgItem, LVM_INSERTGROUP, iGroupId, (LPARAM)&lvg);
}

// Insert Column (ListView)
void Lv_InsertColumn(HWND hWnd, INT iDlgItem, CString lpszText, INT iWidth, INT iItem, INT iFmt)
{
	LVCOLUMN lvc = {0};

	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	lvc.fmt = iFmt;
	lvc.pszText = lpszText.GetBuffer();
	lvc.cchTextMax = lpszText.GetLength();
	lvc.cx = iWidth;
	lvc.iSubItem = iItem;

	SendDlgItemMessage(hWnd, iDlgItem, LVM_INSERTCOLUMN, iItem, (LPARAM)&lvc);
}

// Insert Item (ListView)
void Lv_InsertItem(HWND hWnd, INT iDlgItem, CString lpszText, INT iItem, INT iSubItem, INT iImage, INT iGroupId, LPARAM lParam)
{
	LVITEM lvi = {0}, edit = {0};
	
	lvi.mask = LVIF_TEXT;
	lvi.pszText = lpszText.GetBuffer();
	lvi.cchTextMax = lpszText.GetLength();
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;

	if(lParam != -1 && iSubItem)
	{
		edit.mask = LVIF_PARAM;
		edit.iItem = iItem;
		edit.iSubItem = 0;
		edit.lParam = lParam;

		SendDlgItemMessage(hWnd, iDlgItem, LVM_SETITEM, 0, (LPARAM)&edit);
	}

	if(iImage != -1)
	{
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iImage;
	}

	if(iGroupId != -1)
	{
		lvi.mask |= LVIF_GROUPID;
		lvi.iGroupId = iGroupId;
	}

	SendDlgItemMessage(hWnd, iDlgItem, (iSubItem) ? LVM_SETITEM : LVM_INSERTITEM, 0, (LPARAM)&lvi);
}

// Number Format
CString number_format(LONGLONG lNumber, LPCWSTR lpszAppend, CONST WCHAR cSeparator)
{
	CString buffer;
	BOOL bNegative = lNumber < 0;

    if(!lNumber)
    {
		buffer = L"0";
    }
	else
	{
		do
		{
			if((buffer.GetLength() + 1) % 4 == 0)
				buffer.AppendChar(cSeparator);
 
			int iMod = lNumber % 10;
 
			if(lNumber < 0)
				iMod = -iMod;
 
			buffer.AppendChar(iMod + L'0');
		}
		while(lNumber /= 10);
 
		if(bNegative)
			buffer.AppendChar(L'-');

		buffer.MakeReverse();
	}

	if(lpszAppend)
		buffer.Append(lpszAppend);

	buffer.AppendChar(L'\0');

	return buffer;
}

// Show Balloon Tip for Edit Control
BOOL ShowEditBalloonTip(HWND hWnd, INT iDlgItem, LPCTSTR lpcszTitle, LPCTSTR lpcszText, INT iIcon)
{
	EDITBALLOONTIP ebt = {0};

	ebt.cbStruct = sizeof(ebt);
	ebt.pszTitle = lpcszTitle;
	ebt.pszText = lpcszText;
	ebt.ttiIcon = iIcon;

	return (BOOL)SendDlgItemMessage(hWnd, iDlgItem, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
}

// Check User Is Admin
BOOL IsAdmin()
{
    HANDLE hToken = 0;

	BYTE buffer[512];
	PTOKEN_GROUPS pGroups = (PTOKEN_GROUPS)buffer;
	DWORD dwSize = 0;

	PSID pAdminSid = 0;
	SID_IDENTIFIER_AUTHORITY siaNtAuth = SECURITY_NT_AUTHORITY;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return 0;

    BOOL bSuccess = GetTokenInformation(hToken, TokenGroups, (LPVOID)pGroups, 512, &dwSize);
    CloseHandle(hToken);

    if(!bSuccess)
        return 0;

    if(!AllocateAndInitializeSid(&siaNtAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSid))
        return 0;

    bSuccess = 0;

    for(DWORD i = 0; (i < pGroups->GroupCount) && !bSuccess; i++)
    {
        if(EqualSid(pAdminSid, pGroups->Groups[i].Sid))
            bSuccess = 1;
    }

    FreeSid(pAdminSid);
    
    return bSuccess;
}

// Check is File Exists
BOOL FileExists(LPCTSTR lpcszPath)
{
	return (GetFileAttributes(lpcszPath) != INVALID_FILE_ATTRIBUTES);
}

// Privilege Enabler
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpcszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp = {0};
	LUID luid = {0};

    if(!LookupPrivilegeValue(NULL, lpcszPrivilege, &luid))
        return 0; 

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

    if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
          return 0; 

    if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
          return 0;

    return 1;
}

// Validate Windows Version
BOOL ValidWindowsVersion(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi = {0};
	DWORDLONG dwMask = 0;

	// Initialize Structure
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;

	// Initialize Condition Mask
	VER_SET_CONDITION(dwMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwMask, VER_MINORVERSION, VER_GREATER_EQUAL);

	// Return Result
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwMask);
}

// Set Window Always on Top
VOID SetAlwaysOnTop(HWND hWnd, BOOL bEnable)
{
	SetWindowPos(hWnd, (bEnable ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// Centering Window by Parent
VOID CenterDialog(HWND hWnd)
{
     HWND hParent = GetParent(hWnd);
	 RECT rcChild = {0}, rcParent = {0};

	 // If Parent Doesn't Exists or Invisible - Use Desktop
	 if(!hParent || !IsWindowVisible(hParent))
		 hParent = GetDesktopWindow();

    GetWindowRect(hWnd, &rcChild);
    GetWindowRect(hParent, &rcParent);
 
    int nWidth = rcChild.right - rcChild.left, nHeight = rcChild.bottom - rcChild.top;
    int nX = ((rcParent.right - rcParent.left) - nWidth) / 2 + rcParent.left, nY = ((rcParent.bottom - rcParent.top) - nHeight) / 2 + rcParent.top;
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN), nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    if(nX < 0) nX = 0;
    if(nY < 0) nY = 0;
    if(nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
    if(nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;
 
    MoveWindow(hWnd, nX, nY, nWidth, nHeight, 0);
}

// Toggle Visiblity for Window
VOID ToggleVisible(HWND hWnd)
{
	if(IsWindowVisible(hWnd))
	{
		ShowWindow(hWnd, SW_HIDE);
	}
	else
	{
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
	}
}

// Create Autorun Entry in Registry
VOID CreateAutorunEntry(LPCWSTR lpszName, BOOL bRemove)
{
	HKEY hKey = 0;
	WCHAR szBuffer[MAX_PATH] = {0};

	if(RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	LONG lErrCode = RegQueryValueEx(hKey, lpszName, 0, 0, 0, 0);

	if(bRemove)
	{
		if(lErrCode != ERROR_FILE_NOT_FOUND)
			RegDeleteValue(hKey, lpszName);
	}
	else
	{
		if(lErrCode == ERROR_FILE_NOT_FOUND)
		{
			GetModuleFileName(0, szBuffer, MAX_PATH);
			PathQuoteSpaces(szBuffer);

			RegSetValueEx(hKey, lpszName, 0, REG_SZ, (LPBYTE)szBuffer, MAX_PATH);
		}
	}

	RegCloseKey(hKey);
}

// String Localizer
CString ls(HINSTANCE hInstance, UINT uID)
{
	CString buffer;

	if(!buffer.LoadString(hInstance, uID) && hInstance)
		buffer.LoadString(0, uID);

	if(buffer.IsEmpty())
		buffer.Format(L"%d", uID);

	return buffer;
}

// Get Clipboard Text
CString ClipboardGet()
{
	CString buffer;

	if(OpenClipboard(NULL))
	{
		HGLOBAL hGlobal = GetClipboardData(CF_UNICODETEXT);

		if(hGlobal)
		{
			buffer = (LPWSTR)GlobalLock(hGlobal);

			if(!buffer.IsEmpty())
				GlobalUnlock(hGlobal);
		}
	}

	CloseClipboard();

	return buffer;
}

// Insert Text Into Clipboard
BOOL ClipboardPut(CString buffer)
{
	if(OpenClipboard(NULL))
	{
		if(EmptyClipboard())
		{
			HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (buffer.GetLength() + 1) * sizeof(TCHAR));

			if(hGlobal) 
			{
				LPWSTR pBuffer = (LPWSTR)GlobalLock(hGlobal);

				if(pBuffer)
				{
					StringCchCopy(pBuffer, buffer.GetLength() + 1, buffer);
					SetClipboardData(CF_UNICODETEXT, hGlobal);

					GlobalUnlock(hGlobal);
				}
				else
				{
					GlobalFree(hGlobal);
					hGlobal = NULL;
				}
			}
		}

		CloseClipboard();
	}

	return 1;
}

// Convert UnixTime to Windows SYSTEMTIME
BOOL UnixTimeToSystemTime(time_t t, SYSTEMTIME* pst)
{
	LONGLONG ll = 0; // 64 bit value 
	FILETIME ft = {0};

	ll = Int32x32To64(t, 10000000) + 116444736000000000ui64;

	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = (DWORD)(ll >> 32); 

	return FileTimeToSystemTime(&ft, pst);
}

// Convert Windows SYSTEMTIME to UnixTime
time_t SystemTimeToUnixTime(SYSTEMTIME* pst)
{
	LONGLONG ll = 0; // 64 bit value 
	FILETIME ft = {0};

	SystemTimeToFileTime(pst, &ft);

	return (((((LONGLONG)(ft.dwHighDateTime)) << 32) + ft.dwLowDateTime) - 116444736000000000ui64) / 10000000ui64;
}

// MessageBox with prinf routine
INT MessageBox(HWND hWnd, UINT uType, LPCWSTR lpcszCaption, LPCWSTR lpcszFormat, ...)
{
	va_list pArgList;
	va_start(pArgList, lpcszFormat);

	CString buffer;
	buffer.FormatV(lpcszFormat, pArgList);

	va_end(pArgList);

	return MessageBox(hWnd, buffer, lpcszCaption, uType);
}

// Create Tooltip and set to Control
HWND SetDlgItemTooltip(HWND hWnd, INT iDlgItem, LPWSTR lpszText)
{
	// Create Tooltips
	HWND hTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetModuleHandle(0), NULL);
                   
	TOOLINFO ti = {0};

	ti.cbSize = sizeof(ti);
	ti.hwnd = hWnd;
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.lpszText = lpszText;
	ti.uId = (UINT_PTR)GetDlgItem(hWnd, iDlgItem);

	SendMessage(hTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return hTip;
}


// WM_MUTEX Wrapper
INT WmMutexWrapper(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	if(wParam && (GetCurrentProcessId() != wParam))
	{
		if(!lParam)
		{
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			return 1;
		}
		else
		{
			ShowWindow(hwndDlg, SW_SHOW);
			SetForegroundWindow(hwndDlg);

			return 1;
		}
	}

	return 0;
}

// About Dialog Callback
LRESULT CALLBACK AboutBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
		{
			if(GetParent(hwndDlg))
				EnableWindow(GetParent(hwndDlg), 0);

			CenterDialog(hwndDlg);

			break;
		}
		
		case WM_ENTERSIZEMOVE:
		case WM_EXITSIZEMOVE:
		{
			LONG lExStyle = GetWindowLong(hwndDlg, GWL_EXSTYLE);

			if(!(lExStyle & WS_EX_LAYERED))
				SetWindowLong(hwndDlg, GWL_EXSTYLE, lExStyle | WS_EX_LAYERED);

			SetLayeredWindowAttributes(hwndDlg, 0, 100, uMsg == WM_ENTERSIZEMOVE ? LWA_ALPHA : 0);
			SetCursor(LoadCursor(0, uMsg == WM_ENTERSIZEMOVE ? IDC_SIZEALL : IDC_ARROW));

			break;
		}

		case WM_LBUTTONDBLCLK:
		case WM_CLOSE:
		{
			DestroyWindow(hwndDlg);
			break;
		}

		case WM_DESTROY:
		{
			if(GetParent(hwndDlg))
			{
				EnableWindow(GetParent(hwndDlg), 1);
				SetActiveWindow(GetParent(hwndDlg));
			}

			PostQuitMessage(0);

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps = {0};
			RECT rc = {0};

			HDC hDC = BeginPaint(hwndDlg, &ps);

			GetClientRect(hwndDlg, &rc);
			rc.top = rc.bottom - 43;

			// Instead FillRect
			COLORREF clrOld = SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
			ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			SetBkColor(hDC, clrOld);

			// Draw Line
			for(int i = 0; i < rc.right; i++)
				SetPixel(hDC, i, rc.top, GetSysColor(COLOR_BTNSHADOW));

			EndPaint(hwndDlg, &ps);

			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
			break;
		}

		case WM_CTLCOLORSTATIC:
		{
			return (LPARAM)GetSysColorBrush(COLOR_WINDOW);
		}

		case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
				case NM_RETURN:
				{
					NMLINK* nmlink = (NMLINK*)lParam;

					if(lstrlen(nmlink->item.szUrl))
						ShellExecute(hwndDlg, 0, nmlink->item.szUrl, 0, 0, SW_SHOW);
	
					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			if(LOWORD(wParam) == 100 || LOWORD(wParam) == IDCANCEL)
				DestroyWindow(hwndDlg);

			break;
		}
	}

	return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

// About Dialog
INT AboutBoxCreate(HWND hParent, LPWSTR lpszIcon, LPCWSTR lpcszTitle, LPCWSTR lpszAppName, LPCWSTR lpcszCopyright)
{
	MSG msg = {0};
	WNDCLASSEX wcex = {0};
	HINSTANCE hInstance = GetModuleHandle(0);

	if(!GetClassInfoEx(hInstance, L"AboutBox", &wcex))
	{
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.lpfnWndProc = (WNDPROC)AboutBoxProc;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(0, IDC_ARROW);
		wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
		wcex.lpszClassName = L"AboutBox";

		if(!RegisterClassEx(&wcex))
			return 0;
	}

	HWND hDlg = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"AboutBox", lpcszTitle, WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 353, 270, hParent, 0, hInstance, 0);

	if(!hDlg)
		return 0;

	// Get System Font
	LOGFONT lf = {0};
	GetObject((HFONT)SendMessage(hParent, WM_GETFONT, 0, 0), sizeof(lf), &lf);

	// Create Normal Font
	HFONT hFont = CreateFontIndirect(&lf);

	// Create Bold Font
	lf.lfWeight = FW_BOLD;
	HFONT hTitle = CreateFontIndirect(&lf);

	HWND hWnd = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | SS_ICON, 13, 13, 32, 32, hDlg, 0, hInstance, 0);
	SendMessage(hWnd, STM_SETICON, (WPARAM)LoadIcon(hInstance, lpszIcon), 0);
	
	hWnd = CreateWindowEx(0, WC_STATIC, lpszAppName, WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT | SS_CENTERIMAGE, 56, 13, 280, 32, hDlg, 0, hInstance, 0);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hTitle, 0);

	hWnd = CreateWindowEx(0, WC_LINK, lpcszCopyright, WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT, 56, 50, 280, 140, hDlg, 0, hInstance, 0);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);

	RECT rc = {0};
	GetClientRect(hDlg, &rc);

	hWnd = CreateWindowEx(0, WC_BUTTON, L"OK", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 270, rc.bottom - 33, 70, 23, hDlg, 0, hInstance, 0);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
	SetWindowLong(hWnd, GWL_ID, 100);

	while(GetMessage(&msg, 0, 0, 0))
	{
		if(!IsDialogMessage(hDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DeleteObject(hFont);
	DeleteObject(hTitle);

	return msg.wParam;
}