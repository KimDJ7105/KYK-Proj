#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	// �������̽� �����͵�
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	// Command
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandList = NULL;

	// Pipeline State
	m_pd3dPipelineState = NULL;

	// Render Target
	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_ppd3dSwapChainBackBuffers[i] = NULL;
	}
	m_pd3dRtvDescriptorHeap = NULL;
	m_nRtvDescriptorIncrementSize = 0;

	// Depth Stencil Buffers
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDsvDescriptorHeap = NULL;
	m_nDsvDescriptorIncrementSize = 0;

	// Swap Chain Buffers
	m_nSwapChainBufferIndex = 0;

	// Fence
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;
	m_hFenceEvent = NULL;

	// Client Window Size
	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	// Frame Rate
	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));

	// Scene
	m_pScene = NULL;
}

CGameFramework::~CGameFramework()
{

}


//--------------------------------------------------------------------------
// ���� �Լ��� ���� ���α׷��� ����Ǿ� �� �����찡 �����Ǹ� ȣ��ȴ�,
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// Direct3D ����̽�, ��� ť�� ��� ����Ʈ, ���� ü�� ���� �����ϴ� �Լ� ȣ��
	// Pipeline
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();	

	// View
	CreateDepthStencilView();

	//�÷��̾�� ���� ����(��)�� �����Ѵ�.
	BuildObjects();

	return (true);
}

void CGameFramework::OnDestroy()
{
	// GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.
	WaitForGpuComplete();

	// ��� ��ɸ���Ʈ�� �����ϸ�
	// ���� ��ü(���� ���� ��ü)�� �Ҹ��Ѵ�.
	ReleaseObjects();

	// �ڵ� ����
	::CloseHandle(m_hFenceEvent);

	// Render Target
	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	}
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	// Depth Stencil
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	// Command
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	// Pipeline State
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();

	// Fence
	if (m_pd3dFence) m_pd3dFence->Release();

	// �������̽� �����͵�
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiFactory) m_pdxgiFactory->Release();
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();

	// ����� ��忡���� ���
#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}
//--------------------------------------------------------------------------

