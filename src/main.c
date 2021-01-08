// iplookup
// Copyright (c) 2011-2021 Henry++

#include <ws2tcpip.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

#include "routine.h"

#include "main.h"
#include "rapp.h"

#include "resource.h"

R_SPINLOCK lock_thread;

THREAD_API _app_print (PVOID lparam)
{
	HWND hwnd = (HWND)lparam;
	WSADATA wsa = {0};
	WCHAR buffer[128] = {0};
	PIP_ADAPTER_ADDRESSES adapter_addresses = NULL;
	PIP_ADAPTER_ADDRESSES adapter = NULL;
	ULONG size = 0;
	ULONG code;

	_r_spinlock_acquireshared (&lock_thread);

	_r_status_settext (hwnd, IDC_STATUSBAR, 0, L"Loading....");

	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	if (WSAStartup (WINSOCK_VERSION, &wsa) == ERROR_SUCCESS)
	{
		while (TRUE)
		{
			size += 1024;

			adapter_addresses = _r_mem_allocatezero (size);

			code = GetAdaptersAddresses (AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME, NULL, adapter_addresses, &size);

			if (code == ERROR_SUCCESS)
			{
				break;
			}
			else
			{
				SAFE_DELETE_MEMORY (adapter_addresses);

				if (code == ERROR_BUFFER_OVERFLOW)
					continue;

				break;
			}
		}

		if (adapter_addresses)
		{
			for (adapter = adapter_addresses; adapter != NULL; adapter = adapter->Next)
			{
				if (IF_TYPE_SOFTWARE_LOOPBACK == adapter->IfType)
					continue;

				for (IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress; address != NULL; address = address->Next)
				{
					ADDRESS_FAMILY af = address->Address.lpSockaddr->sa_family;

					if (af == AF_INET)
					{
						// ipv4
						PSOCKADDR_IN ipv4 = (PSOCKADDR_IN)address->Address.lpSockaddr;

						InetNtop (af, &(ipv4->sin_addr), buffer, RTL_NUMBER_OF (buffer));

						_r_listview_additemex (hwnd, IDC_LISTVIEW, -1, 0, buffer, I_IMAGENONE, 0, 0);
					}
					else if (af == AF_INET6)
					{
						// ipv6
						PSOCKADDR_IN6 ipv6 = (PSOCKADDR_IN6)address->Address.lpSockaddr;

						InetNtop (af, &(ipv6->sin6_addr), buffer, RTL_NUMBER_OF (buffer));

						_r_listview_additemex (hwnd, IDC_LISTVIEW, -1, 0, buffer, I_IMAGENONE, 1, 0);
					}
				}
			}

			SAFE_DELETE_MEMORY (adapter_addresses);
		}

		WSACleanup ();
	}

	if (_r_config_getboolean (L"GetExternalIp", FALSE))
	{
		LPCWSTR url_string = _r_config_getstring (L"ExternalUrl", EXTERNAL_URL);

		if (url_string)
		{
			R_DOWNLOAD_INFO info;
			HINTERNET hsession;

			hsession = _r_inet_createsession (_r_app_getuseragent ());

			if (hsession)
			{
				_r_inet_initializedownload (&info, NULL);

				if (_r_inet_begindownload (hsession, url_string, &info) == ERROR_SUCCESS)
				{
					_r_listview_additemex (hwnd, IDC_LISTVIEW, -1, 0, _r_obj_getstringorempty (info.string), I_IMAGENONE, 2, 0);

					_r_inet_finaldownload (&info);
				}

				_r_inet_close (hsession);
			}
		}
	}

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -100);

	_r_status_settextformat (hwnd, IDC_STATUSBAR, 0, _r_locale_getstring (IDS_STATUS), _r_listview_getitemcount (hwnd, IDC_LISTVIEW));

	_r_spinlock_releaseshared (&lock_thread);

	return ERROR_SUCCESS;
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, TRUE);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -95, LVCFMT_LEFT);

			UINT state_mask = 0;

			if (_r_sys_isosversiongreaterorequal (WINDOWS_VISTA))
				state_mask = LVGS_COLLAPSIBLE;

			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 0, L"", 0, state_mask, state_mask);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 1, L"", 0, state_mask, state_mask);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 2, L"", 0, state_mask, state_mask);

			_r_spinlock_initialize (&lock_thread);

			// refresh list
			PostMessage (hwnd, WM_COMMAND, MAKEWPARAM (IDM_REFRESH, 0), 0);

			break;
		}

		case RM_INITIALIZE:
		{
			// configure menu
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				CheckMenuItem (hmenu, IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (_r_config_getboolean (L"AlwaysOnTop", FALSE) ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem (hmenu, IDM_CLASSICUI_CHK, MF_BYCOMMAND | (_r_config_getboolean (L"ClassicUI", APP_CLASSICUI) ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem (hmenu, IDM_GETEXTERNALIP_CHK, MF_BYCOMMAND | (_r_config_getboolean (L"GetExternalIp", FALSE) ? MF_CHECKED : MF_UNCHECKED));
			}

			break;
		}

		case RM_LOCALIZE:
		{
			// localize
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_HELP));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, 1), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));

				_r_menu_setitemtextformat (hmenu, IDM_EXIT, FALSE, L"%s\tEsc", _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtextformat (hmenu, IDM_CLASSICUI_CHK, FALSE, L"%s*", _r_locale_getstring (IDS_CLASSICUI_CHK));
				_r_menu_setitemtext (hmenu, IDM_GETEXTERNALIP_CHK, FALSE, _r_locale_getstring (IDS_GETEXTERNALIP_CHK));
				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_ABOUT, FALSE, _r_locale_getstring (IDS_ABOUT));

				_r_locale_enum (GetSubMenu (hmenu, 1), LANG_MENU, IDX_LANGUAGE); // enum localizations
			}

			// configure listview
			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 0, L"IPv4", 0, 0);
			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 1, L"IPv6", 0, 0);
			_r_listview_setgroup (hwnd, IDC_LISTVIEW, 2, _r_locale_getstring (IDS_GROUP2), 0, 0);

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage (0);
			break;
		}

		case WM_CONTEXTMENU:
		{
			if (GetDlgCtrlID ((HWND)wparam) != IDC_LISTVIEW)
				break;

			HMENU hmenu = LoadMenu (NULL, MAKEINTRESOURCE (IDM_LISTVIEW));
			HMENU hsubmenu = GetSubMenu (hmenu, 0);

			// localize
			_r_menu_setitemtextformat (hmenu, IDM_REFRESH, FALSE, L"%s\tF5", _r_locale_getstring (IDS_REFRESH));
			_r_menu_setitemtextformat (hmenu, IDM_COPY, FALSE, L"%s\tCtrl+C", _r_locale_getstring (IDS_COPY));

			if (_r_spinlock_islocked (&lock_thread))
				_r_menu_enableitem (hsubmenu, IDM_REFRESH, MF_BYCOMMAND, FALSE);

			if (!SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETSELECTEDCOUNT, 0, 0))
				_r_menu_enableitem (hsubmenu, IDM_COPY, MF_BYCOMMAND, FALSE);

			_r_menu_popup (hsubmenu, hwnd, NULL, TRUE);

			DestroyMenu (hmenu);

			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD (wparam) == 0 && LOWORD (wparam) >= IDX_LANGUAGE && LOWORD (wparam) <= IDX_LANGUAGE + _r_locale_getcount ())
			{
				_r_locale_applyfrommenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), LANG_MENU), LOWORD (wparam));

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
					BOOLEAN new_val = !_r_config_getboolean (L"AlwaysOnTop", FALSE);

					CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					_r_config_setboolean (L"AlwaysOnTop", new_val);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_CLASSICUI_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"ClassicUI", APP_CLASSICUI);

					CheckMenuItem (GetMenu (hwnd), IDM_CLASSICUI_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					_r_config_setboolean (L"ClassicUI", new_val);

					_r_app_restart (hwnd);

					break;
				}

				case IDM_GETEXTERNALIP_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"GetExternalIp", FALSE);

					CheckMenuItem (GetMenu (hwnd), IDM_GETEXTERNALIP_CHK, MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					_r_config_setboolean (L"GetExternalIp", new_val);

					PostMessage (hwnd, WM_COMMAND, MAKEWPARAM (IDM_REFRESH, 0), 0);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, NULL, _r_app_getwebsite_url (), NULL, NULL, SW_SHOWDEFAULT);
					break;
				}

				case IDM_ABOUT:
				{
					_r_show_aboutmessage (hwnd);
					break;
				}

				case IDM_REFRESH:
				{
					if (!_r_spinlock_islocked (&lock_thread))
						_r_sys_createthread2 (&_app_print, hwnd);

					break;
				}

				case IDM_COPY:
				{
					R_STRINGBUILDER buffer;
					PR_STRING string;

					_r_obj_initializestringbuilder (&buffer);

					INT item = -1;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						string = _r_listview_getitemtext (hwnd, IDC_LISTVIEW, item, 0);

						if (string)
						{
							_r_obj_appendstringbuilderformat (&buffer, L"%s\r\n", string->buffer);
							_r_obj_dereference (string);
						}
					}

					string = _r_obj_finalstringbuilder (&buffer);

					if (!_r_obj_isstringempty (string))
					{
						_r_obj_trimstring (string, L"\r\n");

						_r_clipboard_set (hwnd, string->buffer, _r_obj_getstringlength (string));
					}

					_r_obj_deletestringbuilder (&buffer);

					break;
				}

				case IDM_SELECT_ALL:
				{
					ListView_SetItemState (GetDlgItem (hwnd, IDC_LISTVIEW), -1, LVIS_SELECTED, LVIS_SELECTED);
					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain (_In_ HINSTANCE hinst, _In_opt_ HINSTANCE prev_hinst, _In_ LPWSTR cmdline, _In_ INT show_cmd)
{
	MSG msg;

	if (_r_app_initialize ())
	{
		if (_r_app_createwindow (IDD_MAIN, IDI_MAIN, &DlgProc))
		{
			HACCEL haccel = LoadAccelerators (_r_sys_getimagebase (), MAKEINTRESOURCE (IDA_MAIN));

			if (haccel)
			{
				while (GetMessage (&msg, NULL, 0, 0) > 0)
				{
					HWND hwnd = GetActiveWindow ();

					if (!TranslateAccelerator (hwnd, haccel, &msg) && !IsDialogMessage (hwnd, &msg))
					{
						TranslateMessage (&msg);
						DispatchMessage (&msg);
					}
				}

				DestroyAcceleratorTable (haccel);
			}
		}
	}

	return ERROR_SUCCESS;
}
