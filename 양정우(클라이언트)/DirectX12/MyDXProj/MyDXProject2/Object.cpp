//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
	m_nReferences = 1;
}

CMaterial::~CMaterial()
{
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_xmf4AlbedoColor, 0);
	pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_xmf4SpecularColor, 4);
	pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_xmf4EmissionColor, 8);
}

void CMaterial::ReleaseShaderVariables()
{
}

void CMaterial::ReleaseUploadBuffers()
{
}

void CMaterial::LoadMaterialFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pFile)
{
	char pstrToken[256] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<AlbedoColor>
	nReads = (UINT)::fread(&m_xmf4AlbedoColor, sizeof(float), 4, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<EmissiveColor>
	nReads = (UINT)::fread(&m_xmf4EmissionColor, sizeof(float), 4, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<SpecularColor>
	nReads = (UINT)::fread(&m_xmf4SpecularColor, sizeof(float), 4, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Glossiness>
	float fGlossiness;
	nReads = (UINT)::fread(&fGlossiness, sizeof(float), 1, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Smoothness>
	float fSmoothness;
	nReads = (UINT)::fread(&fSmoothness, sizeof(float), 1, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Metallic>
	float fMetallic;
	nReads = (UINT)::fread(&fMetallic, sizeof(float), 1, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<SpecularHighlight>
	float fSpecularHighlight;
	nReads = (UINT)::fread(&fSpecularHighlight, sizeof(float), 1, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<GlossyReflection>
	float fGlossyReflection;
	nReads = (UINT)::fread(&fGlossyReflection, sizeof(float), 1, pFile);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaterials)
{
	m_nMaterials = nMaterials;
	m_ppMaterials = NULL;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial * [m_nMaterials];
		for (UINT i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}

	m_xmf4x4World = Matrix4x4::Identity();

	CBoundingBoxMesh* pBoundingBoxMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList);
	SetBoundingBoxMesh(pBoundingBoxMesh);
}
 
CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_pMesh) m_pMesh->Release();
	m_pMesh = NULL;

	if (m_pBoundingBoxMesh) m_pBoundingBoxMesh->Release();
	m_pBoundingBoxMesh = NULL;

	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		delete[] m_ppMaterials;
	}

	if (m_pShader) m_pShader->Release();
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (pMesh) pMesh->AddRef();
}

void CGameObject::SetBoundingBoxMesh(CBoundingBoxMesh* pMesh)
{
	if (m_pBoundingBoxMesh) m_pBoundingBoxMesh->Release();
	m_pBoundingBoxMesh = pMesh;
	if (pMesh) pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetAlbedoColor(xmf4Color);
	}
}

void CGameObject::SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetEmissionColor(xmf4Color);
	}
}

void CGameObject::SetMaterial(UINT nIndex, CMaterial *pMaterial)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release();
		m_ppMaterials[nIndex] = pMaterial;
		if (pMaterial) pMaterial->AddRef();
	}
}

void CGameObject::SetMaterial(UINT nIndex, UINT nReflection)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->m_nMaterial = nReflection;
	}
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void **)&m_pcbMappedGameObject);
}

void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseShaderVariables();
	if (m_pShader) m_pShader->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pcbMappedGameObject) XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	if (m_pd3dcbGameObject)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbGameObject->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
	}
}

void CGameObject::UpdateBoundingBox()
{
	OnPrepareRender();
	if (m_pMesh)
	{
		m_pMesh->m_xmBoundingBox.Transform(m_xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_xmBoundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmBoundingBox.Orientation)));
	}
}

void CGameObject::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pBoundingBoxMesh)
	{
		m_pBoundingBoxMesh->UpdateVertexPosition(&m_xmBoundingBox);
		m_pBoundingBoxMesh->Render(pd3dCommandList);
	}
}

void CGameObject::Animate(float fElapsedTime)
{
	UpdateBoundingBox();
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pMesh && m_ppMaterials)
	{
		if (m_pShader)
		{
			m_pShader->Render(pd3dCommandList, pCamera);
			m_pShader->UpdateShaderVariables(pd3dCommandList);
		}
		UpdateShaderVariables(pd3dCommandList);

		m_pMesh->OnPreRender(pd3dCommandList);
		for (UINT i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
			m_pMesh->Render(pd3dCommandList, i);
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	if (m_pShader) m_pShader->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::LoadGameObjectFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
{
	FILE *pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<GameObject>:
	::ReadUnityBinaryString(pFile, m_pstrName, &nStrLength); //"GameObjectName"

	nReads = (UINT)::fread(&m_xmf4x4World, sizeof(float), 16, pFile);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Materials>:
	nReads = (UINT)::fread(&m_nMaterials, sizeof(int), 1, pFile);

	m_ppMaterials = new CMaterial * [m_nMaterials];

	for (UINT k = 0; k < m_nMaterials; k++)
	{
		m_ppMaterials[k] = new CMaterial();
		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Material>:
		::ReadUnityBinaryString(pFile, m_ppMaterials[k]->m_pstrName, &nStrLength); //"Material Name"

		m_ppMaterials[k]->LoadMaterialFromFile(pd3dDevice, pd3dCommandList, pFile);
		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Material>
	}
	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Materials>

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Mesh>:
	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"Mesh Name"

	char pstrFilePath[64] = { '\0' };
	strcpy_s(pstrFilePath, 64, pstrFileName);
	char* pSlash = strrchr(pstrFilePath, '/');
	*(pSlash + 1) = '\0';
	strncat_s(pstrToken, 64, ".bin", 4);
	strncat_s(pstrFilePath, 64, pstrToken, nStrLength + 4);

	CMesh *pMesh = new CMeshFromFile(pd3dDevice, pd3dCommandList, pstrFilePath);
	SetMesh(pMesh);

	CBoundingBoxMesh* pBoundingBoxMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList);
	SetBoundingBoxMesh(pBoundingBoxMesh);

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Mesh>

	::fclose(pFile);
}
