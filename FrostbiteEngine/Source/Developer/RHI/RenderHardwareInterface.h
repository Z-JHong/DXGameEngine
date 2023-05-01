#pragma once
#include <windows.h>

//RHI:RenderHardwareInterface

class RenderHardwareInterface
{

public:

	virtual bool InitEngineRHI(HWND IN_HWnd ) = 0;

	virtual bool UpdateEngineRHI() = 0;

	virtual bool DrawEngineRHI() = 0;
};

