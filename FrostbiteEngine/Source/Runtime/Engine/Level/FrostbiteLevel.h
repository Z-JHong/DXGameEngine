#pragma once
#include "Runtime/Core/CoreMinimal.h"
//Frostbite Engine Base Level

class FrostbiteLevel
{

public:

	FrostbiteLevel(class FrostbiteWorld* IN_OwenrWorld, const FEString& IN_LevelName);


	FORCEINLINE class FrostbiteWorld* GetWorld()
	{
		return this->OwenrWorld;
	}

	virtual void InitializeFrostbiteLevel();

	virtual void UpdateFrostbiteLevel();

	virtual void DrawFrostbiteLevel();

protected:

	FEString LevelName;

	class FrostbiteWorld* OwenrWorld;
};