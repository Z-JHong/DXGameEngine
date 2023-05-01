#pragma once
#include "Runtime/Core/CoreMinimal.h"

class FrostbiteEngineApplication
{

private:

	static FrostbiteEngineApplication* EngineApplication;

	FrostbiteEngineApplication();

protected:

	// ¿ΩÁ£∫
	class FrostbiteWorld* FrostbiteEngineWorld;

private:

	HINSTANCE EngineInstance;
	HWND      EngineInstanceMainWnd = nullptr;

	//RHI:
	class RenderHardwareInterface* EngineRenderRHI;

public:

	virtual bool InitEngineInstance(HINSTANCE IN_hInstance);

	static FrostbiteEngineApplication* GetEngineApplicationInstance();

	class RenderHardwareInterface* GetEngineRenderHardwareInterface();

	virtual int EngineInstanceTick();

	class FrostbiteWorld* GetApplicationWorld();

private:

	bool CreateEngineInstanceWindow();
};

