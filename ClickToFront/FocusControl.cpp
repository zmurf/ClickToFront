#include "stdafx.h"
#include "FocusControl.h"
#include <vector>

namespace
{
	HHOOK sTabHook = nullptr;
	HHOOK sMouseHook = nullptr;

	DWORD sPreviousTickCount = 0;
	HWND sPreviouslyClickedHwnd = nullptr;
	HWND sLastFocusedHwnd = nullptr;
	HWND sCurrentTopMostHwnd = nullptr;
	std::vector<HWND> sCreatedObjects;
	bool sAltTabHappend = false;
}

FocusControl::FocusControl()
	: mCreateEvent(nullptr)
	, mDialogStartEvent(nullptr)
	, mDialogEndEvent(nullptr)
	, mDestroyEvent(nullptr)
	, mWindowFocusEvent(nullptr)
{
	initFocusControl();
}

FocusControl::~FocusControl()
{
	SetWindowPos(sCurrentTopMostHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	UnhookWindowsHookEx(sTabHook);
	UnhookWinEvent(mWindowFocusEvent);
	UnhookWinEvent(mDialogEndEvent);
	UnhookWinEvent(mDialogStartEvent);
	UnhookWinEvent(mDestroyEvent);
	UnhookWinEvent(mCreateEvent);
	UnhookWindowsHookEx(sMouseHook);

	sTabHook = nullptr;
	mWindowFocusEvent = nullptr;
	mDialogEndEvent = nullptr;
	mDialogStartEvent = nullptr;
	mDestroyEvent = nullptr;
	mCreateEvent = nullptr;
	sMouseHook = nullptr;

	sPreviouslyClickedHwnd = nullptr;
	sLastFocusedHwnd = nullptr;
	sCurrentTopMostHwnd = nullptr;

	delete(sTabHook);
	delete(mWindowFocusEvent);
	delete(mDialogEndEvent);
	delete(mDialogStartEvent);
	delete(mDestroyEvent);
	delete(mCreateEvent);
	delete(sMouseHook);

	delete(sPreviouslyClickedHwnd);
	delete(sLastFocusedHwnd);
	delete(sCurrentTopMostHwnd);

}

void FocusControl::initFocusControl()
{
	HINSTANCE instance = GetModuleHandle(nullptr);
	sMouseHook = SetWindowsHookEx(WH_MOUSE_LL, FocusControl::mouseProcess, instance, 0);
	mCreateEvent = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, nullptr, FocusControl::createEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	mDestroyEvent = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, nullptr, FocusControl::destroyEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	mDialogStartEvent = SetWinEventHook(EVENT_SYSTEM_DIALOGSTART, EVENT_SYSTEM_DIALOGSTART, nullptr, FocusControl::createEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	mDialogEndEvent = SetWinEventHook(EVENT_SYSTEM_DIALOGEND, EVENT_SYSTEM_DIALOGEND, nullptr, FocusControl::destroyEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	mWindowFocusEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, FocusControl::windowFocusEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	sTabHook = SetWindowsHookEx(WH_KEYBOARD_LL, FocusControl::tabProcess, instance, 0);
}

LRESULT CALLBACK FocusControl::mouseProcess(int nCode, WPARAM wParam, LPARAM lParam)
{
	auto mouseHookStruct = (MOUSEHOOKSTRUCT*)lParam;
	if (nullptr == mouseHookStruct)
	{	
		return CallNextHookEx(sMouseHook, nCode, wParam, lParam);
	}

	if (WM_LBUTTONUP == wParam)
	{
		auto hwnd = GetForegroundWindow();

		if (FocusControl::doubleClickOnWindowHappened(hwnd))
		{
			FocusControl::setWindowTopMost(hwnd);
			return CallNextHookEx(sMouseHook, nCode, wParam, lParam);;
		}

		sPreviousTickCount = GetTickCount();
		sPreviouslyClickedHwnd = hwnd;
	}

	return CallNextHookEx(sMouseHook, nCode, wParam, lParam);
}

void CALLBACK FocusControl::createEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	sCreatedObjects.emplace_back(hwnd);
}

void CALLBACK FocusControl::destroyEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (nullptr == sCurrentTopMostHwnd || hwnd != sCurrentTopMostHwnd)
	{
		return;
	}

	FocusControl::setWindowTopMost(sLastFocusedHwnd);
}

void CALLBACK FocusControl::windowFocusEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	WCHAR classNameOut[512];
	GetClassNameW(hwnd, classNameOut, sizeof(classNameOut));
	std::wstring className(classNameOut);

	if (altTabJustEnded(hwnd) || FocusControl::windowWasJustCreated(hwnd) || 0 == className.compare(L"OperationStatusWindow"))
	{
		FocusControl::setWindowTopMost(hwnd);
	}
	
	if (hwnd != sCurrentTopMostHwnd)
	{
		sLastFocusedHwnd = hwnd;
	}
}

