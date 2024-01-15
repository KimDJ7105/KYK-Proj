//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	char							m_pstrName[256] = { '\0' };

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	UINT							m_nVertices = 0;

	XMFLOAT3						*m_pxmf3Positions = NULL;
	ID3D12Resource					*m_pd3dPositionBuffer = NULL;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = NULL;

	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW		*m_pd3dVertexBufferViews = NULL;

	UINT							m_nIndices = 0;
	UINT							*m_pnIndices = NULL;

public:
	UINT							m_nSubsets = 0;

	UINT*							m_pnSubSetIndices = NULL;
	UINT*							m_pnSubSetStartIndices = NULL;
	UINT**							m_ppnSubSetIndices = NULL;

	ID3D12Resource					**m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource					**m_ppd3dSubSetIndexUploadBuffers = NULL;

	D3D12_INDEX_BUFFER_VIEW			*m_pd3dIndexBufferViews = NULL;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;

public:
	BoundingOrientedBox				m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

public:
	virtual void ReleaseUploadBuffers();
	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSubset) { }

	BoundingOrientedBox GetBoundingBox() { return(m_xmBoundingBox); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeshFromFile : public CMesh
{
public:
	CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName = NULL);
	virtual ~CMeshFromFile();

protected:
	XMFLOAT3*						m_pxmf3Normals = NULL;
	ID3D12Resource*					m_pd3dNormalBuffer = NULL;
	ID3D12Resource*					m_pd3dNormalUploadBuffer = NULL;

public:
	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSubset);

	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CBoundingBoxMesh : public CMesh
{
public:
	CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CBoundingBoxMesh();

	XMFLOAT3*						m_pcbMappedPositions = NULL;

	void UpdateVertexPosition(BoundingOrientedBox* pxmBoundingBox);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};
