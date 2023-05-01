#pragma once
#include "Runtime/Core/CoreMinimal.h"

class FrostbiteWorld
{

public:

	FrostbiteWorld(class FrostbiteEngineApplication* IN_OwnerInstacne);

public:

	//������ǰ�ؿ���
	virtual void CreateWorldCurrentLevel(const FEString& IN_LevelName, const FEString& IN_LevelType);

	//��ʼ�����磺
	virtual void InitializeFrostbiteWorld(HWND IN_HWnd);

	//��ȡ��AppInstance��
	virtual FrostbiteEngineApplication* GetApplicationInstance();


protected:

	//�������磺
	virtual void UpdateFrostbiteWorld();

	//�������磺
	virtual void DrawFrostbiteWorld();


public:

	//Tick��
	virtual void FrostbiteWorldTick();

	virtual LRESULT CALLBACK FrostbiteWorldProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	//ʵ������
	class FrostbiteEngineApplication* OwnerInstacne ;

	//��ǰ�ؿ���
	class FrostbiteLevel* CurrentLevel;

	HWND WorldWindowHWnd;
};
