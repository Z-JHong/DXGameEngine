#pragma once

#include "Developer/RHI/RenderHardwareInterface.h"
#include "Runtime/Core/EngineType/EngineType.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3d12.h>	//for d3d12
#include "d3dx12.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif


using namespace DirectX;

//linker
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

class DirectXRHI :public RenderHardwareInterface
{

public:

	void InitEngineRHI(HWND IN_HWnd) override;


	void UpdateEngineRHI() override;

	void DrawEngineRHI() override;

protected:

	void InitDefaultDirectX3D();

	//重置交换链资源：
	virtual void ResetRHISwapChainResources();

	//创建深度缓冲
	virtual void CreateDepthStencilResources();

	virtual void CreateRenderTargetViewportResoureces();

public:

	//重置命令队列：
	virtual void ResetGraphicsCommandList();

	//关闭命令列表：
	virtual void CloseGraphicsCommandList();

	//刷新命令队列：
	virtual void FlushCommandQueue();

	//执行命令队列：
	virtual void ExecuteCommandQueue();

private:


	//工厂对象：
	TSharedDirectXPtr<IDXGIFactory5> pFEDXGIFactory;

	TSharedDirectXPtr<IDXGIAdapter1> pFEDXAdapter;
	TSharedDirectXPtr<ID3D12Device4> pFED3D12Device;

	//命令队列：
	TSharedDirectXPtr<ID3D12CommandQueue> pFECMDQueue;
	TSharedDirectXPtr<ID3D12CommandAllocator> pFEDirectCmdListAlloc;
	TSharedDirectXPtr<ID3D12GraphicsCommandList> pFECommandList;

	//交换链：
	TSharedDirectXPtr<IDXGISwapChain>	pFESwapChain;

	static const int SwapChainBufferCount = 2;
	int pFECurrBackBuffer = 0;
	TSharedDirectXPtr<ID3D12Resource> pFESwapChainBuffer[SwapChainBufferCount];
	TSharedDirectXPtr<ID3D12Resource> pFEDepthStencilBuffer;

	//围栏：
	TSharedDirectXPtr<ID3D12Fence> pFEFence;
	UINT64 pFECurrentFence = 0;

	UINT pFERtvDescriptorSize = 0;
	UINT pFEDsvDescriptorSize = 0;
	UINT pFECbvSrvUavDescriptorSize = 0;

	TSharedDirectXPtr<ID3D12DescriptorHeap> pFERtvHeap;
	TSharedDirectXPtr<ID3D12DescriptorHeap> pFEDsvHeap;

	HWND pFEDXRHIMainWnd = nullptr;


	D3D12_VIEWPORT pFEScreenViewport;
	D3D12_RECT pFEScissorRect;


	int pFEClientWidth = 1280;
	int pFEClientHeight = 720;


	bool      pFE4xMsaaState = false;    // 4X MSAA enabled
	UINT      pFE4xMsaaQuality = 0;      // quality level of 4X MSAA

	D3D_DRIVER_TYPE pFEd3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT pFEBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT pFEDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

};


