#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	// 인터페이스 포인터들
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
// 다음 함수는 응용 프로그램이 실행되어 주 윈도우가 생성되면 호출된다,
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// Direct3D 디바이스, 명령 큐와 명령 리스트, 스왑 체인 등을 생성하는 함수 호출
	// Pipeline
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();	

	// View
	CreateDepthStencilView();

	//플레이어와 게임 세계(씬)을 생성한다.
	BuildObjects();

	return (true);
}

void CGameFramework::OnDestroy()
{
	// GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	WaitForGpuComplete();

	// 모든 명령리스트를 실행하면
	// 게임 객체(게임 월드 객체)를 소멸한다.
	ReleaseObjects();

	// 핸들 정리
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

	// 인터페이스 포인터들
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiFactory) m_pdxgiFactory->Release();
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();

	// 디버그 모드에서만 출력
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
	// 윈도우 크기 가져오기(랜더링을 위한 크기조절)
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);

	// Client Window Size
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;


	// Swap Chain Description - New Ver.(for the Full Screen Mode)
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));	// 메모리 초기화
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 백버퍼에 랜더링 작업의 결과를 표시할거에요. 라는 의미
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// 백 버퍼를 버리고 새로운 프레임을 랜더링할때 사용
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;		// 멀티샘플링의 수(m_bMsaa4xEnable의 값이 False임으로 작동을 안한다) Count는 다중샘플링을 위한 픽셀의 개수
	dxgiSwapChainDesc.SampleDesc.Quality =
		(m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;			// 멀티샘플링의 품질(False임으로 작동을 안한다)	0으로 설정함으로 작동을 안하겟다는 의미
	dxgiSwapChainDesc.Windowed = TRUE;									// 전체화면 모드에서 바탕화면의 해상도를 스왑체인의 크기에 맞게 변경한다.
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// SWITCH 모드를 허락한다.
	
	// Create Swap Chain
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, 
		&dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);

	// Alt + Enter 기능을 비활성화 시킨다.
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	// 현재 Swap Chain의 Back Buffer의 Index값을 저장한다. 랜더링 작업에 여러모로 사용된다.
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
	// DXGI Factory 생성
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);


	//모든 하드웨어 어댑터 대하여 특성 레벨 12.0(D3D_FEATURE_LEVEL_12_0)을 지원하는 하드웨어 디바이스를 생성한다.
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(
			pd3dAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}
	//특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스를 생성한다.
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)m_pd3dDevice);
	}

	// Msaa Level 설정
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;										// Msaa4x 다중샘플링을 할것이기에 값을 4로 지정
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;									// 0으로 값을 지정하여서 품질레벨을 조절할 수는 없고 지원되는 다중샘플링 설정을 확인할 수 있다.
	// 디바이스가 지원하는 다중 샘플의 품질 수준 확인. 어떤 수준까지 하는지를 확인함
	m_pd3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;				// 위에서 확인한 품질 레벨값을 m_nMsaa4xQualityLevels에다가 집어넣어준다.
	// 다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	// Fence 생성						Fence값을 0으로
	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	// Fence와 동기화를 위한 Event 객체를 생성(초기값은 FALSE)
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	// Command Queue
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;				// GPU에 직접 명령을 보낸다., 
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(
		&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	// Command Allocator
	hResult = m_pd3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	// 직접 명령 할당자를 생성한다

	// Command List
	hResult = m_pd3dDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12CommandList), (void**)&m_pd3dCommandList);
	// 직접 명령 리스트를 생성한다

	hResult = m_pd3dCommandList->Close();
	// 명령 리스트는 처음 만들어질때 열린(Open)상태이므로, 다 만들고 나서 잘 닫아(Close)줘야 한다.
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	// Create Render Target View Descriptor Heap
	// RTV에 대한 Descriptor Heap에 대한 설정
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;				// 서술자의 개수 = 스왑체인 버퍼의 개수
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			// Descriptor Heap을 Render Target View의 형식으로 만든다.
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;										// 노드값의 의미? 다중 그래픽카드일때 의미있다. 그래픽카드 하나만 쓸거니까 노드값이 0
	// 설정한 값을 읽어와서 Descriptor Heap을 생성
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(
		&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	// Render Target Descriptor Heap의 원소 크기를 저장.
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create Depth Stencil View Descriptor Heap
	// DSV에 대한 Descriptor Heap에 대한 설정
	d3dDescriptorHeapDesc.NumDescriptors = 1;								// 서술자의 개수 = 1개
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	// 설정한 값을 읽어와서 Descriptor Heap을 생성
	hResult = m_pd3dDevice->CreateDescriptorHeap(
		&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	// Depth Stencil Descriptor Heap의 원소 크기를 저장.
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// 여기까지의 과정을 거치면 RTV에 대한 힙과 Size, DSV에 대한 힙과 Size, 총 4가지가 만들어진다.
}

void CGameFramework::CreateRenderTargetViews()
{
	// 스왑체인의 각 후면버퍼에 대한 랜더 타겟 뷰를 생성한다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();		// Descriptor Heap의 시작 주소를 가져온다.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)																				// Swap Chain의 개수만큼
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);						// 여기서 백버퍼의 내용을 가져와서
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);					// RTV가 그려야되는 내용을 알려준다. (리소스에 대한 인터페이스 포인터, View에 해당하는 Description, Descriptor Heap의 주소)
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;															// 다음 RTV를 위한 디스크립터 핸들을 준비하기 위해 포인터의 위치를 다음으로 넘긴다.
	}
}

