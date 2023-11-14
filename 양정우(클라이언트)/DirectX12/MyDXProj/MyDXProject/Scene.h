#pragma once
#include "stdafx.h"

#include "Shader.h"
#include "Timer.h"


class CScene
{
public:
	CScene();
	~CScene();

	// 키보드와 마우스 입력 처리
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	// Objects
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);	// 버전 업그레이드, 파라메터 추가 ID3D12GraphicsCommandList* pd3dCommandList
	void ReleaseObjects();
	void ReleaseAllObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	// Move
	bool ProcessInput(UCHAR* pKeysBuffer);// 버전 업그레이드, 파라메터 추가 UCHAR* pKeysBuffer
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void ReleaseUploadBuffers();

	// Graphics Rood Signature를 생성
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

// Picking
	//씬의 모든 게임 객체들에 대한 마우스 픽킹을 수행한다.
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);

protected:
	//배치(Batch) 처리를 하기 위하여 Scene을 Shader들의 리스트로 표현한다.
	CObjectsShader* m_pShaders = NULL;
	int m_nShaders = 0;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;				// 루트 시그너쳐를 나타내는 인터페이스 포인터이다.

};