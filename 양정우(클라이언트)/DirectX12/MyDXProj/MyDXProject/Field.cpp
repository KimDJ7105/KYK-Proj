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
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);					// ���� ���� �ʿ���� Object���� �̹� �����ϴ� �Լ��� ����.
}

void CField::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();												// ���� ���� �ʿ���� Object���� �̹� �����ϴ� �Լ��� ����.
}

void CField::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);								// ���� ���� �ʿ���� Object���� �̹� �����ϴ� �Լ��� ����.
}


// for World Transform
void CField::OnPrepareRender()
{
	/*�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	�÷��̾��� Right ���Ͱ� ���� ��ȯ ����� ù ��° �� ����,
	Up ���Ͱ� �� ��° �� ����,
	Look ���Ͱ� �� ��° �� ����,
	�÷��̾��� ��ġ ���Ͱ� �� ��° �� ���Ͱ� �ȴ�.*/

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
	//�� �޽��� �����Ѵ�.
	CMesh* pGreyField = new CModelMesh(
		pd3dDevice,
		pd3dCommandList,
		"Models/ProtoMap.bin"
	);
	SetMesh(pGreyField);

	SelectObjectRender(true);

	//���� ���� ���̴� ������ �����Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//�� �޽��� �������� �� ����� ���̴��� �����Ѵ�.
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

	//����� ���� �׸��� ���� x-������ 90�� ȸ���Ѵ�.
	/*3��Ī ī�޶��� �� �÷��̾� �޽��� ���� x-���� �߽����� +90�� ȸ���ϰ� �������Ѵ�.
	�ֳ��ϸ� ����� �� �޽��� ���� �׸��� ���� y-�� ������ ������� ������ �ǵ��� �𵨸��� �Ǿ��� �����̴�.
	�׸��� �� �޽��� ī�޶��� z- �� �������� ���ϵ��� �׸� ���̱� �����̴�.*/
	//XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	//m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	// �ٵ� �̰� ���ٴ� ����°Ŷ� �׳� �ּ�ó�� �ߴ�.

	//���� ��ġ�� �����Ѵ�.
	SetPosition(XMFLOAT3(0.0f, -12.0f, 0.0f));
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(270.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}