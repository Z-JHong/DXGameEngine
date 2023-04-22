#pragma once

#include "Windows.h"


class FrostbiteEngineApplication
{

private:

	static FrostbiteEngineApplication* EngineApplication;

	FrostbiteEngineApplication();

private:

	HINSTANCE EngineInstance;
	HWND      EngineInstanceMainWnd = nullptr; 

public:

	virtual bool InitEngineInstance(HINSTANCE IN_hInstance);

	static FrostbiteEngineApplication* GetEngineApplicationInstance();


	virtual int EngineInstanceTick();

private:

	bool CreateEngineInstanceWindow();
};
