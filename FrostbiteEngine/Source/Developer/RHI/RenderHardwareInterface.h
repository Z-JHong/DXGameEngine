#pragma once
#include <windows.h>

//RHI:RenderHardwareInterface

class RenderHardwareInterface
{

public:

	virtual void InitEngineRHI(HWND IN_HWnd ) = 0;

	virtual void UpdateEngineRHI() = 0;

	virtual void DrawEngineRHI() = 0;
};

