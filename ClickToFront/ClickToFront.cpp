#include "stdafx.h"
#include "FocusControl.h"
#include "TrayIcon.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment( lib, "Comctl32.lib" )

int main()
{
	// Ignore Ctrl-C, Ctrl-Break, and so forth...
	SetConsoleCtrlHandler(NULL, TRUE);

	// Remove close button
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);

	// Run program stuff...
	FocusControl focusControl;
	TrayIcon trayIcon;

	// Start messagepump
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 0;
}