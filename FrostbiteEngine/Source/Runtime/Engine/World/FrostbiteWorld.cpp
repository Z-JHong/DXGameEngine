
#include "FrostbiteWorld.h"
#include "Developer/RHI/RenderHardwareInterface.h"
#include "Runtime/Engine/Application/EngineApplication.h"
#include "Runtime/Engine/Level/FrostbiteLevel.h"
#include "ZProjectGame/ProjectInstanceLevel.h"

FrostbiteWorld::FrostbiteWorld(FrostbiteEngineApplication* IN_OwnerInstacne): OwnerInstacne(IN_OwnerInstacne)
{
	
}

void FrostbiteWorld::CreateWorldCurrentLevel(const FEString& IN_LevelName, const FEString& IN_LevelType)
{
	if (IN_LevelType._Equal("Basic"))
	{
		//创建Basiclevel：
		CurrentLevel = new FrostbiteLevel(this, IN_LevelName);
	}
	else if (IN_LevelType._Equal("GameInsBoxLevelPrivew"))
	{
		CurrentLevel = new GameInsBoxLevelPrivew(this, IN_LevelName);
	}
}

void FrostbiteWorld::InitializeFrostbiteWorld(HWND IN_HWnd)
{
	this->WorldWindowHWnd = IN_HWnd;

	bool bRHIInitSuccess = this->GetApplicationInstance()->GetEngineRenderHardwareInterface()->InitEngineRHI(this->WorldWindowHWnd);

	if (bRHIInitSuccess)
	{
		if (this->CurrentLevel != nullptr)
		{
			this->CurrentLevel->InitializeFrostbiteLevel();
		}
	}
}

FrostbiteEngineApplication* FrostbiteWorld::GetApplicationInstance()
{
	return this->OwnerInstacne;
}

void FrostbiteWorld::UpdateFrostbiteWorld()
{
	if (this->GetApplicationInstance() != nullptr && this->GetApplicationInstance()->GetEngineRenderHardwareInterface() != nullptr)
	{
		//更新RHI：
		this->GetApplicationInstance()->GetEngineRenderHardwareInterface()->UpdateEngineRHI();
	}

	if (this->CurrentLevel != nullptr)
	{
		this->CurrentLevel->UpdateFrostbiteLevel();
	}
}

void FrostbiteWorld::DrawFrostbiteWorld()
{
	if (this->GetApplicationInstance() != nullptr && this->GetApplicationInstance()->GetEngineRenderHardwareInterface() != nullptr)
	{
		this->GetApplicationInstance()->GetEngineRenderHardwareInterface()->DrawEngineRHI();
	}

	if (CurrentLevel != nullptr)
	{
		CurrentLevel->DrawFrostbiteLevel();
	}
}

void FrostbiteWorld::FrostbiteWorldTick()
{
	this->UpdateFrostbiteWorld();

	this->DrawFrostbiteWorld();

}
