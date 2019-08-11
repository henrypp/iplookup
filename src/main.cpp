// Ip Lookup
// Copyright (c) 2011-2019 Henry++

#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

#include "main.hpp"
#include "rapp.hpp"
#include "routine.hpp"

#include "resource.hpp"

rapp app (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT);

R_FASTLOCK lock;

UINT WINAPI _app_print (LPVOID lparam)
{
	const HWND hwnd = (HWND)lparam;

	_r_fastlock_acquireshared (&lock);

	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	WSADATA wsa = {0};

	if (WSAStartup (WINSOCK_VERSION, &wsa) == ERROR_SUCCESS)
	{
		PIP_ADAPTER_ADDRESSES adapter_addresses = nullptr;
		PIP_ADAPTER_ADDRESSES adapter = nullptr;

		ULONG size = 0;

		rstring buffer;

		while (true)
		{
			size += _R_BUFFER_LENGTH;

			adapter_addresses = new IP_ADAPTER_ADDRESSES[size];

			const DWORD error = GetAdaptersAddresses (AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME, nullptr, adapter_addresses, &size);

			if (error == ERROR_SUCCESS)
			{
				break;
			}
			else
			{
				SAFE_DELETE_ARRAY (adapter_addresses);

				if (error == ERROR_BUFFER_OVERFLOW)
					continue;

				break;
			}
		}

		if (adapter_addresses)
		{
			for (adapter = adapter_addresses; adapter != nullptr; adapter = adapter->Next)
			{
				if (IF_TYPE_SOFTWARE_LOOPBACK == adapter->IfType)
				{
					continue;
				}

				for (IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress; address != nullptr; address = address->Next)
				{
					auto family = address->Address.lpSockaddr->sa_family;

					if (family == AF_INET)
					{
						// ipv4
						PSOCKADDR_IN ipv4 = (PSOCKADDR_IN)(address->Address.lpSockaddr);

						InetNtop (AF_INET, &(ipv4->sin_addr), buffer.GetBuffer (INET_ADDRSTRLEN), INET_ADDRSTRLEN);
						buffer.ReleaseBuffer ();

						_r_listview_additem (hwnd, IDC_LISTVIEW, INVALID_INT, 0, buffer, INVALID_INT, 0);
					}
					else if (family == AF_INET6)
					{
						// ipv6
						PSOCKADDR_IN6 ipv6 = (PSOCKADDR_IN6)(address->Address.lpSockaddr);

						InetNtop (AF_INET6, &(ipv6->sin6_addr), buffer.GetBuffer (INET6_ADDRSTRLEN), INET6_ADDRSTRLEN);
						buffer.ReleaseBuffer ();

						_r_listview_additem (hwnd, IDC_LISTVIEW, INVALID_INT, 0, buffer, INVALID_INT, 1);
					}
					else
					{
						continue;
					}
				}
			}

			SAFE_DELETE_ARRAY (adapter_addresses);
		}

		WSACleanup ();
	}

	if (app.ConfigGet (L"GetExternalIp", false).AsBool ())
	{
		rstring bufferw;

		if (app.DownloadURL (app.ConfigGet (L"ExternalUrl", EXTERNAL_URL), &bufferw, false, nullptr, 0))
			_r_listview_additem (hwnd, IDC_LISTVIEW, INVALID_INT, 0, bufferw, INVALID_INT, 2);
	}

	_r_status_settext (hwnd, IDC_STATUSBAR, 0, _r_fmt (app.LocaleString (IDS_STATUS, nullptr), _r_listview_getitemcount (hwnd, IDC_LISTVIEW)));

	_r_fastlock_releaseshared (&lock);

	return ERROR_SUCCESS;
}

void ResizeWindow (HWND hwnd, INT width, INT height)
{
	RECT rc = {0};
	GetClientRect (GetDlgItem (hwnd, IDC_STATUSBAR), &rc);

	const INT statusbar_height = _R_RECT_HEIGHT (&rc);

	_r_wnd_resize (nullptr, GetDlgItem (hwnd, IDC_LISTVIEW), nullptr, 0, 0, width, height - statusbar_height, 0);

	GetClientRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);
	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, nullptr, _R_RECT_WIDTH (&rc));

	SendDlgItemMessage (hwnd, IDC_STATUSBAR, WM_SIZE, 0, 0);
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
#ifndef _APP_NO_DARKTHEME
			_r_wnd_setdarktheme (hwnd);