void CGameFramework::CreateSwapChain()
{
	// ������ ũ�� ��������(�������� ���� ũ������)
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);

	// Client Window Size
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;


	// Swap Chain Description - New Ver.(for the Full Screen Mode)
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));	// �޸� �ʱ�ȭ
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ����ۿ� ������ �۾��� ����� ǥ���Ұſ���. ��� �ǹ�
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// �� ���۸� ������ ���ο� �������� �������Ҷ� ���
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;		// ��Ƽ���ø��� ��(m_bMsaa4xEnable�� ���� False������ �۵��� ���Ѵ�) Count�� ���߻��ø��� ���� �ȼ��� ����
	dxgiSwapChainDesc.SampleDesc.Quality =
		(m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;			// ��Ƽ���ø��� ǰ��(False������ �۵��� ���Ѵ�)	0���� ���������� �۵��� ���ϰٴٴ� �ǹ�
	dxgiSwapChainDesc.Windowed = TRUE;									// ��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü���� ũ�⿡ �°� �����Ѵ�.
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// SWITCH ��带 ����Ѵ�.
	
	// Create Swap Chain
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, 
		&dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);

	// Alt + Enter ����� ��Ȱ��ȭ ��Ų��.
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	// ���� Swap Chain�� Back Buffer�� Index���� �����Ѵ�. ������ �۾��� ������� ���ȴ�.
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	// DXGI Factory ����
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);


	//��� �ϵ���� ����� ���Ͽ� Ư�� ���� 12.0(D3D_FEATURE_LEVEL_12_0)�� �����ϴ� �ϵ���� ����̽��� �����Ѵ�.
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(
			pd3dAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}
	//Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� �����Ѵ�.
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)m_pd3dDevice);
	}

	// Msaa Level ����
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;										// Msaa4x ���߻��ø��� �Ұ��̱⿡ ���� 4�� ����
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;									// 0���� ���� �����Ͽ��� ǰ�������� ������ ���� ���� �����Ǵ� ���߻��ø� ������ Ȯ���� �� �ִ�.
	// ����̽��� �����ϴ� ���� ������ ǰ�� ���� Ȯ��. � ���ر��� �ϴ����� Ȯ����
	m_pd3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;				// ������ Ȯ���� ǰ�� �������� m_nMsaa4xQualityLevels���ٰ� ����־��ش�.
	// ���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	// Fence ����						Fence���� 0����
	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	// Fence�� ����ȭ�� ���� Event ��ü�� ����(�ʱⰪ�� FALSE)
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	// Command Queue
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;				// GPU�� ���� ����� ������., 
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(
		&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	// Command Allocator
	hResult = m_pd3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	// ���� ��� �Ҵ��ڸ� �����Ѵ�

	// Command List
	hResult = m_pd3dDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12CommandList), (void**)&m_pd3dCommandList);
	// ���� ��� ����Ʈ�� �����Ѵ�

	hResult = m_pd3dCommandList->Close();
	// ��� ����Ʈ�� ó�� ��������� ����(Open)�����̹Ƿ�, �� ����� ���� �� �ݾ�(Close)��� �Ѵ�.
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	// Create Render Target View Descriptor Heap
	// RTV�� ���� Descriptor Heap�� ���� ����
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;				// �������� ���� = ����ü�� ������ ����
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			// Descriptor Heap�� Render Target View�� �������� �����.
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;										// ��尪�� �ǹ�? ���� �׷���ī���϶� �ǹ��ִ�. �׷���ī�� �ϳ��� ���Ŵϱ� ��尪�� 0
	// ������ ���� �о�ͼ� Descriptor Heap�� ����
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(
		&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	// Render Target Descriptor Heap�� ���� ũ�⸦ ����.
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create Depth Stencil View Descriptor Heap
	// DSV�� ���� Descriptor Heap�� ���� ����
	d3dDescriptorHeapDesc.NumDescriptors = 1;								// �������� ���� = 1��
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	// ������ ���� �о�ͼ� Descriptor Heap�� ����
	hResult = m_pd3dDevice->CreateDescriptorHeap(
		&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	// Depth Stencil Descriptor Heap�� ���� ũ�⸦ ����.
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// ��������� ������ ��ġ�� RTV�� ���� ���� Size, DSV�� ���� ���� Size, �� 4������ ���������.
}

void CGameFramework::CreateRenderTargetViews()
{
	// ����ü���� �� �ĸ���ۿ� ���� ���� Ÿ�� �並 �����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();		// Descriptor Heap�� ���� �ּҸ� �����´�.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)																				// Swap Chain�� ������ŭ
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);						// ���⼭ ������� ������ �����ͼ�
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);					// RTV�� �׷��ߵǴ� ������ �˷��ش�. (���ҽ��� ���� �������̽� ������, View�� �ش��ϴ� Description, Descriptor Heap�� �ּ�)
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;															// ���� RTV�� ���� ��ũ���� �ڵ��� �غ��ϱ� ���� �������� ��ġ�� �������� �ѱ��.
	}
}

