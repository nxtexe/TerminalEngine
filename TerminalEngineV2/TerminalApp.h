#include "GameTimer.h"
#include<windows.h>
#include<string>
#include<vector>
#include<iostream>
#include<assert.h>
#include<thread>
#include<chrono>
#include<DirectXMath.h>
#include<DirectXPackedVector.h>

class TerminalApp {
	protected:
		TerminalApp();
		TerminalApp(TerminalApp& rhs) = delete;
		TerminalApp& operator=(TerminalApp& rhs) = delete;
		virtual ~TerminalApp();
		virtual void Update(const GameTimer& gt) = 0;
		virtual void Draw(const GameTimer& gt) = 0;
		void CalculateFrameStats();
		bool InitMainWindow();
		HANDLE mhMainWnd = nullptr;
		std::wstring mMainWndCaption = L"TerminalApp ";
		virtual void CreateSwapChain() = 0;
		virtual void OnResize() = 0;
	public:
		int Run();
		virtual bool Initialize() = 0;
		static TerminalApp* Get();
		static TerminalApp* mApp;
		GameTimer mTimer;
		bool mAppPaused = false;
		int mClientWidth = 250;
		int mClientHeight = 40;
		std::vector<HANDLE> swapChainBuffer;
		int mCurrFrontBuffer = 0;
		static const int mSwapChainCount = 2;
		float AspectRatio() const;
		void clear();
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		COORD char_coords = { 0, 0 };
		wchar_t* char_texture = new wchar_t[mClientWidth * mClientHeight];//index i, j is found by [mClientWidth * i + j]
		void HandleEvents(HANDLE* hStdin, INPUT_RECORD* irInBuff, DWORD& cNumRead);
};