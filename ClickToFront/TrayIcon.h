#pragma once

class TrayIcon
{
	enum menuChoice
	{
		showHide = 1,
		quit
	};

public:
	TrayIcon();
	~TrayIcon();

private:
	NOTIFYICONDATA mNotificationIconData;
	LPCWSTR mLpszClassName;
	HINSTANCE mInstance;

	void initTrayIcon();
	void initMenu(HWND hwnd);
	static INT_PTR windowProcess(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void showContextMenu(HWND hwnd);
};

