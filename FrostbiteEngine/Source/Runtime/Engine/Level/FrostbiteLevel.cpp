
#include "FrostbiteLevel.h"

FrostbiteLevel::FrostbiteLevel(FrostbiteWorld* IN_OwenrWorld, const FEString& IN_LevelName)
{
	
	this->OwenrWorld = IN_OwenrWorld;

	this->LevelName = IN_LevelName;
}

void FrostbiteLevel::InitializeFrostbiteLevel()
{
}

void FrostbiteLevel::UpdateFrostbiteLevel()
{
}

void FrostbiteLevel::DrawFrostbiteLevel()
{
}

LRESULT FrostbiteLevel::FrostbiteLevelInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
