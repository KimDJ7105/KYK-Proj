#include "stdafx.h"
#include "Shader.h"
#include "Field.h"


// CField
CField::CField()
{
	// Vector
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// Rotate Value
	m_fPitch = 0.0f;
	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	// Update Callback
	m_pFieldUpdatedContext = NULL;
}

CField::~CField()
{
	ReleaseShaderVariables();
}


// Rotate Field
void CField::Rotate(float x, float y, float z)
{
}

// Update (Position, Rotate data)
void CField::Update(float fTimeElapsed)
{

}

void CField::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);					// 새로 만들 필요없이 Object내에 이미 존재하는 함수를 쓴다.
}

void CField::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();												// 새로 만들 필요없이 Object내에 이미 존재하는 함수를 쓴다.
}

void CField::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);								// 새로 만들 필요없이 Object내에 이미 존재하는 함수를 쓴다.
}


// for World Transform
void CField::OnPrepareRender()
{
	/*플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	플레이어의 Right 벡터가 월드 변환 행렬의 첫 번째 행 벡터,
	Up 벡터가 두 번째 행 벡터,
	Look 벡터가 세 번째 행 벡터,
	플레이어의 위치 벡터가 네 번째 행 벡터가 된다.*/

	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;

	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;

	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;

	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}

void CField::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
	CGameObject::Render(pd3dCommandList, pCamera);
}

// CField - Airplane
CGreyField::CGreyField(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//땅 메쉬를 생성한다.
	CMesh* pGreyField = new CModelMesh(
		pd3dDevice,
		pd3dCommandList,
		"Models/ProtoMap.bin"
	);
	SetMesh(pGreyField);

	SelectObjectRender(true);

	//땅을 위한 셰이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//땅 메쉬를 렌더링할 때 사용할 셰이더를 생성한다.
	CFieldShader* pShader = new CFieldShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}

CGreyField::~CGreyField()
{
}


void CGreyField::OnPrepareRender()
{
	CField::OnPrepareRender();

	//비행기 모델을 그리기 전에 x-축으로 90도 회전한다.
	/*3인칭 카메라일 때 플레이어 메쉬를 로컬 x-축을 중심으로 +90도 회전하고 렌더링한다.
	왜냐하면 비행기 모델 메쉬는 다음 그림과 같이 y-축 방향이 비행기의 앞쪽이 되도록 모델링이 되었기 때문이다.
	그리고 이 메쉬를 카메라의 z- 축 방향으로 향하도록 그릴 것이기 때문이다.*/
	//XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	//m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	// 근데 이건 땅바닥 만드는거라 그냥 주석처리 했다.

	//땅의 위치를 설정한다.
	SetPosition(XMFLOAT3(0.0f, -12.0f, 0.0f));
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(270.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}