#include "DirectXRHI.h"
#include "Windows.h"

//初始化DX的RHI：
bool DirectXRHI::InitEngineRHI(HWND IN_HWnd)
{
	//赋值：
	this->pFEDXRHIMainWnd = IN_HWnd;

	//初始化DX：
	return this->InitDefaultDirectX3D();
}

bool DirectXRHI::UpdateEngineRHI()
{
	return true;
}

bool DirectXRHI::DrawEngineRHI()
{
	return true;
}

bool DirectXRHI::InitDefaultDirectX3D()
{

	//01.打开DX3D调试层:
#if defined(DEBUG) || defined(_DEBUG)

	ComPtr<ID3D12Debug> debugController;

	HRESULT CreateDebugController = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

	if(SUCCEEDED(CreateDebugController))
	{
		debugController->EnableDebugLayer();
	}

#endif

	//02.创建DXFactory工厂对象：
	HRESULT CreateFactoryObject = CreateDXGIFactory1(IID_PPV_ARGS(&this->pFEDXGIFactory));

	if(FAILED(CreateFactoryObject))
	{
		//Log:
		return false;
	}

	//03.创建DX设备对象：
	DXGI_ADAPTER_DESC1 stAdapterDesc = {};

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != this->pFEDXGIFactory->EnumAdapters1(adapterIndex, &this->pFEDXAdapter); ++adapterIndex)
	{
		this->pFEDXAdapter->GetDesc1(&stAdapterDesc);

		if (stAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{//跳过软件虚拟适配器设备
			continue;
		}
		//检查适配器对D3D支持的兼容级别，这里直接要求支持12.1的能力，注意返回接口的那个参数被置为了nullptr，这样
		//就不会实际创建一个设备了，也不用我们啰嗦的再调用release来释放接口。这也是一个重要的技巧，请记住！
		if (SUCCEEDED(D3D12CreateDevice(this->pFEDXAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	HRESULT CreateDXDevice = D3D12CreateDevice(this->pFEDXAdapter.Get(), D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&this->pFED3D12Device));

	if(FAILED(CreateDXDevice))
	{
		//Log:
		return false;
	}


#if defined(DEBUG) || defined(_DEBUG)

	//输出设备信息：
	TCHAR pszWndTitle[MAX_PATH] = {};

	this->pFEDXAdapter->GetDesc1(&stAdapterDesc);

	GetWindowText(this->pFEDXRHIMainWnd, pszWndTitle, MAX_PATH);

	StringCchPrintf(pszWndTitle, MAX_PATH, _T("%s (GPU:%s)"), pszWndTitle, stAdapterDesc.Description);

	SetWindowText(this->pFEDXRHIMainWnd, pszWndTitle);

#endif

	//04.创建围栏并获取描述符的大小
	this->pFED3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&this->pFEFence));

	pFERtvDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	pFEDsvDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	pFECbvSrvUavDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//05.检测对 4X MSAA 质量级别的支持

	//检查我们的后台缓冲区格式的4X MSAA质量支持。
	//所有支持Direct3D 11的设备都支持所有渲染目标格式的4X MSAA，所以我们只需要检查质量支持。

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = pFEBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;


	this->pFED3D12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels));

	this->pFE4xMsaaQuality = msQualityLevels.NumQualityLevels;

	assert(this->pFE4xMsaaQuality > 0 && "Unexpected MSAA quality level.");


	//06.创建命令队列和命令列表

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT CreateCMDQueue = this->pFED3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->pFECMDQueue));

	if (FAILED(CreateCMDQueue))
	{
		//Log:
		return false;
	}

	HRESULT CreateCMDListAlloc = this->pFED3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(this->pFEDirectCmdListAlloc.GetAddressOf()));

	if (FAILED(CreateCMDListAlloc))
	{
		//Log:
		return false;
	}

	this->pFED3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->pFEDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(this->pFECommandList.GetAddressOf()));

	// 在关闭状态下开始。这是因为当我们第一次引用命令列表时，我们将重置它，并且在调用Reset之前需要关闭它。
	this->CloseGraphicsCommandList();


	//07.描述并创建交换链
	this->pFESwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = this->pFEClientWidth;
	sd.BufferDesc.Height = this->pFEClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = this->pFEBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = this->pFE4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = this->pFE4xMsaaState ? (this->pFE4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = this->pFEDXRHIMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	HRESULT CreateSpawnChain = this->pFEDXGIFactory->CreateSwapChain(this->pFECMDQueue.Get(), &sd, this->pFESwapChain.GetAddressOf());

	if (FAILED(CreateSpawnChain))
	{
		//Log:
		return false;
	}

	//08.创建描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	HRESULT CreateRTVHeap = this->pFED3D12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(this->pFERtvHeap.GetAddressOf()));

	if (FAILED(CreateRTVHeap))
	{
		//Log:
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	//创建DSVHeap：
	HRESULT CreateDSVHeap = this->pFED3D12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(this->pFEDsvHeap.GetAddressOf()));

	if (FAILED(CreateDSVHeap))
	{
		//Log:
		return false;
	}

	//在改变任何资源之前需要刷新命令队列：
	this->FlushCommandQueue();

	this->ResetGraphicsCommandList();

	//09.创建渲染目标视图

	//10.创建深度/模板缓冲区及其视图

	//创建模板缓冲：
	this->CreateRenderTargetViewportResoureces();

	//创建深度缓冲：
	this->CreateDepthStencilResources();


	// Execute the resize commands.
	this->ExecuteCommandQueue();

	// Wait until resize is complete.
	this->FlushCommandQueue();


	//11.设置视口
	// Update the viewport transform to cover the client area.
	this->pFEScreenViewport.TopLeftX = 0;
	this->pFEScreenViewport.TopLeftY = 0;
	this->pFEScreenViewport.Width = static_cast<float>(this->pFEClientWidth);
	this->pFEScreenViewport.Height = static_cast<float>(this->pFEClientHeight);
	this->pFEScreenViewport.MinDepth = 0.0f;
	this->pFEScreenViewport.MaxDepth = 1.0f;

	//12.设置裁剪矩形
	this->pFEScissorRect = { 0, 0, this->pFEClientWidth, this->pFEClientHeight };


	return true;

}

