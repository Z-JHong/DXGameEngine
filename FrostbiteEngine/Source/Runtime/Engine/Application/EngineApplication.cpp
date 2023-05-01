#include "EngineApplication.h"
#include "Runtime/Render/DirectXRHI.h"

FrostbiteEngineApplication* FrostbiteEngineApplication::EngineApplication = nullptr;

FrostbiteEngineApplication::FrostbiteEngineApplication()
{
	EngineRenderRHI = new DirectXRHI();
}

bool FrostbiteEngineApplication::InitEngineInstance(HINSTANCE IN_hInstance)
{
	//初始化引擎实例：
	this->EngineInstance = IN_hInstance;

	bool bCreateEngineWindowsSuccessful = this->CreateEngineInstanceWindow();

	if (bCreateEngineWindowsSuccessful)
	{
		EngineRenderRHI->InitEngineRHI(this->EngineInstanceMainWnd);
	}

	return true;
}

FrostbiteEngineApplication* FrostbiteEngineApplication::GetEngineApplicationInstance()
{
	if (EngineApplication != nullptr)
	{
		return EngineApplication;
	}

	return new FrostbiteEngineApplication();
}

int FrostbiteEngineApplication::EngineInstanceTick()
{
	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (this->EngineRenderRHI != nullptr)
			{
				this->EngineRenderRHI->UpdateEngineRHI();
				this->EngineRenderRHI->DrawEngineRHI();
			}
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK EngineApplicationProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool FrostbiteEngineApplication::CreateEngineInstanceWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = EngineApplicationProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = EngineInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"FrostbiteEngineInstance";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = {0, 0, 1280, 720};

	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	EngineInstanceMainWnd = CreateWindow(L"FrostbiteEngineInstance", L"FrostbiteEngine Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, EngineInstance, 0);

	if (!EngineInstanceMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(EngineInstanceMainWnd, SW_SHOW);
	UpdateWindow(EngineInstanceMainWnd);

	return true;
}
