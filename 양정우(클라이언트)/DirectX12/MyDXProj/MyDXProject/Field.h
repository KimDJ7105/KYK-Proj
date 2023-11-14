#pragma once
#include "stdafx.h"
#include "Object.h"


class CField : public CGameObject
{
protected:
	// Vector
	XMFLOAT3 m_xmf3Position;			// Position Vector
	XMFLOAT3 m_xmf3Right;				// x - ��(Right) Vector
	XMFLOAT3 m_xmf3Up;					// y - ��(Up) Vector
	XMFLOAT3 m_xmf3Look;				// z - ��(Look) Vector

	// Rotate Value
	float m_fPitch;						//  x-��(Right)
	float m_fYaw;						//  y-��(Up)
	float m_fRoll;						// z-��(Look)

	// Update Callback
	LPVOID m_pFieldUpdatedContext;		// Player

public:
	CField();
	virtual ~CField();

	// Vector
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	// Rotate Value
	float GetPitch() { return(m_fPitch); }
	float GetYaw() { return(m_fYaw); }
	float GetRoll() { return(m_fRoll); }

	// Rotate Field
	void Rotate(float x, float y, float z);

	// Update (Position, Rotate data)
	void Update(float fTimeElapsed);

	// Update Callback
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }								// Field
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pFieldUpdatedContext = pContext; }	// Field

	// Shader Buffer(�ٸ������� ���� ����. �ٵ� �������� �ּ��̸��� �ٸ��� ��������)
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	// for World Transform
	virtual void OnPrepareRender();

	// if Third Person Mode, Then Render Player Mesh
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

// CGreyField
class CGreyField : public CField
{
public:
	CGreyField(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CGreyField();

	virtual void OnPrepareRender();
};