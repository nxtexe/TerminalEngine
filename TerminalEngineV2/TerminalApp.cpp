#include "TerminalApp.h"

using namespace std;
bool TerminalApp::InitMainWindow() {
	assert(mhMainWnd == nullptr);
	mhMainWnd = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
	COORD dwSize;
	SMALL_RECT rect;
	rect.Top = 0;
	rect.Left = 0;
	rect.Bottom = mClientHeight - 1;
	rect.Right = mClientWidth - 1;
	dwSize.X = mClientWidth;
	dwSize.Y = mClientHeight;
	SetConsoleScreenBufferSize(mhMainWnd, dwSize);
	SetConsoleWindowInfo(mhMainWnd, TRUE, &rect);
	DWORD mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(mhMainWnd, mode))
		return false;
	HWND wnd = GetConsoleWindow();
	RECT r;
	GetWindowRect(wnd, &r);
	SetConsoleActiveScreenBuffer(mhMainWnd);
	MoveWindow(wnd, r.left, r.top, mClientWidth + 1000, mClientHeight + 1000, TRUE);
	if (mhMainWnd) {
		return true;
	}

	return false;
}

bool TerminalApp::Initialize() {
	if (!InitMainWindow())
		return false;
	CreateSwapChain();
	//get number of character cells in the current buffer
	if (!GetConsoleScreenBufferInfo(mhMainWnd, &csbi))
		return false;
	return true;
}


void TerminalApp::CalculateFrameStats() {
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f) {
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = mMainWndCaption +
			L"	fps: " + fpsStr + L" " +
			L"	mspf: " + mspfStr;

		SetConsoleTitle(windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

TerminalApp* TerminalApp::mApp = nullptr;
TerminalApp* TerminalApp::Get() {
	return mApp;
}

TerminalApp::~TerminalApp() {

}

TerminalApp::TerminalApp() {
	assert(mApp == nullptr);
	mApp = this;
}

VOID KeyEventProc(KEY_EVENT_RECORD ker, TerminalApp* theApp)
{
	if (ker.uChar.UnicodeChar == 27)
		exit(0);

	if (ker.uChar.UnicodeChar == 97) {
		if (theApp->char_coords.X <= 0)
			theApp->char_coords.X = 0;
		else
			theApp->char_coords.X -= 1;
	}
	else if (ker.uChar.UnicodeChar == 100) {
		if (theApp->char_coords.X >= theApp->mClientWidth - 1)
			theApp->char_coords.X = theApp->mClientWidth - 1;
		else
			theApp->char_coords.X += 1;
	}
	else if (ker.uChar.UnicodeChar == 115) {
		if (theApp->char_coords.Y >= theApp->mClientHeight - 1)
			theApp->char_coords.Y = theApp->mClientHeight - 1;
		else
			theApp->char_coords.Y += 1;
	}
	else if (ker.uChar.UnicodeChar == 119) {
		if (theApp->char_coords.Y <= 0)
			theApp->char_coords.Y = 0;
		else
			theApp->char_coords.Y -= 1;
	}
	cout << "Key event: ";

	if (ker.bKeyDown)
		cout << "key pressed " << ker.uChar.UnicodeChar << endl;
	else cout << "key released " << ker.uChar.UnicodeChar << endl;
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
	cout << "Mouse event: ";

	switch (mer.dwEventFlags)
	{
	case 0:

		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			cout << "left button press \n";
		}
		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
		{
			cout << "right button press \n";
		}
		else
		{
			cout << "button press\n";
		}
		break;
	case DOUBLE_CLICK:
		cout << "double click\n";
		break;
	case MOUSE_HWHEELED:
		cout << "horizontal mouse wheel\n";
		break;
	case MOUSE_MOVED:
		cout << "mouse moved\n";
		break;
	case MOUSE_WHEELED:
		cout << "vertical mouse wheel\n";
		break;
	default:
		cout << "unknown\n";
		break;
	}
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr, TerminalApp* theApp)
{
	if (theApp->mClientHeight == wbsr.dwSize.Y && theApp->mClientWidth == wbsr.dwSize.X)
		return;
	theApp->mAppPaused = true;
	theApp->mTimer.Stop();
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	theApp->mClientWidth = wbsr.dwSize.X;
	theApp->mClientHeight = wbsr.dwSize.Y;
	COORD dwSize;
	SMALL_RECT rect;
	rect.Top = 0;
	rect.Left = 0;
	rect.Bottom = theApp->mClientHeight - 1;
	rect.Right = theApp->mClientWidth - 1;
	dwSize.X = theApp->mClientWidth;
	dwSize.Y = theApp->mClientHeight;

	for (int i = 0; i < theApp->mSwapChainCount; i++) {
		SetConsoleScreenBufferSize(theApp->swapChainBuffer[theApp->mCurrFrontBuffer], dwSize);
		SetConsoleWindowInfo(theApp->swapChainBuffer[theApp->mCurrFrontBuffer], TRUE, &rect);
	}
	DWORD mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(theApp->swapChainBuffer[theApp->mCurrFrontBuffer], mode))
		return;
	HWND wnd = GetConsoleWindow();
	RECT r;
	GetWindowRect(wnd, &r);
	SetConsoleActiveScreenBuffer(theApp->swapChainBuffer[theApp->mCurrFrontBuffer]);
	MoveWindow(wnd, r.left, r.top, theApp->mClientWidth + 1000, theApp->mClientHeight + 1000, TRUE);
	theApp->mAppPaused = false;
	theApp->mTimer.Start();

	std::cout << "Width: " << theApp->mClientWidth << " Height: " << theApp->mClientHeight << std::endl;
}