void CGameFramework::CreateDepthStencilView()
{
	// Create Depth Stencil Buffer
	// Resource에 대한 내용을 설정해주는 부분
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;								// Resource가 Buffer형식인지 Texture형식인지를 설정하는것, Buffer는 그림데이터가 아닌것, 우리는 주로 이와 같은 TEXTURE2D의 형식을 자주 쓸 것이다.
	d3dResourceDesc.Alignment = 0;																// Resource의 시작 주소들이 어떤 배수로 되어있는가, 0(64KB, 가장 효율적), 4KB, 4MB......0 쓰면 알아서 지정해준다고 보면 된다.
	d3dResourceDesc.Width = m_nWndClientWidth;													// 가로픽셀수
	d3dResourceDesc.Height = m_nWndClientHeight;												// 세로픽셀수
	d3dResourceDesc.DepthOrArraySize = 1;														// 깊이 혹은 배열크기, 위에서 Texture2D로 설정해서 깊이값이 없어 1로 설정하였다.
	d3dResourceDesc.MipLevels = 1;																// Mip레벨, Texture를 사용할때 설정해줌, 0을 쓰면 자동으로 계산해준다, 1은 사용을 안함을 알람
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;										// 리소스 형식
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;								// MSAA(다중샘플링)기능을 할수있게 설정해줄 수 있다.
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;										// 다차원 텍스쳐를 1차원 리소스로 맵핑하게 하는 방법, UNKNOWN을 하면 알아서 해주지만 직접 설정을 해줄 수도 있다.
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;							// Resource가 어떤 용도로 사용될것인가를 반드시 명시해줘야 한다, 깊이 스탠실 리소스로 사용할것입니다 라고 알려주는거
	
	// Heap에 대한 내용을 설정해주는 부분
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;											// Heap의 유형(DEFAULT - GPU가 읽고 쓰기 가능, CPU는 불가/UNLOAD -  CPU 가 Write할 수 있는 힙타입을 만듬/READBACK - CPU 가 Read할 수 있는 힙타입을 만듬/CUSTOM - 내맘대로)
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;						// UNKNOWN을 하면 알아서 해준다.
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;							// CPU로 대역폭을 크게할지, GPU에 대역폭을 크게할지 여부, 이거도 UNKNOWN하면 알아서 해준다.
	d3dHeapProperties.CreationNodeMask = 1;														// 단일 GPU 어댑터이냐에 따라서 지정해주면 된다.(굳이 신경 안써도 된다)
	d3dHeapProperties.VisibleNodeMask = 1;														// 다중어댑터일때 노드들의 비트들을 결정(굳이 신경 안써도 된다)
	
	// Clear Value 생성, 최적의 방법으로 초기화를 해야한다.
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;										// Depth와 Stencil을 Resource로 받고 그 Resource의 구조는 위와 같다.
	d3dClearValue.DepthStencil.Depth = 1.0f;													// 지우려는(초기화 하려는) Depth값
	d3dClearValue.DepthStencil.Stencil = 0;														// 지우려는(초기화 하려는) Stencil값
	// 만약 Clear할 내용이 일반적인 Buffer는 OptimizedClearValue를 사용할 수 없으므로 NULL로 사용한다.
	// NULL인 경우에는 RESOURCE_DIMENSION이 Buffer가 되어야 한다. D3D12_RESOURCE_DIMENSION_BUFFER 
	
	// 위에 만든 3개를 합쳐서 최종적으로 Depth Stencil Buffer View에 쓸 Resource를 생성
	m_pd3dDevice->CreateCommittedResource(														// 입력 버퍼 객체를 생성
		&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,																	// 여러가지 HEAP의 속성을 정해줄 수 있음,(기본적으로 NONE를 사용하면 된다.)HEAP이 Texture를 포함할 수 있는가, 혹은 Resource가 어댑터들 사이에서 공유가 가능한가를 나타낸다.
		&d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,														// Resource의 초기 상태값을 설정, 사용하는 설정의 종류가 많다. 해당 상태는 Depth를 Write하는 상태설정
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
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);						// Command List를 오브젝트 빌드 전에 초기화 안해주면 그려지질 않으니 조심하자

// Scene
	// Scene 객체를 생성하고 Scene에 포함될 게임 객체들을 생성
	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

//플레이어 게임 객체를 생성하고 카메라와 위치를 설정한다.
	// 이제 Airplane의 Camera Mode에 따라서 카메라를 만든다.
	CBoxPlayer* pAirplanePlayer = new CBoxPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pPlayer = pAirplanePlayer;
	m_pCamera = m_pPlayer->GetCamera();

	CGreyField* pGreyField = new CGreyField(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pField = pGreyField;

	// Scene 객체를 생성하기 위해 필요한 Graphics Command List 들을 Command Queue에 추가한다.
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommanaLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommanaLists);


// Command List
	// Graphics Command List들이 모두 실행될 때 까지 기다린다.
	WaitForGpuComplete();

	// Graphics Resource들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

// 마우스 입력
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스가 눌려지면 마우스 픽킹을 하여 선택한 게임 객체를 찾는다.
		//m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		//아무짓도 안하고싶으면 주석처리를 갈겨버리자
		if (m_pSelectedObject)
		{
			m_pSelectedObject->SelectObjectRender(false);
		}
		::SetCapture(hWnd);					// 마우스 캡쳐를 하고 
		::GetCursorPos(&m_ptOldCursorPos);	// 현재 마우스 위치를 가져온다.
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		
		::ReleaseCapture();					// 마우스 캡쳐를 해제한다.
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
// 키보드 입력
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:					//‘F1’ 키를 누르면 1인칭 카메라
		case VK_F2:					//‘F2’ 키를 누르면 스페이스-쉽 카메라
		case VK_F3:					//‘F3’ 키를 누르면 3인칭 카메라
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
		case VK_F9:					// F9키를 누르면 전체화면 모드가 된다.
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
// 윈도우 처리
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

	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);		// 현재의 전체 화면 모드를 가져옴
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);		// 현재의 전체화면 모드를 변경

	// 새로운 Description을 만들어준다.
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;					// 초당 "60"프레임으로 설정해주세요
	dxgiTargetParameters.RefreshRate.Denominator = 1;					// Numerator의 프레임 수를 "1"만큼 나눠주세요
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);				// 만든 새 디스크립션을 Swap Chain에 넣어준다.

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
	//픽킹으로 선택한 게임 객체가 있으면 키보드를 누르거나 마우스를 움직이면 게임 개체를 이동 또는 회전한다.
	// 만약 여기 내용을 바꾼다면? 픽킹을 했을 때 하는 짓이 바뀌지 않을까?
	// 픽킹을 하고 움직이면 실행되버린다. 오브젝트 지우는건 다른곳에서 한다.
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
	//키보드의 상태 정보를 반환한다. 화살표 키(‘→’, ‘←’, ‘↑’, ‘↓’)를 누르면
	//플레이어를 오른쪽/왼쪽(로컬 x-축), 앞/뒤(로컬 z-축)로 이동한다.
	//‘Page Up’과 ‘Page Down’ 키를 누르면 플레이어를 위/아래(로컬 y-축)로 이동한다.
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


		//마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다.
		//마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의 메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다.
		//그러므로 마우스가 캡쳐된 것은 마우스 버튼이 눌려진 상태를 의미한다.
		//마우스 버튼이 눌려진 상태에서 마우스를 좌우 또는 상하로 움직이면 플레이어를 x-축 또는 y-축으로 회전한다.
		if (::GetCapture() == m_hWnd)
		{
			//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
			::SetCursor(NULL);

			//현재 마우스 커서의 위치를 가져온다.
			::GetCursorPos(&ptCursorPos);

			//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;

			//마우스 커서의 위치를 마우스가 눌려졌던 위치로 설정한다.
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		//마우스 또는 키 입력이 있으면 플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				///cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다.
				//오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			//플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다).
			//이동 거리는 시간에 비례하도록 한다.
			//플레이어의 이동 속력은 (50/초)로 가정한다.
			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}
