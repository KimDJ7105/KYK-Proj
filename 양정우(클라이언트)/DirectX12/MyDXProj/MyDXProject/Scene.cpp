#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}
// Scene������  Mouse�� Keyboard �޽����� ó��
bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return (false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return (false);
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	// Root Signature�� ���� Parameter�� Set���ش�.
	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	// 0�� Parameter
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;				// 32bit ����� ����
	pd3dRootParameters[0].Constants.Num32BitValues = 16;											// 16���� 32bit���� ���Եȴ�.
	pd3dRootParameters[0].Constants.ShaderRegister = 0;												// ��𿡼� ������ �Ǿ��ִ°�. 0�� Parameter�̱� ������ 1�� Register�� ����
	pd3dRootParameters[0].Constants.RegisterSpace = 0;												// Register�� ������ �����Ѵ�.
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;						// � Shader���� ����� �����Ѱ�. (���� Vertex���� ��밡������ �����Ǿ��ִ�.)
	// 1�� Parameter
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;				// 0���� ����
	pd3dRootParameters[1].Constants.Num32BitValues = 32;											// 32���� 32bit���� ���Եȴ�.
	pd3dRootParameters[1].Constants.ShaderRegister = 1;												// ��� ������ �Ǿ��ִ°�. 1�� Parameter�̱� ������ 1�� Register�� ����
	pd3dRootParameters[1].Constants.RegisterSpace = 0;												// 0���� ����
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;						// 0���� ����


	// Root Signature�� ���� Flag�� Set���ش�.
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |								// �Է� ���̾ƿ��� ����Ѵ�.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |									// Hull Shader���� Root Parameter�� Access �� �� ������ �Ѵ�.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |									// Domain Shader���� Root Parameter�� Access �� �� ������ �Ѵ�.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |								// Geometry Shader���� Root Parameter�� Access �� �� ������ �Ѵ�.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;									// Pixel Shader���� Root Parameter�� Access �� �� ������ �Ѵ�.
																									// ��? �� ���� �Ⱦ��� �����ϱ�~

	// Root Signature�� �����Ѵ�
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);								// ���Ե� �Ķ������ ���� ����, ������ �����Ķ������ ������ �ҷ��´�
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;											// �迭�� ���� ������ ����
	d3dRootSignatureDesc.NumStaticSamplers = 0;														// ���� ���÷��� ����, 0�̹Ƿ� ����
	d3dRootSignatureDesc.pStaticSamplers = NULL;													// ���� ���÷� �迭�� ������
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;												// ������ ���� Flag ������ �����´�.
	

	ID3DBlob* pd3dSignatureBlob = NULL;																// ����ϱ� ���� �ʱ�ȭ
	ID3DBlob* pd3dErrorBlob = NULL;																	// ����ϱ� ���� �ʱ�ȭ
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	pd3dDevice->CreateRootSignature(
		0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature),
		(void**)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return (pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::GetGraphicsRootSignature()
{
	return(m_pd3dGraphicsRootSignature);
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	if (!pCamera) return(NULL);

	XMFLOAT4X4 xmf4x4View = pCamera->GetViewMatrix();
	XMFLOAT4X4 xmf4x4Projection = pCamera->GetProjectionMatrix();
	D3D12_VIEWPORT d3dViewport = pCamera->GetViewport();

	XMFLOAT3 xmf3PickPosition;
	/*ȭ�� ��ǥ���� �� (xClient, yClient)�� ȭ�� ��ǥ ��ȯ�� ����ȯ�� ���� ��ȯ�� ����ȯ�� �Ѵ�. �� ����� ī�޶�
	��ǥ���� ���̴�. ���� ����� ī�޶󿡼� z-������ �Ÿ��� 1�̹Ƿ� z-��ǥ�� 1�� �����Ѵ�.*/
	xmf3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
	CGameObject* pIntersectedObject = NULL, * pNearestObject = NULL;

	//���̴��� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ͽ� ī�޶�� ���� ����� ���� ��ü�� ���Ѵ�.
	for (int i = 0; i < m_nShaders; i++)
	{
		pIntersectedObject = m_pShaders[i].PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if (pIntersectedObject && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = pIntersectedObject;
		}
	}
	return(pNearestObject);
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	// Graphics Root Signature�� �����
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nShaders = 1;															// ����� Shader�� ����, �� 1��!
	m_pShaders = new CObjectsShader[m_nShaders];							// �� ����� Shader �޸� �Ҵ�.
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);	// Shader�� ������ְ�
	m_pShaders[0].BuildObjects(pd3dDevice, pd3dCommandList);				// Build ���ش�.
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();
	}
	if (m_pShaders) delete[] m_pShaders;
}

void CScene::ReleaseAllObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();										// �Ʊ� ���� ������Ʈ���� �迭�� �°� ���� ����������.				
	}
	m_pShaders = new CObjectsShader[m_nShaders];							// �� ����� Shader �޸� �Ҵ�.
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);	// Shader�� ������ְ�
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return (false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].AnimateObjects(fTimeElapsed);
	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// Camera�� ���� Viewport�� ScissorRect�� Set���ش�.
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	// Graphics Root Signature�� Pipeline�� ����(����)�Ѵ�
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->UpdateShaderVariables(pd3dCommandList);									// �̰� ��� ȭ�鿡 �ﰢ�� ȸ���ϴ°� ������ �ʾҾ���.
	
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].Render(pd3dCommandList, pCamera);
	}
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseUploadBuffers();		// ������ �����ϴ� ���� �ƴ� Shader �� �����ϰ� ����� �ǰ� �ȴ�.
}
