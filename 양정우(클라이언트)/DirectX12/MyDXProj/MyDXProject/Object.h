#pragma once
#include "stdafx.h"
#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

private:
	int m_nReferences = 0;			// ������Ʈ�� ����

public:
	// Object Count;
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	XMFLOAT4X4 m_xmf4x4World;
	CMesh* m_pMesh = NULL;

	CShader* m_pShader = NULL;

	bool m_bObjectRender = false;

public:
	void ReleaseUploadBuffers();

// Set
	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);

// Animate
	virtual void Animate(float fTimeElapsed);

// Render
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

// Rotate
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

// Constant Buffer
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);			// ��� ���۸� ����
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);										// ��� ������ ������ �����Ѵ�.
	virtual void ReleaseShaderVariables();

// Object Position
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

// Object Vector
	// ���� ��ü�� ���� ��ȯ ��Ŀ���
	XMFLOAT3 GetPosition();			//  ��ġ ���Ϳ� 
	XMFLOAT3 GetLook();				// ����(x-��, 
	XMFLOAT3 GetUp();				// y-��, 
	XMFLOAT3 GetRight();			// z-��) ���͸� ��ȯ�Ѵ�

// Object Local Move
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

// Object Rotate
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

// Frustum
	// ���� ��ü�� ī�޶�� ���̴� ���� �˻��Ѵ�.
	bool IsVisible(CCamera* pCamera = NULL);

// Picking
	//�� ��ǥ���� ��ŷ ������ �����Ѵ�. 
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	//ī�޶� ��ǥ���� �� ���� ���� �� ��ǥ���� ��ŷ ������ �����ϰ� ��ü���� ������ �˻��Ѵ�. 
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

// ������Ʈ�� �׸��� ������ ����
	// ������Ʈ ������ ���θ� �����Ѵ�
	void SelectObjectRender(bool isObjectRender);
	// ���� ������Ʈ�� ������ ���θ� �ҷ��´�.
	bool IsObjectRender();
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	// Set
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	// Animate
	virtual void Animate(float fTimeElapsed);
};
