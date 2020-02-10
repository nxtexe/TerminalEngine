#include "TerminalApp.h"
#include "MathHelper.h"
#include<array>

using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex {
	XMFLOAT3 Pos;
};

struct ObjectConstants {
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4X4();
};

class MainApp : public TerminalApp {
	public:
		MainApp();
		MainApp(const MainApp& rhs) = delete;
		MainApp& operator=(const MainApp& rhs) = delete;
		~MainApp();
		virtual void CreateSwapChain();
		virtual bool Initialize() override;
		void OnResize();
	private:
		virtual void Draw(const GameTimer& gt) override;
		virtual void Update(const GameTimer& gt) override;
	private:
		XMFLOAT4X4 mWorld = MathHelper::Identity4X4();
		XMFLOAT4X4 mView = MathHelper::Identity4X4();
		XMFLOAT4X4 mProj = MathHelper::Identity4X4();
		float mTheta = 1.5f * XM_PI;
		float mPhi = XM_PIDIV4;
		float mRadius = 5.0f;
		void BuildBoxGeometry();
		POINT mLastMousePos;
};

int main() {
	MainApp app;
	if (!app.Initialize())
		return 0;

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == nullptr)
		return 0;
	
	std::thread events_thread(&MainApp::HandleEvents, &app, &hStdin);
	events_thread.join();
	return app.Run();
}

void MainApp::OnResize() {
	//get number of character cells in the current buffer
	if (!GetConsoleScreenBufferInfo(mhMainWnd, &csbi))
		return;
	//the window resized, so we update the aspect ratio and recompute the projection matrix

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

bool MainApp::Initialize() {
	if (!TerminalApp::Initialize())
		return false;
}

void MainApp::BuildBoxGeometry() {
	std::array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f) })
	};

	std::array<std::uint16_t, 36> indices = {
		//front face
		0, 1, 2,
		0, 2, 3,

		//back face
		4, 6, 5,
		4, 7, 6,

		//left face
		4, 5, 1,
		4, 1, 0,

		//right face
		3, 2, 6,
		3, 6, 7,

		//top face
		1, 5, 6,
		1, 6, 2,

		//bottom face
		4, 0, 3,
		4, 3, 7
	};
}
void MainApp::Draw(const GameTimer& gt) {
	mhMainWnd = swapChainBuffer[mCurrFrontBuffer];
	if (mCurrFrontBuffer < mSwapChainCount - 1) {
		mCurrFrontBuffer++;
	}
	else {
		mCurrFrontBuffer = 0;
	}
	static COORD coords;
	if (coords.X < mClientWidth && coords.Y < mClientHeight) {
		coords.X += 1;
		coords.Y += 1;
	}
	else {
		coords.X = 0;
		coords.Y = 0;
	}

	static DWORD num = 0;
	const static WORD clearAttribs = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	static WORD newAttribs = FOREGROUND_GREEN;

	{
		/*
		COORD cords;
		cords.X = 0;
		cords.Y = 0;
		DWORD num = 0;
		//WORD wAttribs = BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
		WORD wAttribs = FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_GREEN;
		WORD clearAttribs = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
		GetConsoleScreenBufferInfo(swapChainBuffer[mCurrFrontBuffer], &bufferInfo);
		DWORD screenSize = bufferInfo.dwMaximumWindowSize.X * bufferInfo.dwMaximumWindowSize.Y;
		SetConsoleTextAttribute(swapChainBuffer[mCurrFrontBuffer], clearAttribs);
		FillConsoleOutputAttribute(
			mhMainWnd,
			screenSize,
			mClientWidth * mClientHeight,
			cords,
			&num
		);
		/*for (int i = 0; i < 10; i++) {
			cords.X = i;
			for (int j = 0; j < 10; j++) {
				cords.Y = j;
				FillConsoleOutputAttribute(
					swapChainBuffer[mCurrFrontBuffer],
					100,
					100,
					cords,
					&num
				);
			}
		}*/
		/*FillConsoleOutputAttribute(
			swapChainBuffer[mCurrFrontBuffer],
			screenSize,
			mClientWidth*mClientHeight,
			cords,
			&num
		);
		WriteConsoleOutputCharacter(
			swapChainBuffer[mCurrFrontBuffer],
			L"Nathan",
			6,
			cords,
			&num
		);*/
	}

	WriteConsoleOutputCharacter(
		swapChainBuffer[mCurrFrontBuffer],
		L"*",
		1,
		char_coords,
		&num
	);
	SetConsoleActiveScreenBuffer(swapChainBuffer[mCurrFrontBuffer]);
	clear();
}

void MainApp::Update(const GameTimer& gt) {
	//convert spherical to cartesian coordinates

	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float y = mRadius * sinf(mPhi) * sinf(mTheta);
	float z = mRadius * cosf(mPhi);

	//buil the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	//update the constant buffer with the latest worldViewProj matrix
	//here {} rendering pipeline needed

}

void MainApp::CreateSwapChain() {
	swapChainBuffer.push_back(mhMainWnd);
	for (int i = 1; i < mSwapChainCount; i++) {
		HANDLE buffer = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
		swapChainBuffer.push_back(buffer);
	}
}

MainApp::MainApp() {

}

MainApp::~MainApp() {

}