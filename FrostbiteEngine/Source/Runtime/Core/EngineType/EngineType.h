#pragma once

#include <wrl.h>  //���WTL֧�� ����ʹ��COM

using namespace Microsoft::WRL;

template <typename Type>
class TSharedDirectXPtr :public Microsoft::WRL::ComPtr<Type>
{
	
};
