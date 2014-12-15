/************************************
*  	Ip Lookup
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

// Include
#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <atlstr.h> // cstring
#include <process.h> // _beginthreadex

#include "iplookup.h"
#include "routine.h"
#include "resource.h"
#include "ini.h"

INI ini;
CONFIG cfg = {0};
CONST UINT WM_MUTEX = RegisterWindowMessage(APP_NAME_SHORT);

// Check Updates
UINT WINAPI CheckUpdates(LPVOID lpParam)
{
	BOOL bStatus = 0;
	HINTERNET hInternet = 0, hConnect = 0;

	// Disable Menu
	EnableMenuItem(GetMenu(cfg.hWnd), IDM_CHECK_UPDATES, MF_BYCOMMAND | MF_DISABLED);

	// Connect
	if((hInternet = InternetOpen(APP_USERAGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) && (hConnect = InternetOpenUrl(hInternet, APP_WEBSITE L"/update.php?product=" APP_NAME_SHORT, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0)))
	{
		// Get Status
		DWORD dwStatus = 0, dwStatusSize = sizeof(dwStatus);
		HttpQueryInfo(hConnect, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL);

		// Check Errors
		if(dwStatus == HTTP_STATUS_OK)
		{
			// Reading
			ULONG ulReaded = 0;
			CHAR szBufferA[MAX_PATH] = {0};

			if(InternetReadFile(hConnect, szBufferA, MAX_PATH, &ulReaded) && ulReaded)
			{
				// Convert to Unicode
				CA2W newver(szBufferA, CP_UTF8);

				// If NEWVER == CURVER
				if(lstrcmpi(newver, APP_VERSION) == 0)
				{
					if(!lpParam)
						MessageBox(cfg.hWnd, MB_OK | MB_ICONINFORMATION, APP_NAME, ls(cfg.hLocale, IDS_UPDATE_NO));
				}
				else
				{
					if(MessageBox(cfg.hWnd, MB_YESNO | MB_ICONQUESTION, APP_NAME, ls(cfg.hLocale, IDS_UPDATE_YES), newver) == IDYES)
						ShellExecute(cfg.hWnd, 0, APP_WEBSITE L"/?product=" APP_NAME_SHORT, NULL, NULL, SW_SHOWDEFAULT);
				}

				// Switch Result
				bStatus = 1;
			}
		}
	}

	if(!bStatus && !lpParam)
		MessageBox(cfg.hWnd, MB_OK | MB_ICONSTOP, APP_NAME, ls(cfg.hLocale, IDS_UPDATE_ERROR));

	// Restore Menu
	EnableMenuItem(GetMenu(cfg.hWnd), IDM_CHECK_UPDATES, MF_BYCOMMAND | MF_ENABLED);

	// Clear Memory
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return bStatus;
}

// Print IP list to EditBox, include external IP
UINT WINAPI GetIPList(LPVOID lpParam)
{
	CString buffer = ls(cfg.hLocale, IDS_IP_LOCAL) + L"\r\n";

	// Disable Button
	EnableWindow(GetDlgItem(cfg.hWnd, IDC_REFRESH), 0);

	// Show Indicator
	SetDlgItemText(cfg.hWnd, IDC_IP_LIST, ls(cfg.hLocale, IDS_IP_LOADING));

	// Get Local IP
	struct hostent* sh = NULL;
	CHAR chInfo[64] = {0};

    if(!gethostname(chInfo, sizeof(chInfo)) && (sh = gethostbyname((char*)&chInfo)))
    {
		INT i = 0;

        while(sh->h_addr_list[i])
        {
			sockaddr_in adr = {0};
            memcpy(&adr.sin_addr, sh->h_addr_list[i++], sh->h_length);

			CA2W unicode(inet_ntoa(adr.sin_addr), CP_UTF8);
			buffer.Append(unicode);
			buffer.Append(L"\r\n");
        }

		buffer.Append(L"\r\n");
    }
	else
	{
		buffer.Append(ls(cfg.hLocale, IDS_IP_ERROR) + L"\r\n\r\n");
	}

	// Get External IP
	if(IsDlgButtonChecked(cfg.hWnd, IDC_EXTERNAL_IP_CHK) == BST_CHECKED)
	{
		HINTERNET hInternet = NULL, hConnect = NULL;
		DWORD dwStatus = 0, dwStatusSize = sizeof(dwStatus);
		ULONG ulReaded = 0;
		CHAR szBuffer[MAX_PATH] = {0};

		if((hInternet = InternetOpen(APP_USERAGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) && (hConnect = InternetOpenUrl(hInternet, L"http://api.externalip.net/ip/", NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0)) && HttpQueryInfo(hConnect, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL) && dwStatus == HTTP_STATUS_OK && InternetReadFile(hConnect, szBuffer, MAX_PATH, &ulReaded))
		{
			CA2W unicode(szBuffer, CP_UTF8);

			buffer.Append(ls(cfg.hLocale, IDS_IP_EXTERNAL) + L"\r\n");
			buffer.Append(unicode);
		}
		else
		{
			buffer.Append(ls(cfg.hLocale, IDS_IP_ERROR) + L"\r\n\r\n");
		}

		// Free Memory
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
	}

	// Print Result
	buffer.TrimRight();
	SetDlgItemText(cfg.hWnd, IDC_IP_LIST, buffer);

	// Restore Button
	EnableWindow(GetDlgItem(cfg.hWnd, IDC_REFRESH), 1);

	return 0;
}

LRESULT CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hMutex = NULL;
	INT iBuffer = 0;
	CString buffer;

	if(uMsg == WM_MUTEX)
		return WmMutexWrapper(hwndDlg, wParam, lParam);

	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			// Check Mutex
			CreateMutex(NULL, TRUE, APP_NAME_SHORT);

			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				PostMessage(HWND_BROADCAST, WM_MUTEX, GetCurrentProcessId(), 1);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);

				return 0;
			}

			// Load Winsock Library
			WSADATA wsa = {0};

			if(WSAStartup(MAKEWORD(2, 2), &wsa))
			{
				MessageBox(hwndDlg, MB_OK | MB_ICONSTOP, APP_NAME, ls(cfg.hLocale, IDS_WINSOCK_ERROR), WSAGetLastError());
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);

				return 0;
			}

			// Config
			cfg.hWnd = hwndDlg;

			// Set Window Title
			SetWindowText(hwndDlg, APP_NAME L" " APP_VERSION);

			// Set Icons
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 32, 32, 0));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0));

			// Modify System Menu
			HMENU hMenu = GetSystemMenu(hwndDlg, 0);
			InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, IDM_ABOUT, ls(cfg.hLocale, IDS_ABOUT));

			// Get External IP Cfg
			CheckDlgButton(hwndDlg, IDC_EXTERNAL_IP_CHK, ini.read(APP_NAME_SHORT, L"GetExternalIp", 0) ? BST_CHECKED : BST_UNCHECKED);

			// Always on Top
			if(ini.read(APP_NAME_SHORT, L"AlwaysOnTop", 1))
			{
				SetAlwaysOnTop(hwndDlg, 1);
				CheckMenuItem(GetMenu(hwndDlg), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | MF_CHECKED);
			}

			// Print IP List
			if(ini.read(APP_NAME_SHORT, L"GetAtStartup", 1))
			{
				SendMessage(hwndDlg, WM_COMMAND, MAKELPARAM(IDC_REFRESH, 0), 0);
				CheckMenuItem(GetMenu(hwndDlg), IDM_GETATSTARTUP_CHK, MF_BYCOMMAND | MF_CHECKED);
			}
			
			// Check Updates
			if(ini.read(APP_NAME_SHORT, L"CheckUpdateAtStartup", 0))
			{
				_beginthreadex(NULL, 0, &CheckUpdates, (LPVOID)1, 0, NULL);
				CheckMenuItem(GetMenu(hwndDlg), IDM_CHECKUPDATEATSTARTUP_CHK, MF_BYCOMMAND | MF_CHECKED);
			}

			break;
		}

		case WM_CLOSE:
		{
			// Save Settings
			ini.write(APP_NAME_SHORT, L"GetExternalIp", (IsDlgButtonChecked(hwndDlg, IDC_EXTERNAL_IP_CHK) == BST_CHECKED));

			// Unload Winsock Library
			WSACleanup();

			// Destroy Window and Quit
			DestroyWindow(hwndDlg);
			PostQuitMessage(0);

			break;
		}

		case WM_PAINT:
		{
			RECT rc = {0};
			PAINTSTRUCT ps = {0};
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

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORDLG:
		{
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		}

		case WM_SYSCOMMAND:
		{
			if(wParam == IDM_ABOUT)
				SendMessage(hwndDlg, WM_COMMAND, MAKELPARAM(IDM_ABOUT, 0), 0);

			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				case IDC_EXIT:
				{
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				
				case IDM_CHECKUPDATEATSTARTUP_CHK:
				{
					iBuffer = ini.read(APP_NAME_SHORT, L"CheckUpdateAtStartup", 0);
					CheckMenuItem(GetMenu(hwndDlg), IDM_CHECKUPDATEATSTARTUP_CHK, MF_BYCOMMAND | (iBuffer) ? MF_UNCHECKED : MF_CHECKED);
					ini.write(APP_NAME_SHORT, L"CheckUpdateAtStartup", iBuffer ? 0 : 1);

					break;
				}
				
				case IDM_ALWAYSONTOP_CHK:
				{
					iBuffer = ini.read(APP_NAME_SHORT, L"AlwaysOnTop", 1);
					CheckMenuItem(GetMenu(hwndDlg), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (iBuffer) ? MF_UNCHECKED : MF_CHECKED);
					ini.write(APP_NAME_SHORT, L"AlwaysOnTop", iBuffer ? 0 : 1);

					SetAlwaysOnTop(hwndDlg, !iBuffer);

					break;
				}

				case IDM_GETATSTARTUP_CHK:
				{
					iBuffer = ini.read(APP_NAME_SHORT, L"GetAtStartup", 1);
					CheckMenuItem(GetMenu(hwndDlg), IDM_GETATSTARTUP_CHK, MF_BYCOMMAND | (iBuffer) ? MF_UNCHECKED : MF_CHECKED);
					ini.write(APP_NAME_SHORT, L"GetAtStartup", iBuffer ? 0 : 1);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute(hwndDlg, 0, APP_WEBSITE, NULL, NULL, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECK_UPDATES:
				{
					_beginthreadex(NULL, 0, &CheckUpdates, 0, 0, NULL);
					break;
				}

				case IDM_ABOUT:
				{
					buffer.Format(ls(cfg.hLocale, IDS_COPYRIGHT), APP_WEBSITE, APP_HOST);
					AboutBoxCreate(hwndDlg, MAKEINTRESOURCE(IDI_MAIN), ls(cfg.hLocale, IDS_ABOUT), APP_NAME L" " APP_VERSION, L"Copyright © 2012 Henry++\r\nAll Rights Reversed\r\n\r\n" + buffer);

					break;
				}

				case IDC_REFRESH:
				{
					_beginthreadex(NULL, 0, &GetIPList, NULL, 0, NULL);
					break;
				}
			}

			break;
		}
	}

	return 0;
}

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, INT nShowCmd)
{
	CString buffer;

	// Load Settings
	GetModuleFileName(0, buffer.GetBuffer(MAX_PATH), MAX_PATH);
	PathRenameExtension(buffer.GetBuffer(MAX_PATH), L".cfg"); buffer.ReleaseBuffer();
	ini.load(buffer);

	// Current Dir
	PathRemoveFileSpec(buffer.GetBuffer(MAX_PATH)); buffer.ReleaseBuffer();
	StringCchCopy(cfg.szCurrentDir, MAX_PATH, buffer);

	// Language
	buffer.Format(L"%s\\Languages\\%s.dll", cfg.szCurrentDir, ini.read(APP_NAME_SHORT, L"Language", MAX_PATH, 0));

	if(FileExists(buffer))
		cfg.hLocale = LoadLibraryEx(buffer, 0, LOAD_LIBRARY_AS_DATAFILE);

	// Initialize and Create Window
	MSG msg = {0};
	INITCOMMONCONTROLSEX icex = {0};

	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;

	if(!InitCommonControlsEx(&icex))
		return 0;

	if(!(cfg.hWnd = CreateDialog(cfg.hLocale, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgProc)))
		return 0;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!IsDialogMessage(cfg.hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	return msg.wParam;
}