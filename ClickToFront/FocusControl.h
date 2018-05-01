#pragma once

class FocusControl
{
public:
	FocusControl();
	~FocusControl();

private:
	HWINEVENTHOOK mCreateEvent;
	HWINEVENTHOOK mDialogStartEvent;
	HWINEVENTHOOK mDialogEndEvent;
	HWINEVENTHOOK mDestroyEvent;
	HWINEVENTHOOK mWindowFocusEvent;

	void initFocusControl();
	static LRESULT mouseProcess(int nCode, WPARAM wParam, LPARAM lParam);
	static void createEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static void destroyEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static void windowFocusEventProcess(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static LRESULT tabProcess(int nCode, WPARAM wParam, LPARAM lParam);
	static void setWindowTopMost(HWND hwnd);
	static bool doubleClickOnWindowHappened(HWND hwnd);
	static bool altTabHappened(KBDLLHOOKSTRUCT * kbdLlHookStruct);
	static bool windowWasJustCreated(HWND hwnd);
	static bool altTabJustEnded(HWND hwnd);
	static bool windowTitleEmptyOrTaskSwitching(HWND hwnd);
};