//아래는 픽킹을 한 오브젝트를 움직이려고 사용한 프로세스
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
//			//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
//			::SetCursor(NULL);
//
//			//현재 마우스 커서의 위치를 가져온다.
//			::GetCursorPos(&ptCursorPos);
//
//			//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다.
//			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
//			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
//
//			//마우스 커서의 위치를 마우스가 눌려졌던 위치로 설정한다.
//			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
//		}
//	}
//
//	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
//	{
//		if (m_pSelectedObject) // 만약 픽된 오브젝트가 있으면
//		{
//			ProcessSelectedObject(dwDirection, cxDelta, cyDelta); // 그 행위를 수행한다.
//		}
//		else
//		{
//			if (cxDelta || cyDelta)
//			{
//				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다.
//				오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
//				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
//					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
//				else
//					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
//			}
//			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다).
//			이동 거리는 시간에 비례하도록 한다.
//			플레이어의 이동 속력은 (50/초)로 가정한다.*/
//			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
//		}
//	}
//	//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다
//	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
//}

void CGameFramework::AnimateObjects()
{
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::WaitForGpuComplete()
{
	
	// CPU의 Fence값을 증가시킨다
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];

	// GPU가 Fence의 값을 설정하는 명령을 Command Queue에 추가한다.
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	// Fence의 현재 값이 설정한 값보다 작다면
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		// Fence의 현재 값이 설정한 값이 될 때까지 기다린다.
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

// 플레이어가(오브젝트 배열에 해당하지 않는 애들)은 항상 맨앞에 랜더링 되게 한다.
//#define _WITH_PLAYER_TOP


void CGameFramework::FrameAdvance()
{
	// Timer의 시간이 갱신되도록 하고 Frame Rate를 계산한다.
	m_GameTimer.Tick(0.0f);

	//사용자 입력을 처리한다.
	ProcessInput();

	//게임 세계를 애니메이션(움직이게)한다.
	AnimateObjects();

	// 명령 할당자(Command Allocator)와 명령 리스트(Command List)를 리셋한다.
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	// 현재 Render Target에 대한 Present가 끝나기를 기다린다.
	// Present가 끝난다면 Render Target Buffer의 상태는 Present상태에서 Render Target상태로 바뀔것이다.
	// D3D12_RESOURCE_STATE_PRESENT --> D3D12_RESOURCE_STATE_RENDER_TARGET
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;														// 리소스 배리어가 상태를 변환해주는 역할을 한다.
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;									// NONE - 기본동작, BEGIN_ONLY - 시작부분만 적용, END_ONLY - 끝부분만 적용
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// 현재의 Render Target에 해당하는 Descriptor의 CPU Handle(주소)을 계산
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);		// 계산하고나서 다음으로....
	
	// Depth Stencil Descriptor의 CPU Handle(주소)을 계산
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	// Render Target View와 Depth Stencil View를 OM(출력-병합 단계)에 연결
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);											// (Render Target의 서술자 개수, Render Target의 시작 주소, Descriptor의 연결이 연속적인지(True이면 연속적이다), Depth Stencil 서술자1개)
																																					// 만약 FALSE가 된다면, 주소가 아닌 배열의 형식으로 건네줘야 한다.

	// 원하는 색상으로 Render Target View를 초기화 한다.
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };											// RGBA, 해당 색상은 Azure이다.
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);														// (이 디스크립터 핸들이 나타내는 RenderTarget을, 이 색으로, Struct의 개수만큼 ,Render Target 전체(NULL)를 Clear해줘)
	// 원하는 값으로 Depth Stencil View를 초기화 한다.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);		// (이 디스크립터 핸들이 나타내는 RenderTarget을, 이러한 설정으로, Depth값은 이렇게, Stencil값은 이렇게, Depth Stencil 전체(NULL)를 Clear해줘)

	// Scene
	//씬을 렌더링한다.
	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

	// 랜더링 될 친구들을 놓는 곳//////////////////////////////////////////
	//
	
	
	
	//3인칭 카메라일 때 플레이어가 항상 보이도록 렌더링한다.