#endif // _APP_NO_DARKTHEME

			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, nullptr, -95, LVCFMT_LEFT);

			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 0, L"", 0, 0);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 1, L"", 0, 0);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 2, L"", 0, 0);

			break;
		}

		case RM_INITIALIZE:
		{
			// configure menu
			CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (app.ConfigGet (L"AlwaysOnTop", false).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (app.ConfigGet (L"CheckUpdates", true).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_GETEXTERNALIP_CHK, MF_BYCOMMAND | (app.ConfigGet (L"GetExternalIp", false).AsBool () ? MF_CHECKED : MF_UNCHECKED));

			break;
		}

		case RM_LOCALIZE:
		{
			// configure listview

			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 0, L"IPv4", 0, 0);
			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 1, L"IPv6", 0, 0);
			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 2, app.LocaleString (IDS_GROUP2, nullptr), 0, 0);

			// localize
			const HMENU hmenu = GetMenu (hwnd);

			app.LocaleMenu (hmenu, IDS_FILE, 0, true, nullptr);
			app.LocaleMenu (hmenu, IDS_EXIT, IDM_EXIT, false, L"\tEsc");
			app.LocaleMenu (hmenu, IDS_SETTINGS, 1, true, nullptr);
			app.LocaleMenu (hmenu, IDS_ALWAYSONTOP_CHK, IDM_ALWAYSONTOP_CHK, false, nullptr);
			app.LocaleMenu (hmenu, IDS_CHECKUPDATES_CHK, IDM_CHECKUPDATES_CHK, false, nullptr);
			app.LocaleMenu (GetSubMenu (hmenu, 1), IDS_LANGUAGE, 5, true, L" (Language)");
			app.LocaleMenu (hmenu, IDS_GETEXTERNALIP_CHK, IDM_GETEXTERNALIP_CHK, false, nullptr);
			app.LocaleMenu (hmenu, IDS_HELP, 2, true, nullptr);
			app.LocaleMenu (hmenu, IDS_WEBSITE, IDM_WEBSITE, false, nullptr);
			app.LocaleMenu (hmenu, IDS_CHECKUPDATES, IDM_CHECKUPDATES, false, nullptr);
			app.LocaleMenu (hmenu, IDS_ABOUT, IDM_ABOUT, false, nullptr);

			// refresh list
			SendMessage (hwnd, WM_COMMAND, MAKEWPARAM (IDM_REFRESH, 0), 0);

			app.LocaleEnum ((HWND)GetSubMenu (hmenu, 1), 5, true, IDX_LANGUAGE); // enum localizations

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage (0);
			break;
		}

		case WM_SIZE:
		{
			ResizeWindow (hwnd, LOWORD (lparam), HIWORD (lparam));
			RedrawWindow (hwnd, nullptr, nullptr, RDW_NOFRAME | RDW_NOINTERNALPAINT | RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);

			break;
		}

		case WM_CONTEXTMENU:
		{
			if (GetDlgCtrlID ((HWND)wparam) == IDC_LISTVIEW)
			{
				const HMENU hmenu = LoadMenu (nullptr, MAKEINTRESOURCE (IDM_LISTVIEW));
				const HMENU hsubmenu = GetSubMenu (hmenu, 0);

				// localize
				app.LocaleMenu (hsubmenu, IDS_REFRESH, IDM_REFRESH, false, L"\tF5");
				app.LocaleMenu (hsubmenu, IDS_COPY, IDM_COPY, false, L"\tCtrl+C");

				if (_r_fastlock_islocked (&lock))
					EnableMenuItem (hsubmenu, IDM_REFRESH, MF_BYCOMMAND | MF_DISABLED);

				if (!SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETSELECTEDCOUNT, 0, 0))
					EnableMenuItem (hsubmenu, IDM_COPY, MF_BYCOMMAND | MF_DISABLED);

				POINT pt = {0};
				GetCursorPos (&pt);

				TrackPopupMenuEx (hsubmenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, hwnd, nullptr);

				DestroyMenu (hmenu);
			}

			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD (wparam) == 0 && LOWORD (wparam) >= IDX_LANGUAGE && LOWORD (wparam) <= IDX_LANGUAGE + app.LocaleGetCount ())
			{
				app.LocaleApplyFromMenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), 5), LOWORD (wparam), IDX_LANGUAGE);

				return FALSE;
			}

			switch (LOWORD (wparam))
			{
				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_ALWAYSONTOP_CHK:
				{
					const bool new_val = !app.ConfigGet (L"AlwaysOnTop", false).AsBool ();

					CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"AlwaysOnTop", new_val);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					const bool new_val = !app.ConfigGet (L"CheckUpdates", true).AsBool ();

					CheckMenuItem (GetMenu (hwnd), IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"CheckUpdates", new_val);

					break;
				}

				case IDM_GETEXTERNALIP_CHK:
				{
					const bool new_val = !app.ConfigGet (L"GetExternalIp", false).AsBool ();

					CheckMenuItem (GetMenu (hwnd), IDM_GETEXTERNALIP_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"GetExternalIp", new_val);

					SendMessage (hwnd, WM_COMMAND, MAKEWPARAM (IDM_REFRESH, 0), 0);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, nullptr, _APP_WEBSITE_URL, nullptr, nullptr, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					app.UpdateCheck (true);
					break;
				}

				case IDM_ABOUT:
				{
					app.CreateAboutWindow (hwnd);
					break;
				}

				case IDM_REFRESH:
				{
					_r_createthread (&_app_print, hwnd, false);
					break;
				}

				case IDM_COPY:
				{
					rstring buffer;

					INT item = INVALID_INT;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != INVALID_INT)
					{
						buffer.Append (_r_listview_getitemtext (hwnd, IDC_LISTVIEW, item, 0));
						buffer.Append (L"\r\n");
					}

					if (!buffer.IsEmpty ())
					{
						buffer.Trim (L"\r\n");

						_r_clipboard_set (hwnd, buffer, buffer.GetLength ());
					}

					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain (HINSTANCE, HINSTANCE, LPWSTR, INT)
{
	MSG msg = {0};

	if (app.CreateMainWindow (IDD_MAIN, IDI_MAIN, &DlgProc))
	{
		const HACCEL haccel = LoadAccelerators (app.GetHINSTANCE (), MAKEINTRESOURCE (IDA_MAIN));

		if (haccel)
		{
			while (GetMessage (&msg, nullptr, 0, 0) > 0)
			{
				TranslateAccelerator (app.GetHWND (), haccel, &msg);

				if (!IsDialogMessage (app.GetHWND (), &msg))
				{
					TranslateMessage (&msg);
					DispatchMessage (&msg);
				}
			}

			DestroyAcceleratorTable (haccel);
		}
	}

	return (INT)msg.wParam;
}
