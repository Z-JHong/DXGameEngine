#pragma once

#include "Runtime/Core/CoreMinimal.h"


struct FrostbiteBasicVertex
{
	DirectX::XMFLOAT3 Pos;

	DirectX::XMFLOAT4 Color;

	FrostbiteBasicVertex();

	FrostbiteBasicVertex(DirectX::XMFLOAT3 IN_Pos, DirectX::XMFLOAT4 IN_Color);
};

