#pragma once
#include "stdafx.h"

#include "Timer.h"
#include "Scene.h"				// CScene�� ����ϱ� ����
#include "Camera.h"				// CCamera�� ����ϱ� ����
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

	// �������̽� �����͵�
	// DXGI Factory �������̽��� ���� ������, ���� ���� ������ �����(IDXGIFactory, 2, 3, 4), �׷���ī�� ���� ���� ���
	IDXGIFactory4* m_pdxgiFactory;

	// Swap Chain �������̽��� ���� ������, �ַ� Display�� �����ϴµ� ���, ���������� �ֽŹ��� �����(IDXGISwapChain, 2, 3)
	IDXGISwapChain3* m_pdxgiSwapChain;

	// Direct3D Device �������̽��� ���� ������, �ַ� Resource�� �����ϴµ� ���
	ID3D12Device* m_pd3dDevice;


	// MSAA
	bool m_bMsaa4xEnable = false;														// 4�� ��Ƽ���ø� ���� ����
	UINT m_nMsaa4xQualityLevels = 0;													// ǰ�� ����, 0�� �⺻(default)ǰ��, ���ڰ� �ö󰥼��� ��������.(2, 4, 8, 16, 32....)

	// Swap Chain Buffers
	static const UINT m_nSwapChainBuffers = 2;											// �ĸ� ������ ����
	UINT m_nSwapChainBufferIndex;														// ���� ����ü�� �ĸ� ������ �ε���


	// Render Target
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];					// Render Target Buffer���� �����ϴ� �迭
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;										// Render Target View�� ����ϴ� Descriptor Heap �������̽� ������, ���� Ÿ���� �ٲٴµ� ���
	UINT m_nRtvDescriptorIncrementSize;													// Render Target�� �� �� ����ϴ� Descriptor Heap�� ���� ũ��


	// Depth Stencil
	ID3D12Resource* m_pd3dDepthStencilBuffer;											// Depth Stencil Buffer
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;										// Depth Stencil View�� ����ϴ� Descriptor Heap �������̽� ������
	UINT m_nDsvDescriptorIncrementSize;													// Depth Stencil�� �� �� ����ϴ� Descriptor Heap�� ���� ũ��

	// Command
	ID3D12CommandQueue* m_pd3dCommandQueue;												// Command Queue �� ��ġ�� ����Ų��
	ID3D12CommandAllocator* m_pd3dCommandAllocator;										// ����� �����ϰ� �����ϱ� ���� �޸𸮸� �Ҵ��ϰ� ��������
	ID3D12GraphicsCommandList* m_pd3dCommandList;										// Command List �������̽� ������, ������ ����� �ۼ��ǰ� ���� ť�� ����

	// Pipeline State
	ID3D12PipelineState* m_pd3dPipelineState;											// �׷��Ƚ� ������������ ���� ��ü�� ���� �������̽� ������

	// Fence
	ID3D12Fence* m_pd3dFence;															// Fence�� �������ν� ������, �׷��� ����� �Ǿ����� Ȯ��
	UINT64 m_nFenceValues[m_nSwapChainBuffers];											// Fence�� ��, ���¸� �����ϰ� �����Ȳ Ȯ��, ������ �潺���� �����ϱ� ���� �����Ͽ���
	HANDLE m_hFenceEvent;																// �̺�Ʈ �ڵ�, �������� �̺�Ʈ �ڵ��� ����Ŵ, �潺 �Ϸ� �̺�Ʈ�� ��ȣ�� �� ����

	// Frame Rate
	CGameTimer m_GameTimer;																// ���� �����ӿ�ũ���� ����� Ÿ�̸�
	_TCHAR m_pszFrameRate[50];															// ������ ����Ʈ�� �� �������� ĸ�ǿ� ����ϱ� ���� ���ڿ�

	// Scene
	CScene* m_pScene;

	// Picking
	CGameObject* m_pSelectedObject = NULL;

public:
	CGameFramework();																	// ������
	~CGameFramework();																	// �Ҹ���

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);									// Framework �ʱ�ȭ(�� �����찡 �����Ǹ� ȣ��)

	void OnDestroy();

	// Pipeline
	void CreateSwapChain();																// Swap Chain ����
	void CreateRtvAndDsvDescriptorHeaps();												// Render Target View, Depth Stencil View�� ���� Descriptor Heap ����
	void CreateDirect3DDevice();														// Device ����
	void CreateCommandQueueAndList();													// Command Queue�� Command List ����

	// View
	void CreateRenderTargetViews();														// Render Target View���� ����
	void CreateDepthStencilView();														// Depth Stencil View�� ����

	// Object
	void BuildObjects();																// Object ����
	void ReleaseObjects();																// Object �Ҹ�

	// Framework�� �ٽɿ��
	void ProcessInput();																// ����� �Է�
	void AnimateObjects();																// �ִϸ��̼�
	void FrameAdvance();																// ������

	// CPU�� GPU ����ȭ
	void WaitForGpuComplete();

	// ������ �޽��� ó��
	void OnProcessingMouseMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// ���콺 �Է�
	void OnProcessingKeyboardMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// Ű���� �Է�
	LRESULT CALLBACK OnProcessingWindowMessage(
		HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);						// �޼��� ó��
	
	// Swap Chain Change	-	for the Full Screen Mode
	void ChangeSwapChainState();

	// ���� ���������� �Ѿ�� �ϴ� �Լ�
	void MoveToNextFrame();

	// Camera
	CCamera* m_pCamera = NULL;

	// Player
	CPlayer* m_pPlayer = NULL;															// �÷��̾� ��ü�� ���� �������̴�.

	// Mouse Point
	POINT m_ptOldCursorPos;																//���������� ���콺 ��ư�� Ŭ���� ���� ���콺 Ŀ���� ��ġ�̴�.

	// ���ٴ�
	CField* m_pField = NULL;

	// Picking
	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);
};

