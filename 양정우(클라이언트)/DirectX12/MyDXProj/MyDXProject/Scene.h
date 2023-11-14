#pragma once
#include "stdafx.h"

#include "Shader.h"
#include "Timer.h"


class CScene
{
public:
	CScene();
	~CScene();

	// Ű����� ���콺 �Է� ó��
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	// Objects
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);	// ���� ���׷��̵�, �Ķ���� �߰� ID3D12GraphicsCommandList* pd3dCommandList
	void ReleaseObjects();
	void ReleaseAllObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	// Move
	bool ProcessInput(UCHAR* pKeysBuffer);// ���� ���׷��̵�, �Ķ���� �߰� UCHAR* pKeysBuffer
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void ReleaseUploadBuffers();

	// Graphics Rood Signature�� ����
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

// Picking
	//���� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�.
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);

protected:
	//��ġ(Batch) ó���� �ϱ� ���Ͽ� Scene�� Shader���� ����Ʈ�� ǥ���Ѵ�.
	CObjectsShader* m_pShaders = NULL;
	int m_nShaders = 0;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;				// ��Ʈ �ñ׳��ĸ� ��Ÿ���� �������̽� �������̴�.

};