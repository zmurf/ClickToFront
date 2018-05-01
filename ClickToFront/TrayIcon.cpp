#include "stdafx.h"
#include "TrayIcon.h"
#include "resource.h"
#include "Strsafe.h"
#include "Commctrl.h"

#define TRAYICONMESSAGE (WM_APP+100)

namespace
{

	HMENU sContextMenu = nullptr;
	HMENU sMenu = nullptr;
}

TrayIcon::TrayIcon()
	: mNotificationIconData({})
	, mLpszClassName(L"__hidden__")
	, mInstance(GetModuleHandle(nullptr))
{
	initTrayIcon();
}

TrayIcon::~TrayIcon()
{
	DestroyMenu(sMenu);
	DestroyMenu(sContextMenu);
	Shell_NotifyIcon(NIM_DELETE, &mNotificationIconData);
	UnregisterClass(mLpszClassName, mInstance);
	mInstance = nullptr;
	mLpszClassName = nullptr;

	delete(mInstance);
	delete(mLpszClassName);
}

void TrayIcon::initTrayIcon()
{
	WNDCLASS wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = nullptr;
	wndClass.hCursor = nullptr;
	wndClass.hIcon = nullptr;
	wndClass.hInstance = mInstance;
	wndClass.lpfnWndProc = TrayIcon::windowProcess;
	wndClass.lpszClassName = mLpszClassName;
	wndClass.lpszMenuName = nullptr;
	wndClass.style = 0;
	RegisterClass(&wndClass);
		
	HWND windowHwnd = CreateWindow(mLpszClassName, mLpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, mInstance, nullptr);

	initMenu(windowHwnd);

	mNotificationIconData.hWnd = windowHwnd;
	mNotificationIconData.uVersion = NOTIFYICON_VERSION_4;
	mNotificationIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
	StringCchCopy(mNotificationIconData.szInfo, ARRAYSIZE(mNotificationIconData.szInfo), L"Click To Front");
	StringCchCopy(mNotificationIconData.szTip, ARRAYSIZE(mNotificationIconData.szTip), L"Click To Front is running");
	LoadIconMetric(mInstance, MAKEINTRESOURCE(IDI_CLICKTOFRONTICON), LIM_SMALL, &(mNotificationIconData.hIcon));
	mNotificationIconData.uCallbackMessage = TRAYICONMESSAGE;
	Shell_NotifyIcon(NIM_ADD, &mNotificationIconData) ? S_OK : E_FAIL;

	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void TrayIcon::initMenu(HWND hwnd)
{
	sContextMenu = CreateMenu();
	sMenu = CreateMenu();

	AppendMenu(sContextMenu, MF_POPUP, (UINT_PTR)sMenu, L"Menu");
	AppendMenu(sMenu, MF_STRING, menuChoice::showHide, L"Show/Hide");
	AppendMenu(sMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenu(sMenu, MF_STRING, menuChoice::quit, L"Quit");

	SetMenu(hwnd, sContextMenu);
}

INT_PTR CALLBACK TrayIcon::windowProcess(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case TRAYICONMESSAGE:
	{
		switch (lParam)
		{
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			TrayIcon::showContextMenu(hwnd);
			break;
		}
		default:
		{
			break;
		}
		}
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case menuChoice::showHide:
		{
			if (IsWindowVisible(GetConsoleWindow()))
			{
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			}
			else
			{
				ShowWindow(GetConsoleWindow(), SW_SHOW);
			}
			break;
		}
		case menuChoice::quit:
		{
			PostQuitMessage(WM_QUIT);
		}
		default:
		{
			break;
		}
		}
	}
	default:
	{
		break;
	}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);;
}

void TrayIcon::showContextMenu(HWND hwnd)
{
	POINT mousePosition;
	if (GetCursorPos(&mousePosition))
	{
		SetForegroundWindow(hwnd);
		TrackPopupMenu(sMenu, NULL, mousePosition.x, mousePosition.y, 0, hwnd, nullptr);
	}
}