void DirectXRHI::ResetRHISwapChainResources()
{
	// Release the previous resources we will be recreating.
	for (int SwapChainIndex = 0; SwapChainIndex < SwapChainBufferCount; ++SwapChainIndex)
	{
		this->pFESwapChainBuffer[SwapChainIndex].Reset();
	}

	HRESULT ResizeBuffer = this->pFESwapChain->ResizeBuffers(SwapChainBufferCount, this->pFEClientWidth, this->pFEClientHeight, this->pFEBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	if (FAILED(ResizeBuffer))
	{
		//Log:
	}

	this->pFECurrBackBuffer = 0;
}

void DirectXRHI::CreateDepthStencilResources()
{
	//指针重置：
	this->pFEDepthStencilBuffer.Reset();

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = this->pFEClientWidth;
	depthStencilDesc.Height = this->pFEClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = this->pFEDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = this->pFE4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = this->pFE4xMsaaState ? (this->pFE4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = this->pFEDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES DSVBufferPRO (D3D12_HEAP_TYPE_DEFAULT);

	this->pFED3D12Device->CreateCommittedResource(&DSVBufferPRO, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(this->pFEDepthStencilBuffer.GetAddressOf()));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	this->pFED3D12Device->CreateDepthStencilView(this->pFEDepthStencilBuffer.Get(), nullptr, this->pFEDsvHeap->GetCPUDescriptorHandleForHeapStart());

	// Transition the resource from its initial state to be used as a depth buffer.

	CD3DX12_RESOURCE_BARRIER DSVRB = CD3DX12_RESOURCE_BARRIER::Transition(this->pFEDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	this->pFECommandList->ResourceBarrier(1, &DSVRB);

}

void DirectXRHI::CreateRenderTargetViewportResoureces()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(this->pFERtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT SwapBufferIndex = 0; SwapBufferIndex < SwapChainBufferCount; SwapBufferIndex++)
	{
		//获取到当前的交换链：
		HRESULT GetCurrentChain = this->pFESwapChain->GetBuffer(SwapBufferIndex, IID_PPV_ARGS(&this->pFESwapChainBuffer[SwapBufferIndex]));

		if (this->pFED3D12Device.Get() != nullptr && SUCCEEDED(GetCurrentChain))
		{
			this->pFED3D12Device->CreateRenderTargetView(this->pFESwapChainBuffer[SwapBufferIndex].Get(), nullptr, rtvHeapHandle);

			rtvHeapHandle.Offset(1, this->pFERtvDescriptorSize);
		}
	}
}

void DirectXRHI::ResetGraphicsCommandList()
{
	HRESULT CurrentResetSuccess;

	if (this->pFECommandList.Get() != nullptr)
	{
		CurrentResetSuccess = this->pFECommandList.Get()->Reset(this->pFEDirectCmdListAlloc.Get(), nullptr);

		if (SUCCEEDED(CurrentResetSuccess))
		{
			
		}
		else
		{
			
		}
	}
}

void DirectXRHI::CloseGraphicsCommandList()
{
	HRESULT CurrentCloseSuccess;

	if (this->pFECommandList.Get() != nullptr)
	{
		CurrentCloseSuccess = this->pFECommandList->Close();

		if (SUCCEEDED(CurrentCloseSuccess))
		{

		}
		else
		{

		}
	}
}

void DirectXRHI::FlushCommandQueue()
{
	// 推进栅格值以将命令标记到此栅格点。
	this->pFECurrentFence++;

	// 向命令队列添加指令以设置新的栅格点。因为我们在GPU时间线上，所以在GPU完成处理此信号之前的所有命令之前，不会设置新的栅格点()。
	HRESULT SingCMDQueue = this->pFECMDQueue->Signal(this->pFEFence.Get(), this->pFECurrentFence);

	if (SUCCEEDED(SingCMDQueue))
	{
		// 等待，直到GPU完成此栅格点的命令。
		if (this->pFEFence->GetCompletedValue() < this->pFECurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			// Fire event when GPU hits current fence.  
			HRESULT SetEventSuccess = this->pFEFence->SetEventOnCompletion(this->pFECurrentFence, eventHandle);

			// Wait until the GPU hits current fence event is fired.
			if (SUCCEEDED(SetEventSuccess))
			{
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
		}
	}
}

void DirectXRHI::ExecuteCommandQueue()
{
	//先关闭命令列表：
	this->CloseGraphicsCommandList();

	if (this->pFECommandList.Get() != nullptr)
	{
		//取出当前命令队列集中的相关命令集合：

		ID3D12CommandList* cmdsLists[] = { this->pFECommandList.Get() };

		if (this->pFECMDQueue.Get() != nullptr)
		{
			this->pFECMDQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		}
	}
}

void DirectXRHI::ResizeEngineWindow()
{
	//在改变任何资源之前需要刷新命令队列：
	this->FlushCommandQueue();

	this->ResetGraphicsCommandList();

	//09.创建渲染目标视图

	//10.创建深度/模板缓冲区及其视图

	//创建模板缓冲：
	this->CreateRenderTargetViewportResoureces();

	//创建深度缓冲：
	this->CreateDepthStencilResources();


	// Execute the resize commands.
	this->ExecuteCommandQueue();

	// Wait until resize is complete.
	this->FlushCommandQueue();


	//11.设置视口
	// Update the viewport transform to cover the client area.
	this->pFEScreenViewport.TopLeftX = 0;
	this->pFEScreenViewport.TopLeftY = 0;
	this->pFEScreenViewport.Width = static_cast<float>(this->pFEClientWidth);
	this->pFEScreenViewport.Height = static_cast<float>(this->pFEClientHeight);
	this->pFEScreenViewport.MinDepth = 0.0f;
	this->pFEScreenViewport.MaxDepth = 1.0f;

	//12.设置裁剪矩形
	this->pFEScissorRect = { 0, 0, this->pFEClientWidth, this->pFEClientHeight };

}
