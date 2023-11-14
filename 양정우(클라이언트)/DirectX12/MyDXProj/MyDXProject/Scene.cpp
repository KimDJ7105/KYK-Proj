#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}
// Scene에서의  Mouse와 Keyboard 메시지를 처리
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

	// Root Signature에 쓰일 Parameter를 Set해준다.
	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	// 0번 Parameter
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;				// 32bit 상수로 설정
	pd3dRootParameters[0].Constants.Num32BitValues = 16;											// 16개의 32bit값이 포함된다.
	pd3dRootParameters[0].Constants.ShaderRegister = 0;												// 어디에서 연결이 되어있는가. 0번 Parameter이기 때문에 1번 Register에 연결
	pd3dRootParameters[0].Constants.RegisterSpace = 0;												// Register의 공간을 지정한다.
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;						// 어떤 Shader에서 사용이 가능한가. (현재 Vertex에서 사용가능으로 설정되어있다.)
	// 1번 Parameter
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;				// 0번과 동일
	pd3dRootParameters[1].Constants.Num32BitValues = 32;											// 32개의 32bit값이 포함된다.
	pd3dRootParameters[1].Constants.ShaderRegister = 1;												// 어디에 연결이 되어있는가. 1번 Parameter이기 때문에 1번 Register에 연결
	pd3dRootParameters[1].Constants.RegisterSpace = 0;												// 0번과 동일
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;						// 0번과 동일


	// Root Signature에 쓰일 Flag를 Set해준다.
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |								// 입력 레이아웃을 허용한다.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |									// Hull Shader에서 Root Parameter에 Access 할 수 없도록 한다.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |									// Domain Shader에서 Root Parameter에 Access 할 수 없도록 한다.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |								// Geometry Shader에서 Root Parameter에 Access 할 수 없도록 한다.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;									// Pixel Shader에서 Root Parameter에 Access 할 수 없도록 한다.
																									// 왜? 다 지금 안쓰고 있으니까~

	// Root Signature를 생성한다
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);								// 포함될 파라메터의 개수 지정, 위에서 만든파라메터의 개수를 불러온다
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;											// 배열에 대한 포인터 지정
	d3dRootSignatureDesc.NumStaticSamplers = 0;														// 정적 샘플러의 개수, 0이므로 없음
	d3dRootSignatureDesc.pStaticSamplers = NULL;													// 정적 샘플러 배열의 포인터
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;												// 위에서 만든 Flag 설정을 가져온다.
	

	ID3DBlob* pd3dSignatureBlob = NULL;																// 사용하기 전에 초기화
	ID3DBlob* pd3dErrorBlob = NULL;																	// 사용하기 전에 초기화
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
	/*화면 좌표계의 점 (xClient, yClient)를 화면 좌표 변환의 역변환과 투영 변환의 역변환을 한다. 그 결과는 카메라
	좌표계의 점이다. 투영 평면이 카메라에서 z-축으로 거리가 1이므로 z-좌표는 1로 설정한다.*/
	xmf3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
	CGameObject* pIntersectedObject = NULL, * pNearestObject = NULL;

	//셰이더의 모든 게임 객체들에 대한 마우스 픽킹을 수행하여 카메라와 가장 가까운 게임 객체를 구한다.
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
	// Graphics Root Signature를 만든다
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nShaders = 1;															// 사용할 Shader의 개수, 단 1개!
	m_pShaders = new CObjectsShader[m_nShaders];							// 그 사용할 Shader 메모리 할당.
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);	// Shader를 만들어주고
	m_pShaders[0].BuildObjects(pd3dDevice, pd3dCommandList);				// Build 해준다.
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
		m_pShaders[i].ReleaseObjects();										// 아까 만든 오브젝트들을 배열에 맞게 전부 지워버린다.				
	}
	m_pShaders = new CObjectsShader[m_nShaders];							// 그 사용할 Shader 메모리 할당.
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);	// Shader를 만들어주고
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
	// Camera에 대한 Viewport와 ScissorRect를 Set해준다.
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	// Graphics Root Signature를 Pipeline에 연결(설정)한다
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->UpdateShaderVariables(pd3dCommandList);									// 이게 없어서 화면에 삼각형 회전하는게 나오지 않았었다.
	
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].Render(pd3dCommandList, pCamera);
	}
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseUploadBuffers();		// 일일히 제거하는 것이 아닌 Shader 만 쓱싺하고 지우면 되게 된다.
}