void CGameFramework::CreateDepthStencilView()
{
	// Create Depth Stencil Buffer
	// Resource�� ���� ������ �������ִ� �κ�
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;								// Resource�� Buffer�������� Texture���������� �����ϴ°�, Buffer�� �׸������Ͱ� �ƴѰ�, �츮�� �ַ� �̿� ���� TEXTURE2D�� ������ ���� �� ���̴�.
	d3dResourceDesc.Alignment = 0;																// Resource�� ���� �ּҵ��� � ����� �Ǿ��ִ°�, 0(64KB, ���� ȿ����), 4KB, 4MB......0 ���� �˾Ƽ� �������شٰ� ���� �ȴ�.
	d3dResourceDesc.Width = m_nWndClientWidth;													// �����ȼ���
	d3dResourceDesc.Height = m_nWndClientHeight;												// �����ȼ���
	d3dResourceDesc.DepthOrArraySize = 1;														// ���� Ȥ�� �迭ũ��, ������ Texture2D�� �����ؼ� ���̰��� ���� 1�� �����Ͽ���.
	d3dResourceDesc.MipLevels = 1;																// Mip����, Texture�� ����Ҷ� ��������, 0�� ���� �ڵ����� ������ش�, 1�� ����� ������ �˶�
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;										// ���ҽ� ����
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;								// MSAA(���߻��ø�)����� �Ҽ��ְ� �������� �� �ִ�.
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;										// ������ �ؽ��ĸ� 1���� ���ҽ��� �����ϰ� �ϴ� ���, UNKNOWN�� �ϸ� �˾Ƽ� �������� ���� ������ ���� ���� �ִ�.
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;							// Resource�� � �뵵�� ���ɰ��ΰ��� �ݵ�� �������� �Ѵ�, ���� ���Ľ� ���ҽ��� ����Ұ��Դϴ� ��� �˷��ִ°�
	
	// Heap�� ���� ������ �������ִ� �κ�
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;											// Heap�� ����(DEFAULT - GPU�� �а� ���� ����, CPU�� �Ұ�/UNLOAD -  CPU �� Write�� �� �ִ� ��Ÿ���� ����/READBACK - CPU �� Read�� �� �ִ� ��Ÿ���� ����/CUSTOM - �������)
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;						// UNKNOWN�� �ϸ� �˾Ƽ� ���ش�.
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;							// CPU�� �뿪���� ũ������, GPU�� �뿪���� ũ������ ����, �̰ŵ� UNKNOWN�ϸ� �˾Ƽ� ���ش�.
	d3dHeapProperties.CreationNodeMask = 1;														// ���� GPU ������̳Ŀ� ���� �������ָ� �ȴ�.(���� �Ű� �Ƚᵵ �ȴ�)
	d3dHeapProperties.VisibleNodeMask = 1;														// ���߾�����϶� ������ ��Ʈ���� ����(���� �Ű� �Ƚᵵ �ȴ�)
	
	// Clear Value ����, ������ ������� �ʱ�ȭ�� �ؾ��Ѵ�.
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;										// Depth�� Stencil�� Resource�� �ް� �� Resource�� ������ ���� ����.
	d3dClearValue.DepthStencil.Depth = 1.0f;													// �������(�ʱ�ȭ �Ϸ���) Depth��
	d3dClearValue.DepthStencil.Stencil = 0;														// �������(�ʱ�ȭ �Ϸ���) Stencil��
	// ���� Clear�� ������ �Ϲ����� Buffer�� OptimizedClearValue�� ����� �� �����Ƿ� NULL�� ����Ѵ�.
	// NULL�� ��쿡�� RESOURCE_DIMENSION�� Buffer�� �Ǿ�� �Ѵ�. D3D12_RESOURCE_DIMENSION_BUFFER 
	
	// ���� ���� 3���� ���ļ� ���������� Depth Stencil Buffer View�� �� Resource�� ����
	m_pd3dDevice->CreateCommittedResource(														// �Է� ���� ��ü�� ����
		&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,																	// �������� HEAP�� �Ӽ��� ������ �� ����,(�⺻������ NONE�� ����ϸ� �ȴ�.)HEAP�� Texture�� ������ �� �ִ°�, Ȥ�� Resource�� ����͵� ���̿��� ������ �����Ѱ��� ��Ÿ����.
		&d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,														// Resource�� �ʱ� ���°��� ����, ����ϴ� ������ ������ ����. �ش� ���´� Depth�� Write�ϴ� ���¼���
		&d3dClearValue,
		__uuidof(ID3D12Resource),
		(void**)&m_pd3dDepthStencilBuffer);

	// Create Depth Stencil Buffer View
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);						// Command List�� ������Ʈ ���� ���� �ʱ�ȭ �����ָ� �׷����� ������ ��������

// Scene
	// Scene ��ü�� �����ϰ� Scene�� ���Ե� ���� ��ü���� ����
	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

//�÷��̾� ���� ��ü�� �����ϰ� ī�޶�� ��ġ�� �����Ѵ�.
	// ���� Airplane�� Camera Mode�� ���� ī�޶� �����.
	CBoxPlayer* pAirplanePlayer = new CBoxPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pPlayer = pAirplanePlayer;
	m_pCamera = m_pPlayer->GetCamera();

	CGreyField* pGreyField = new CGreyField(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pField = pGreyField;

	// Scene ��ü�� �����ϱ� ���� �ʿ��� Graphics Command List ���� Command Queue�� �߰��Ѵ�.
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommanaLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommanaLists);


// Command List
	// Graphics Command List���� ��� ����� �� ���� ��ٸ���.
	WaitForGpuComplete();

	// Graphics Resource���� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ�
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

