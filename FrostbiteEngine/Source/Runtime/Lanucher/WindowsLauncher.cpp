#pragma once

#include "Runtime/Core/CoreMinimal.h"
#include "Runtime/Engine/Application/EngineApplication.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{

	FrostbiteEngineApplication::GetEngineApplicationInstance()->InitEngineInstance(hInstance);

	FrostbiteEngineApplication::GetEngineApplicationInstance()->EngineInstanceTick();

	return 0;
}
