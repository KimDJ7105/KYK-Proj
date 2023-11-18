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
	int m_nReferences = 0;			// 오브젝트의 개수

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
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);			// 상수 버퍼를 생성
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);										// 상수 버퍼의 내용을 갱신한다.
	virtual void ReleaseShaderVariables();

// Object Position
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

// Object Vector
	// 게임 객체의 월드 변환 행렬에서
	XMFLOAT3 GetPosition();			//  위치 벡터와 
	XMFLOAT3 GetLook();				// 방향(x-축, 
	XMFLOAT3 GetUp();				// y-축, 
	XMFLOAT3 GetRight();			// z-축) 벡터를 반환한다

// Object Local Move
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

// Object Rotate
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

// Frustum
	// 게임 객체가 카메라게 보이는 가를 검사한다.
	bool IsVisible(CCamera* pCamera = NULL);

// Picking
	//모델 좌표계의 픽킹 광선을 생성한다. 
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	//카메라 좌표계의 한 점에 대한 모델 좌표계의 픽킹 광선을 생성하고 객체와의 교차를 검사한다. 
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

// 오브젝트를 그릴지 말지의 여부
	// 오브젝트 랜더링 여부를 결정한다
	void SelectObjectRender(bool isObjectRender);
	// 현재 오브젝트의 랜더링 여부를 불러온다.
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
