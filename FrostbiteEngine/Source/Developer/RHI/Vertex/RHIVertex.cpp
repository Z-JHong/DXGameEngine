#include "RHIVertex.h"

FrostbiteBasicVertex::FrostbiteBasicVertex()
{
	this->Pos = DirectX::XMFLOAT3(0, 0, 0);
	//RGBA:
	this->Color = DirectX::XMFLOAT4(0, 0, 0, 0);
}

FrostbiteBasicVertex::FrostbiteBasicVertex(DirectX::XMFLOAT3 IN_Pos, DirectX::XMFLOAT4 IN_Color)
{
	this->Pos = IN_Pos;

	this->Color = IN_Color;
}