LRESULT CALLBACK FocusControl::tabProcess(int nCode, WPARAM wParam, LPARAM lParam)
{
	// Ugly hack! I would have wanted to use "SetWinEventHook(EVENT_SYSTEM_SWITCHSTART, EVENT_SYSTEM_SWITCHEND, nullptr, FocusControl::tabEventProcess, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);"
	// but for some reason I can't get that to work on Windows 10. See my qustion on Stack Overflow: https://stackoverflow.com/questions/49588438/the-system-events-event-system-switchstart-and-event-system-switchend-works-in-w/49597746#49597746
	auto kbdLlHookStruct = (KBDLLHOOKSTRUCT*)lParam;

	switch (nCode)
	{
		case HC_ACTION:
		{
			if (FocusControl::altTabHappened(kbdLlHookStruct))
			{
				sAltTabHappend = true;
			}
		}
	}

	return CallNextHookEx(sTabHook, nCode, wParam, lParam);
}

void FocusControl::setWindowTopMost(HWND hwnd)
{
	sCreatedObjects.clear();
	
	if (nullptr != sCurrentTopMostHwnd)
	{
		SetWindowPos(sCurrentTopMostHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	//Ugly hack! setting WindowPos a second time... cause for some reason some windows does not get set correctly otherwise. :P
	if (nullptr != sCurrentTopMostHwnd)
	{
		SetWindowPos(sCurrentTopMostHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	sCurrentTopMostHwnd = hwnd;

	std::wstring newWindowOnTop;
	newWindowOnTop.resize(GetWindowTextLength(hwnd) + 1, '\0');
	GetWindowText(hwnd, LPWSTR(newWindowOnTop.c_str()), GetWindowTextLength(hwnd) + 1);
	wprintf(L"New window on top: %s\n", newWindowOnTop.c_str());
}

bool FocusControl::doubleClickOnWindowHappened(HWND hwnd)
{
	return GetDoubleClickTime() > GetTickCount() - sPreviousTickCount && sPreviouslyClickedHwnd == hwnd;
}

bool FocusControl::altTabHappened(KBDLLHOOKSTRUCT* kbdLlHookStruct)
{
	return VK_TAB == kbdLlHookStruct->vkCode && LLKHF_ALTDOWN & kbdLlHookStruct->flags;
}

bool FocusControl::windowWasJustCreated(HWND hwnd)
{
	if (windowTitleEmptyOrTaskSwitching(hwnd))
	{
		return false;
	}

	return std::find(sCreatedObjects.begin(), sCreatedObjects.end(), hwnd) != sCreatedObjects.end();
}

bool FocusControl::altTabJustEnded(HWND hwnd)
{
	if (!sAltTabHappend || windowTitleEmptyOrTaskSwitching(hwnd))
	{
		return false;
	}

	sAltTabHappend = false;
	return true;
}

bool FocusControl::windowTitleEmptyOrTaskSwitching(HWND hwnd)
{
	WCHAR windowTitleOut[512];
	GetWindowText(hwnd, windowTitleOut, sizeof(windowTitleOut));
	std::wstring windowTitle(windowTitleOut);

	if (windowTitle.empty() || 0 == windowTitle.compare(L"Task Switching"))
	{
		return true;
	}

	return false;
}