#pragma once
#include "Runtime/Core/CoreMinimal.h"

class FrostbiteWorld
{

public:

	FrostbiteWorld(class FrostbiteEngineApplication* IN_OwnerInstacne);

public:

	//创建当前关卡：
	virtual void CreateWorldCurrentLevel(const FEString& IN_LevelName, const FEString& IN_LevelType);

	//初始化世界：
	virtual void InitializeFrostbiteWorld(HWND IN_HWnd);

	//获取到AppInstance：
	virtual FrostbiteEngineApplication* GetApplicationInstance();


protected:

	//更新世界：
	virtual void UpdateFrostbiteWorld();

	//绘制世界：
	virtual void DrawFrostbiteWorld();


public:

	//Tick：
	virtual void FrostbiteWorldTick();

	virtual LRESULT CALLBACK FrostbiteWorldProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	//实例对象：
	class FrostbiteEngineApplication* OwnerInstacne ;

	//当前关卡：
	class FrostbiteLevel* CurrentLevel;

	HWND WorldWindowHWnd;
};