// ���콺 �Է�
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//���콺�� �������� ���콺 ��ŷ�� �Ͽ� ������ ���� ��ü�� ã�´�.
		//m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		//�ƹ����� ���ϰ������ �ּ�ó���� ���ܹ�����
		if (m_pSelectedObject)
		{
			m_pSelectedObject->SelectObjectRender(false);
		}
		::SetCapture(hWnd);					// ���콺 ĸ�ĸ� �ϰ� 
		::GetCursorPos(&m_ptOldCursorPos);	// ���� ���콺 ��ġ�� �����´�.
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		
		::ReleaseCapture();					// ���콺 ĸ�ĸ� �����Ѵ�.
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
// Ű���� �Է�
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:					//��F1�� Ű�� ������ 1��Ī ī�޶�
		case VK_F2:					//��F2�� Ű�� ������ �����̽�-�� ī�޶�
		case VK_F3:					//��F3�� Ű�� ������ 3��Ī ī�޶�
			if (m_pPlayer) m_pCamera = m_pPlayer->ChangeCamera((wParam - VK_F1 + 1),
				m_GameTimer.GetTimeElapsed());
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F4:
			break;
		case VK_F5:
			break;
		case VK_F6:
			break;
		case VK_F7:
			break;
		case VK_F8:
			break;
		case VK_F9:					// F9Ű�� ������ ��üȭ�� ��尡 �ȴ�.
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
// ������ ó��
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;

	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);		// ������ ��ü ȭ�� ��带 ������
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);		// ������ ��üȭ�� ��带 ����

	// ���ο� Description�� ������ش�.
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;					// �ʴ� "60"���������� �������ּ���
	dxgiTargetParameters.RefreshRate.Denominator = 1;					// Numerator�� ������ ���� "1"��ŭ �����ּ���
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);				// ���� �� ��ũ������ Swap Chain�� �־��ش�.

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dSwapChainBackBuffers[i])
		{
			m_ppd3dSwapChainBackBuffers[i]->Release();
		}
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta)
{
	//��ŷ���� ������ ���� ��ü�� ������ Ű���带 �����ų� ���콺�� �����̸� ���� ��ü�� �̵� �Ǵ� ȸ���Ѵ�.
	// ���� ���� ������ �ٲ۴ٸ�? ��ŷ�� ���� �� �ϴ� ���� �ٲ��� ������?
	// ��ŷ�� �ϰ� �����̸� ����ǹ�����. ������Ʈ ����°� �ٸ������� �Ѵ�.
	/*if (dwDirection != 0)
	{
		if (dwDirection & DIR_FORWARD) m_pSelectedObject->MoveForward(+1.0f);
		if (dwDirection & DIR_BACKWARD) m_pSelectedObject->MoveForward(-1.0f);
		if (dwDirection & DIR_LEFT) m_pSelectedObject->MoveStrafe(+1.0f);
		if (dwDirection & DIR_RIGHT) m_pSelectedObject->MoveStrafe(-1.0f);
		if (dwDirection & DIR_UP) m_pSelectedObject->MoveUp(+1.0f);
		if (dwDirection & DIR_DOWN) m_pSelectedObject->MoveUp(-1.0f);
	}
	else if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
	m_pSelectedObject->Rotate(cyDelta, cxDelta, 0.0f);
	}*/
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;
	//Ű������ ���� ������ ��ȯ�Ѵ�. ȭ��ǥ Ű(���桯, ���硯, ���衯, ���顯)�� ������
	//�÷��̾ ������/����(���� x-��), ��/��(���� z-��)�� �̵��Ѵ�.
	//��Page Up���� ��Page Down�� Ű�� ������ �÷��̾ ��/�Ʒ�(���� y-��)�� �̵��Ѵ�.
	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;

		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;


		//���콺�� ĸ�������� ���콺�� �󸶸�ŭ �̵��Ͽ��� ���� ����Ѵ�.
		//���콺 ���� �Ǵ� ������ ��ư�� ������ ���� �޽���(WM_LBUTTONDOWN, WM_RBUTTONDOWN)�� ó���� �� ���콺�� ĸ���Ͽ���.
		//�׷��Ƿ� ���콺�� ĸ�ĵ� ���� ���콺 ��ư�� ������ ���¸� �ǹ��Ѵ�.
		//���콺 ��ư�� ������ ���¿��� ���콺�� �¿� �Ǵ� ���Ϸ� �����̸� �÷��̾ x-�� �Ǵ� y-������ ȸ���Ѵ�.
		if (::GetCapture() == m_hWnd)
		{
			//���콺 Ŀ���� ȭ�鿡�� ���ش�(������ �ʰ� �Ѵ�).
			::SetCursor(NULL);

			//���� ���콺 Ŀ���� ��ġ�� �����´�.
			::GetCursorPos(&ptCursorPos);

			//���콺 ��ư�� ���� ���¿��� ���콺�� ������ ���� ���Ѵ�.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;

			//���콺 Ŀ���� ��ġ�� ���콺�� �������� ��ġ�� �����Ѵ�.
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		//���콺 �Ǵ� Ű �Է��� ������ �÷��̾ �̵��ϰų�(dwDirection) ȸ���Ѵ�(cxDelta �Ǵ� cyDelta).
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				///cxDelta�� y-���� ȸ���� ��Ÿ���� cyDelta�� x-���� ȸ���� ��Ÿ����.
				//������ ���콺 ��ư�� ������ ��� cxDelta�� z-���� ȸ���� ��Ÿ����.
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			//�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�).
			//�̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�.
			//�÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�.
			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	//�÷��̾ ������ �̵��ϰ� ī�޶� �����Ѵ�. �߷°� �������� ������ �ӵ� ���Ϳ� �����Ѵ�
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}
//�Ʒ��� ��ŷ�� �� ������Ʈ�� �����̷��� ����� ���μ���
//void CGameFramework::ProcessInput()
//{
//	static UCHAR pKeyBuffer[256];
//	DWORD dwDirection = 0;
//
//	float cxDelta = 0.0f, cyDelta = 0.0f;
//	POINT ptCursorPos;
//
//	if (::GetKeyboardState(pKeyBuffer))
//	{
//		if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
//		if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
//		if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
//		if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
//		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
//		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
//
//		if (::GetCapture() == m_hWnd)
//		{
//			//���콺 Ŀ���� ȭ�鿡�� ���ش�(������ �ʰ� �Ѵ�).
//			::SetCursor(NULL);
//
//			//���� ���콺 Ŀ���� ��ġ�� �����´�.
//			::GetCursorPos(&ptCursorPos);
//
//			//���콺 ��ư�� ���� ���¿��� ���콺�� ������ ���� ���Ѵ�.
//			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
//			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
//
//			//���콺 Ŀ���� ��ġ�� ���콺�� �������� ��ġ�� �����Ѵ�.
//			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
//		}
//	}
//
//	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
//	{
//		if (m_pSelectedObject) // ���� �ȵ� ������Ʈ�� ������
//		{
//			ProcessSelectedObject(dwDirection, cxDelta, cyDelta); // �� ������ �����Ѵ�.
//		}
//		else
//		{
//			if (cxDelta || cyDelta)
//			{
//				/*cxDelta�� y-���� ȸ���� ��Ÿ���� cyDelta�� x-���� ȸ���� ��Ÿ����.
//				������ ���콺 ��ư�� ������ ��� cxDelta�� z-���� ȸ���� ��Ÿ����.*/
//				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
//					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
//				else
//					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
//			}
//			/*�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�).
//			�̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�.
//			�÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�.*/
//			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
//		}
//	}
//	//�÷��̾ ������ �̵��ϰ� ī�޶� �����Ѵ�. �߷°� �������� ������ �ӵ� ���Ϳ� �����Ѵ�
//	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
//}

void CGameFramework::AnimateObjects()
{
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::WaitForGpuComplete()
{
	
	// CPU�� Fence���� ������Ų��
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];

	// GPU�� Fence�� ���� �����ϴ� ����� Command Queue�� �߰��Ѵ�.
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	// Fence�� ���� ���� ������ ������ �۴ٸ�
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		// Fence�� ���� ���� ������ ���� �� ������ ��ٸ���.
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

// �÷��̾(������Ʈ �迭�� �ش����� �ʴ� �ֵ�)�� �׻� �Ǿտ� ������ �ǰ� �Ѵ�.
//#define _WITH_PLAYER_TOP


void CGameFramework::FrameAdvance()
{
	// Timer�� �ð��� ���ŵǵ��� �ϰ� Frame Rate�� ����Ѵ�.
	m_GameTimer.Tick(0.0f);

	//����� �Է��� ó���Ѵ�.
	ProcessInput();

	//���� ���踦 �ִϸ��̼�(�����̰�)�Ѵ�.
	AnimateObjects();

	// ��� �Ҵ���(Command Allocator)�� ��� ����Ʈ(Command List)�� �����Ѵ�.
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	// ���� Render Target�� ���� Present�� �����⸦ ��ٸ���.
	// Present�� �����ٸ� Render Target Buffer�� ���´� Present���¿��� Render Target���·� �ٲ���̴�.
	// D3D12_RESOURCE_STATE_PRESENT --> D3D12_RESOURCE_STATE_RENDER_TARGET
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;														// ���ҽ� �踮� ���¸� ��ȯ���ִ� ������ �Ѵ�.
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;									// NONE - �⺻����, BEGIN_ONLY - ���ۺκи� ����, END_ONLY - ���κи� ����
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// ������ Render Target�� �ش��ϴ� Descriptor�� CPU Handle(�ּ�)�� ���
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);		// ����ϰ��� ��������....
	
	// Depth Stencil Descriptor�� CPU Handle(�ּ�)�� ���
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	// Render Target View�� Depth Stencil View�� OM(���-���� �ܰ�)�� ����
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);											// (Render Target�� ������ ����, Render Target�� ���� �ּ�, Descriptor�� ������ ����������(True�̸� �������̴�), Depth Stencil ������1��)
																																					// ���� FALSE�� �ȴٸ�, �ּҰ� �ƴ� �迭�� �������� �ǳ���� �Ѵ�.

	// ���ϴ� �������� Render Target View�� �ʱ�ȭ �Ѵ�.
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };											// RGBA, �ش� ������ Azure�̴�.
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);														// (�� ��ũ���� �ڵ��� ��Ÿ���� RenderTarget��, �� ������, Struct�� ������ŭ ,Render Target ��ü(NULL)�� Clear����)
	// ���ϴ� ������ Depth Stencil View�� �ʱ�ȭ �Ѵ�.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);		// (�� ��ũ���� �ڵ��� ��Ÿ���� RenderTarget��, �̷��� ��������, Depth���� �̷���, Stencil���� �̷���, Depth Stencil ��ü(NULL)�� Clear����)

	// Scene
	//���� �������Ѵ�.
	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

	// ������ �� ģ������ ���� ��//////////////////////////////////////////
	//
	
	
	
	//3��Ī ī�޶��� �� �÷��̾ �׻� ���̵��� �������Ѵ�.
