#pragma once

#include "Runtime/Core/CoreMinimal.h"
#include "Runtime/Engine/Level/FrostbiteLevel.h"

class GameInsBoxLevelPrivew :public FrostbiteLevel
{

public:

	GameInsBoxLevelPrivew(FrostbiteWorld* IN_OwenrWorld, const FEString& IN_LevelName);

	virtual void InitializeFrostbiteLevel();

	virtual void UpdateFrostbiteLevel();

	virtual void DrawFrostbiteLevel();
};