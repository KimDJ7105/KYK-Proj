//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	if (m_pnSubSetStartIndices) delete[] m_pnSubSetStartIndices;
	for (UINT i = 0; i < m_nSubsets; i++) if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
	if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;

	if (m_pd3dVertexBufferViews) delete[] m_pd3dVertexBufferViews;

	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	for (UINT i = 0; i < m_nSubsets; i++) if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
	if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;

	if (m_pd3dIndexBufferViews) delete[] m_pd3dIndexBufferViews;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if (m_ppd3dSubSetIndexUploadBuffers)
	{
		for (UINT i = 0; i < m_nSubsets; i++) if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		delete[] m_ppd3dSubSetIndexUploadBuffers;
	}
	m_ppd3dSubSetIndexUploadBuffers = NULL;
};

void CMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);
}

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
//
CMeshFromFile::CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName) : CMesh(pd3dDevice, pd3dCommandList)
{
	if (pstrFileName) LoadMeshFromFile(pd3dDevice, pd3dCommandList, pstrFileName);
}

CMeshFromFile::~CMeshFromFile()
{
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}

void CMeshFromFile::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
};

void CMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSubset)
{
	if (m_nSubsets > 0)
	{
		pd3dCommandList->IASetIndexBuffer(&m_pd3dIndexBufferViews[nSubset]);
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubset], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMeshFromFile::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[256] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	while (1)
	{
		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength);

		if (!strcmp(pstrToken, "<BoundingBox>:"))
		{
			nReads = (UINT)::fread(&m_xmBoundingBox.Center, sizeof(float), 3, pFile);
			nReads = (UINT)::fread(&m_xmBoundingBox.Extents, sizeof(float), 3, pFile);
			m_xmBoundingBox.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Positions, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Normals, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
			m_pnIndices = new UINT[m_nIndices];
			nReads = (UINT)::fread(m_pnIndices, sizeof(UINT), m_nIndices, pFile);
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&m_nSubsets, sizeof(int), 1, pFile);

			m_pnSubSetIndices = new UINT[m_nSubsets];
			m_pnSubSetStartIndices = new UINT[m_nSubsets];
			m_ppnSubSetIndices = new UINT * [m_nSubsets];

			for (UINT i = 0; i < m_nSubsets; i++)
			{
				nReads = (UINT)::fread(&m_pnSubSetStartIndices[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_pnSubSetIndices[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
				m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
				nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pFile);
			}

			break;
		}
	}

	::fclose(pFile);

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_nVertexBufferViews = 2;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dVertexBufferViews[1].BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[1].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[1].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubsets];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubsets];
	m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubsets];

	for (UINT i = 0; i < m_nSubsets; i++)
	{
		m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

		m_pd3dIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
		m_pd3dIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		m_pd3dIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CBoundingBoxMesh::CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 12 * 2;
	m_nStride = sizeof(XMFLOAT3);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nStride * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dPositionBuffer->Map(0, NULL, (void**)&m_pcbMappedPositions);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CBoundingBoxMesh::~CBoundingBoxMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Unmap(0, NULL);
}

void CBoundingBoxMesh::UpdateVertexPosition(BoundingOrientedBox* pxmBoundingBox)
{
	XMFLOAT3 xmf3Corners[8];
	pxmBoundingBox->GetCorners(xmf3Corners);

	int i = 0;

	m_pcbMappedPositions[i++] = xmf3Corners[0];
	m_pcbMappedPositions[i++] = xmf3Corners[1];

	m_pcbMappedPositions[i++] = xmf3Corners[1];
	m_pcbMappedPositions[i++] = xmf3Corners[2];

	m_pcbMappedPositions[i++] = xmf3Corners[2];
	m_pcbMappedPositions[i++] = xmf3Corners[3];

	m_pcbMappedPositions[i++] = xmf3Corners[3];
	m_pcbMappedPositions[i++] = xmf3Corners[0];

	m_pcbMappedPositions[i++] = xmf3Corners[4];
	m_pcbMappedPositions[i++] = xmf3Corners[5];

	m_pcbMappedPositions[i++] = xmf3Corners[5];
	m_pcbMappedPositions[i++] = xmf3Corners[6];

	m_pcbMappedPositions[i++] = xmf3Corners[6];
	m_pcbMappedPositions[i++] = xmf3Corners[7];

	m_pcbMappedPositions[i++] = xmf3Corners[7];
	m_pcbMappedPositions[i++] = xmf3Corners[4];

	m_pcbMappedPositions[i++] = xmf3Corners[0];
	m_pcbMappedPositions[i++] = xmf3Corners[4];

	m_pcbMappedPositions[i++] = xmf3Corners[1];
	m_pcbMappedPositions[i++] = xmf3Corners[5];

	m_pcbMappedPositions[i++] = xmf3Corners[2];
	m_pcbMappedPositions[i++] = xmf3Corners[6];

	m_pcbMappedPositions[i++] = xmf3Corners[3];
	m_pcbMappedPositions[i++] = xmf3Corners[7];
}

void CBoundingBoxMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	OnPreRender(pd3dCommandList);
	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}

