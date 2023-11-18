#pragma once
#include "stdafx.h"

#include "Timer.h"
#include "Scene.h"				// CScene을 사용하기 위함
#include "Camera.h"				// CCamera를 사용하기 위함
#include "Player.h"
#include "Field.h"


class CGameFramework
{

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// Client Window Size
	int m_nWndClientWidth;
	int m_nWndClientHeight;

	// 인터페이스 포인터들
	// DXGI Factory 인터페이스에 대한 포인터, 가장 높은 버전을 사용중(IDXGIFactory, 2, 3, 4), 그래픽카드 고르기 역할 등등
	IDXGIFactory4* m_pdxgiFactory;

	// Swap Chain 인터페이스에 대한 포인터, 주로 Display를 제어하는데 사용, 마찬가지로 최신버전 사용중(IDXGISwapChain, 2, 3)
	IDXGISwapChain3* m_pdxgiSwapChain;

	// Direct3D Device 인터페이스에 대한 포인터, 주로 Resource를 생성하는데 사용
	ID3D12Device* m_pd3dDevice;


	// MSAA
	bool m_bMsaa4xEnable = false;														// 4배 멀티샘플링 실행 여부
	UINT m_nMsaa4xQualityLevels = 0;													// 품질 레벨, 0은 기본(default)품질, 숫자가 올라갈수록 높아진다.(2, 4, 8, 16, 32....)

	// Swap Chain Buffers
	static const UINT m_nSwapChainBuffers = 2;											// 후면 버퍼의 개수
	UINT m_nSwapChainBufferIndex;														// 현재 스왑체인 후면 버퍼의 인덱스


	// Render Target
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];					// Render Target Buffer들을 관리하는 배열
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;										// Render Target View에 사용하는 Descriptor Heap 인터페이스 포인터, 렌더 타겟을 바꾸는데 사용
	UINT m_nRtvDescriptorIncrementSize;													// Render Target을 할 때 사용하는 Descriptor Heap의 원소 크기


	// Depth Stencil
	ID3D12Resource* m_pd3dDepthStencilBuffer;											// Depth Stencil Buffer
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;										// Depth Stencil View에 사용하는 Descriptor Heap 인터페이스 포인터
	UINT m_nDsvDescriptorIncrementSize;													// Depth Stencil을 할 때 사용하는 Descriptor Heap의 원소 크기

	// Command
	ID3D12CommandQueue* m_pd3dCommandQueue;												// Command Queue 의 위치를 가리킨다
	ID3D12CommandAllocator* m_pd3dCommandAllocator;										// 명령을 생성하고 구성하기 위해 메모리를 할당하고 관리해줌
	ID3D12GraphicsCommandList* m_pd3dCommandList;										// Command List 인터페이스 포인터, 실제로 명령이 작성되고 실행 큐에 제출

	// Pipeline State
	ID3D12PipelineState* m_pd3dPipelineState;											// 그래픽스 파이프라인의 상태 객체에 대한 인터페이스 포인터

	// Fence
	ID3D12Fence* m_pd3dFence;															// Fence의 인터페인스 포인터, 그래픽 명령이 되었는지 확인
	UINT64 m_nFenceValues[m_nSwapChainBuffers];											// Fence의 값, 상태를 추적하고 진행상황 확인, 현재의 펜스값을 관리하기 위해 수정하였음
	HANDLE m_hFenceEvent;																// 이벤트 핸들, 윈도우의 이벤트 핸들을 가리킴, 펜스 완료 이벤트의 신호로 잘 사용됨

	// Frame Rate
	CGameTimer m_GameTimer;																// 게임 프레임워크에서 사용할 타이머
	_TCHAR m_pszFrameRate[50];															// 프레임 레이트를 주 윈도우의 캡션에 출력하기 위한 문자열

	// Scene
	CScene* m_pScene;

	// Picking
	CGameObject* m_pSelectedObject = NULL;

public:
	CGameFramework();																	// 생성자
	~CGameFramework();																	// 소멸자

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);									// Framework 초기화(주 윈도우가 생성되면 호출)

	void OnDestroy();

	// Pipeline
	void CreateSwapChain();																// Swap Chain 생성
	void CreateRtvAndDsvDescriptorHeaps();												// Render Target View, Depth Stencil View에 대한 Descriptor Heap 생성
	void CreateDirect3DDevice();														// Device 생성
	void CreateCommandQueueAndList();													// Command Queue와 Command List 생성

	// View
	void CreateRenderTargetViews();														// Render Target View들을 생성
	void CreateDepthStencilView();														// Depth Stencil View를 생성

	// Object
	void BuildObjects();																// Object 생성
	void ReleaseObjects();																// Object 소멸

	// Framework의 핵심요소
	void ProcessInput();																// 사용자 입력
	void AnimateObjects();																// 애니메이션
	void FrameAdvance();																// 랜더링

	// CPU와 GPU 동기화
	void WaitForGpuComplete();

	// 윈도우 메시지 처리
	void OnProcessingMouseMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// 마우스 입력
	void OnProcessingKeyboardMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// 키보드 입력
	LRESULT CALLBACK OnProcessingWindowMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// 메세지 처리
	
	// Swap Chain Change	-	for the Full Screen Mode
	void ChangeSwapChainState();

	// 다음 프레임으로 넘어가게 하는 함수
	void MoveToNextFrame();

	// Camera
	CCamera* m_pCamera = NULL;

	// Player
	CPlayer* m_pPlayer = NULL;															// 플레이어 객체에 대한 포인터이다.

	// Mouse Point
	POINT m_ptOldCursorPos;																//마지막으로 마우스 버튼을 클릭할 때의 마우스 커서의 위치이다.

	// 땅바닥
	CField* m_pField = NULL;

	// Picking
	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);
};