#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(
		d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0,
		NULL);
#endif // _WITH_PLAYER_TOP

	//3인칭 카메라일 때 플레이어를 렌더링한다.
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	// 이건 땅이야 바닥을 그려
	if (m_pField) m_pField->Render(m_pd3dCommandList, m_pCamera);

	//
	/////////////////////////////////////////////////////////////

	// 현재 Render Target에 대한 렌더링이 끝나기를 기다린다.
	// GPU가 Render Target (Buffer)을 더 이상 사용하지 않으면 Render Target의 상태는 Present상태로 바뀐다.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// Command List를 닫힌 상태로 만든다.
	hResult = m_pd3dCommandList->Close();

	// Command List를 Command Queue에 추가하여 실행한다.
	ID3D12CommandList* ppd3dCommandList[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandList), ppd3dCommandList);

	// GPU가 모든 Command List를 실행할 때 까지 기다린다.
	WaitForGpuComplete();

	//m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	// 이건 왜인지 모르겟는데 LabProject11에서 사라졌다.

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	// 현재의 Frame Rate를 문자열로 가져와서 주 윈도우의 타이틀로 출력한다.
	// m_pszBuffer 문자열이 "LabProject("으로 초기화가 되었음으로
	// m_pszFrameRate + 12 에서부터 Frame Rate를 문자열로 출력하여 " FPS)"문자열과 합친다.
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

