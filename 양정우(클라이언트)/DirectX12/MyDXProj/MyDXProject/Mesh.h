#pragma once
#include "stdafx.h"

// Vertex - Position
class CVertex	// ������ ǥ���ϱ� ���� Ŭ������ �����Ѵ�
{
protected:
	XMFLOAT3 m_xmf3Position;	// ������ ��ġ �����̴�(��� ������ �ּ��� ��ġ ���͸� ������ �Ѵ�)

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }
};
// Vertex - Color
class CDiffusedVertex : public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse;	//������ �����̴�

public:
	CDiffusedVertex() 
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) 
	{
		m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse;
	}
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) 
	{
		m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse;
	}
	~CDiffusedVertex() { }
};


//Mesh////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

private:
	//�ν��Ͻ�(Instancing)�� ���Ͽ� �޽��� ���� ��ü�鿡 ������ �� �ִ�.
	//���� ������(Reference Count)�� �޽��� �����Ǵ� ���� ��ü�� ������ ��Ÿ����.	
	int m_nReferences = 0;

public:
	//�޽��� ���� ��ü�� ������ ������ �������� 1�� ������Ų��. 
	void AddRef() { m_nReferences++; }
	//�޽��� �����ϴ� ���� ��ü�� �Ҹ�� ������ �������� 1�� ���ҽ�Ų��.
	//�������� 0�̵Ǹ� �޽��� �Ҹ��Ų��. 
	void Release() { m_nReferences--; if (m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();

protected:
// For The Model Load
	XMFLOAT3* m_pxmf3Positions = NULL;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;

	XMFLOAT3* m_pxmf3Normals = NULL;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;

	XMFLOAT2* m_pxmf2TextureCoords = NULL;
	ID3D12Resource* m_pd3dTextureCoordBuffer = NULL;
	ID3D12Resource* m_pd3dTextureCoordUploadBuffer = NULL;

// For The Vertex Buffer
	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;

	//D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;											// �Է� ���ۿ� ���� View�� ���� �ϱ� ���� ����
	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dVertexBufferViews = NULL;
	// ���� ���� �ҷ��;� �ؼ� �迭�������� �����Ͽ���.
	// ���ۺ��� ������ ������ ��������� �Ѵ�.(������ 1��, �𵨷ε�� ��κ�3��)

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	// ������ ������� �׸��� - �ﰢ�� ����Ʈ��
	UINT m_nSlot = 0;																		// GPU�������� ����
	UINT m_nVertices = 0;																	// ���� ��
	UINT m_nStride = 0;																		// ������ ũ��
	UINT m_nOffset = 0;																		// ������ ������ġ���� ���� �������� ������

// For The Index Buffer
	ID3D12Resource* m_pd3dIndexBuffer = NULL;												// �ε��� ����(�ε����� �迭), ���� ����(�迭)�� ���� �ε����� ������.
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;											// �ε��� ���۸� ���� ���ε� ���ۿ� ���� �������̽� ������

	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	UINT m_nIndices = 0;																	// �ε��� ���ۿ� ���ԵǴ� �ε����� �����̴�.
	UINT m_nStartIndex = 0;																	// �ε��� ���ۿ��� �޽��� �׸��� ���� ���Ǵ� ���� �ε����̴�.
	int m_nBaseVertex = 0;																	// �ε��� ������ �ε����� ������ �ε����̴�. 

// ����ü �ø��� ���� ����� OOBB�ٿ�� �ڽ��� �����͸� ��� ���� ����
	BoundingOrientedBox m_xmBoundingBox;

public:
	//�޽��� �������Ѵ�.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

// ����ü �ø��� ���� ����� OOBB�ٿ�� �ڽ��� �����͸� ��� ���� ����
	// �� �ٿ�� �ڽ��� ���°�? ����ü�� �ٿ�� �ڽ��� �浹�˻縦 ���ؼ�...
	BoundingOrientedBox GetBoundingBox() { return(m_xmBoundingBox); }

// ������ �޽��� ������ �˻��ϰ� �����ϴ� Ƚ���� �Ÿ��� ��ȯ�ϴ� �Լ��̴�.
	int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance);

protected:
//������ ��ŷ�� ���Ͽ� �����Ѵ�(���� ���۸� Map()�Ͽ� ���� �ʾƵ� �ǵ���).
	CDiffusedVertex* m_pVertices = NULL;

//�޽��� �ε����� �����Ѵ�(�ε��� ���۸� Map()�Ͽ� ���� �ʾƵ� �ǵ���).
	UINT* m_pnIndices = NULL;
};
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CModelMesh : public CMesh
{
public:
	CModelMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
	virtual ~CModelMesh();
};

//Mesh�� �̿��ؼ� ���� ������Ʈ��////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Triangle
class CTriangleMesh : public CMesh
{
public:
	CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTriangleMesh() { }
};

// Cube
class CCubeMeshDiffused : public CMesh
{
public:
	// ������ü�� ����, ����, ������ ���̸� �����Ͽ� ������ü Mesh�� �����Ѵ�.
	CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshDiffused();
};

// Airplane
class CAirplaneMeshDiffused : public CMesh
{
public:
	CAirplaneMeshDiffused(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f,
		XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f)
	);
	virtual ~CAirplaneMeshDiffused();
};

// Ground
class CGroundMeshDiffused : public CMesh
{
public:
	CGroundMeshDiffused(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		float xlength = 2000.0f, float zlength = 2000.0f,
		XMFLOAT4 xmf4Color = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f)
	);
	virtual ~CGroundMeshDiffused();
};

// Sphere
class CSphereMeshDiffused : public CMesh
{
public:
	CSphereMeshDiffused(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList * pd3dCommandList,
		float fRadius = 10.0f, int nSlices = 20,
		int nStacks = 20
	);
	virtual ~CSphereMeshDiffused();
};
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////