#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(
		d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0,
		NULL);
#endif // _WITH_PLAYER_TOP

	//3��Ī ī�޶��� �� �÷��̾ �������Ѵ�.
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	// �̰� ���̾� �ٴ��� �׷�
	if (m_pField) m_pField->Render(m_pd3dCommandList, m_pCamera);

	//
	/////////////////////////////////////////////////////////////

	// ���� Render Target�� ���� �������� �����⸦ ��ٸ���.
	// GPU�� Render Target (Buffer)�� �� �̻� ������� ������ Render Target�� ���´� Present���·� �ٲ��.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// Command List�� ���� ���·� �����.
	hResult = m_pd3dCommandList->Close();

	// Command List�� Command Queue�� �߰��Ͽ� �����Ѵ�.
	ID3D12CommandList* ppd3dCommandList[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandList), ppd3dCommandList);

	// GPU�� ��� Command List�� ������ �� ���� ��ٸ���.
	WaitForGpuComplete();

	//m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	// �̰� ������ �𸣰ٴµ� LabProject11���� �������.

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	// ������ Frame Rate�� ���ڿ��� �����ͼ� �� �������� Ÿ��Ʋ�� ����Ѵ�.
	// m_pszBuffer ���ڿ��� "LabProject("���� �ʱ�ȭ�� �Ǿ�������
	// m_pszFrameRate + 12 �������� Frame Rate�� ���ڿ��� ����Ͽ� " FPS)"���ڿ��� ��ģ��.
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

