#include "DirectXRHI.h"
#include "Windows.h"

//��ʼ��DX��RHI��
bool DirectXRHI::InitEngineRHI(HWND IN_HWnd)
{
	//��ֵ��
	this->pFEDXRHIMainWnd = IN_HWnd;

	//��ʼ��DX��
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

	//01.��DX3D���Բ�:
#if defined(DEBUG) || defined(_DEBUG)

	ComPtr<ID3D12Debug> debugController;

	HRESULT CreateDebugController = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

	if(SUCCEEDED(CreateDebugController))
	{
		debugController->EnableDebugLayer();
	}

#endif

	//02.����DXFactory��������
	HRESULT CreateFactoryObject = CreateDXGIFactory1(IID_PPV_ARGS(&this->pFEDXGIFactory));

	if(FAILED(CreateFactoryObject))
	{
		//Log:
		return false;
	}

	//03.����DX�豸����
	DXGI_ADAPTER_DESC1 stAdapterDesc = {};

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != this->pFEDXGIFactory->EnumAdapters1(adapterIndex, &this->pFEDXAdapter); ++adapterIndex)
	{
		this->pFEDXAdapter->GetDesc1(&stAdapterDesc);

		if (stAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{//������������������豸
			continue;
		}
		//�����������D3D֧�ֵļ��ݼ�������ֱ��Ҫ��֧��12.1��������ע�ⷵ�ؽӿڵ��Ǹ���������Ϊ��nullptr������
		//�Ͳ���ʵ�ʴ���һ���豸�ˣ�Ҳ�������ǆ��µ��ٵ���release���ͷŽӿڡ���Ҳ��һ����Ҫ�ļ��ɣ����ס��
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

	//����豸��Ϣ��
	TCHAR pszWndTitle[MAX_PATH] = {};

	this->pFEDXAdapter->GetDesc1(&stAdapterDesc);

	GetWindowText(this->pFEDXRHIMainWnd, pszWndTitle, MAX_PATH);

	StringCchPrintf(pszWndTitle, MAX_PATH, _T("%s (GPU:%s)"), pszWndTitle, stAdapterDesc.Description);

	SetWindowText(this->pFEDXRHIMainWnd, pszWndTitle);

#endif

	//04.����Χ������ȡ�������Ĵ�С
	this->pFED3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&this->pFEFence));

	pFERtvDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	pFEDsvDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	pFECbvSrvUavDescriptorSize = this->pFED3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//05.���� 4X MSAA ���������֧��

	//������ǵĺ�̨��������ʽ��4X MSAA����֧�֡�
	//����֧��Direct3D 11���豸��֧��������ȾĿ���ʽ��4X MSAA����������ֻ��Ҫ�������֧�֡�

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = pFEBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;


	this->pFED3D12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels));

	this->pFE4xMsaaQuality = msQualityLevels.NumQualityLevels;

	assert(this->pFE4xMsaaQuality > 0 && "Unexpected MSAA quality level.");


	//06.����������к������б�

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

	// �ڹر�״̬�¿�ʼ��������Ϊ�����ǵ�һ�����������б�ʱ�����ǽ��������������ڵ���Reset֮ǰ��Ҫ�ر�����
	this->CloseGraphicsCommandList();


	//07.����������������
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

	//08.������������
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

	//����DSVHeap��
	HRESULT CreateDSVHeap = this->pFED3D12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(this->pFEDsvHeap.GetAddressOf()));

	if (FAILED(CreateDSVHeap))
	{
		//Log:
		return false;
	}

	//�ڸı��κ���Դ֮ǰ��Ҫˢ��������У�
	this->FlushCommandQueue();

	this->ResetGraphicsCommandList();

	//09.������ȾĿ����ͼ

	//10.�������/ģ�建����������ͼ

	//����ģ�建�壺
	this->CreateRenderTargetViewportResoureces();

	//������Ȼ��壺
	this->CreateDepthStencilResources();


	// Execute the resize commands.
	this->ExecuteCommandQueue();

	// Wait until resize is complete.
	this->FlushCommandQueue();


	//11.�����ӿ�
	// Update the viewport transform to cover the client area.
	this->pFEScreenViewport.TopLeftX = 0;
	this->pFEScreenViewport.TopLeftY = 0;
	this->pFEScreenViewport.Width = static_cast<float>(this->pFEClientWidth);
	this->pFEScreenViewport.Height = static_cast<float>(this->pFEClientHeight);
	this->pFEScreenViewport.MinDepth = 0.0f;
	this->pFEScreenViewport.MaxDepth = 1.0f;

	//12.���òü�����
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
	//ָ�����ã�
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
		//��ȡ����ǰ�Ľ�������
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
	// �ƽ�դ��ֵ�Խ������ǵ���դ��㡣
	this->pFECurrentFence++;

	// ������������ָ���������µ�դ��㡣��Ϊ������GPUʱ�����ϣ�������GPU��ɴ�����ź�֮ǰ����������֮ǰ�����������µ�դ���()��
	HRESULT SingCMDQueue = this->pFECMDQueue->Signal(this->pFEFence.Get(), this->pFECurrentFence);

	if (SUCCEEDED(SingCMDQueue))
	{
		// �ȴ���ֱ��GPU��ɴ�դ�������
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
	//�ȹر������б�
	this->CloseGraphicsCommandList();

	if (this->pFECommandList.Get() != nullptr)
	{
		//ȡ����ǰ������м��е��������ϣ�

		ID3D12CommandList* cmdsLists[] = { this->pFECommandList.Get() };

		if (this->pFECMDQueue.Get() != nullptr)
		{
			this->pFECMDQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		}
	}
}

void DirectXRHI::ResizeEngineWindow()
{
	//�ڸı��κ���Դ֮ǰ��Ҫˢ��������У�
	this->FlushCommandQueue();

	this->ResetGraphicsCommandList();

	//09.������ȾĿ����ͼ

	//10.�������/ģ�建����������ͼ

	//����ģ�建�壺
	this->CreateRenderTargetViewportResoureces();

	//������Ȼ��壺
	this->CreateDepthStencilResources();


	// Execute the resize commands.
	this->ExecuteCommandQueue();

	// Wait until resize is complete.
	this->FlushCommandQueue();


	//11.�����ӿ�
	// Update the viewport transform to cover the client area.
	this->pFEScreenViewport.TopLeftX = 0;
	this->pFEScreenViewport.TopLeftY = 0;
	this->pFEScreenViewport.Width = static_cast<float>(this->pFEClientWidth);
	this->pFEScreenViewport.Height = static_cast<float>(this->pFEClientHeight);
	this->pFEScreenViewport.MinDepth = 0.0f;
	this->pFEScreenViewport.MaxDepth = 1.0f;

	//12.���òü�����
	this->pFEScissorRect = { 0, 0, this->pFEClientWidth, this->pFEClientHeight };

}