void TerminalApp::HandleEvents(HANDLE* hStdin) {
	static INPUT_RECORD ir[128];
	DWORD cNumRead = 0;
	while (true) {
		//message loop
		if (!ReadConsoleInput(
				hStdin,
				ir,
				128,
				&cNumRead
			)
		)
			return;
		for (DWORD i = 0; i < cNumRead; i++) {
			switch (ir[i].EventType) {
			case KEY_EVENT:
				KeyEventProc(ir[i].Event.KeyEvent, this);//a- 97, d - 100
				break;
			case MOUSE_EVENT:
				MouseEventProc(ir[i].Event.MouseEvent);
				break;
			case WINDOW_BUFFER_SIZE_EVENT://screen buffer resizing
				ResizeEventProc(ir[i].Event.WindowBufferSizeEvent, this);
				break;
			case FOCUS_EVENT:
			case MENU_EVENT:
				break;
			default:
				cout << "Unknown Event";
				break;
			}
		}
	}
}

float TerminalApp::AspectRatio() const {
	return static_cast<float>(mClientWidth) / mClientHeight;
}

void TerminalApp::clear() {
	static COORD coordScreen = { 0, 0 };
	static DWORD cCharsWritten;
	static DWORD dwConSize;

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	//fill the entire screen with blanks
	if(!FillConsoleOutputCharacter(
			mhMainWnd,
			(TCHAR) ' ',
			dwConSize,
			coordScreen,
			&cCharsWritten
		)
	)	return;

	//get the current text attribute
	if (!GetConsoleScreenBufferInfo(mhMainWnd, &csbi))
		return;

	//set the buffer's attributes accordingly
	if (!FillConsoleOutputAttribute(
			mhMainWnd,
			csbi.wAttributes,
			dwConSize,
			coordScreen,
			&cCharsWritten
		)
	) return;
}

static void do_nothing();

void do_nothing() {

}

int	TerminalApp::Run() {
	mTimer.Reset();

	static DWORD saveOldMode, mode;

	if (!GetConsoleMode(swapChainBuffer[mCurrFrontBuffer], &saveOldMode))
		return 0;

	mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;

	if (!SetConsoleMode(swapChainBuffer[mCurrFrontBuffer], mode))
		return 0;
	while (true) {
		mTimer.Tick();


	//	std::thread events_thread(HandleEvents, ir, cNumRead, std::ref(*this));
	//	HandleEvents(ir, cNumRead, this);
		OnResize();

		if (!mAppPaused) {
			CalculateFrameStats();
			Update(mTimer);
			Draw(mTimer);
		}
		else {
			Sleep(100);
		}
	}
	SetConsoleMode(swapChainBuffer[mCurrFrontBuffer], saveOldMode);
	return 0;
}