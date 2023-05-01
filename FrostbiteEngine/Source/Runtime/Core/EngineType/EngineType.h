#pragma once

#include <wrl.h>  //添加WTL支持 方便使用COM

using namespace Microsoft::WRL;

template <typename Type>
class TSharedDirectXPtr :public Microsoft::WRL::ComPtr<Type>
{
	
};
