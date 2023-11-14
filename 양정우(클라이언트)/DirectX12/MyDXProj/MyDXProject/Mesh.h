#pragma once
#include "stdafx.h"

// Vertex - Position
// 정점을 표현하기 위한 클래스를 선언한다
class CVertex
{
protected:
	// 정점의 위치 벡터이다(모든 정점은 최소한 위치 벡터를 가져야 한다)
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }
};
// Vertex - Color
class CDiffusedVertex : public CVertex
{
protected:
	//정점의 색상이다
	XMFLOAT4 m_xmf4Diffuse;

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

// Mesh
class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();

protected:

// For The Vertex Buffer
	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;											// 입력 버퍼에 대한 View를 생성 하기 위한 변수

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	// 정점을 어떤식으로 그릴지 - 삼각형 리스트로
	UINT m_nSlot = 0;																		// GPU레지스터 슬롯
	UINT m_nVertices = 0;																	// 정점 수
	UINT m_nStride = 0;																		// 정점의 크기
	UINT m_nOffset = 0;																		// 버퍼의 시작위치에서 읽을 데이터의 오프셋

// For The Index Buffer
	ID3D12Resource* m_pd3dIndexBuffer = NULL;												// 인덱스 버퍼(인덱스의 배열), 정점 버퍼(배열)에 대한 인덱스를 가진다.
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;											// 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터

	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	UINT m_nIndices = 0;																	// 인덱스 버퍼에 포함되는 인덱스의 개수이다.
	UINT m_nStartIndex = 0;																	// 인덱스 버퍼에서 메쉬를 그리기 위해 사용되는 시작 인덱스이다.
	int m_nBaseVertex = 0;																	// 인덱스 버퍼의 인덱스에 더해질 인덱스이다. 

// 절두체 컬링을 위해 만드는 OOBB바운딩 박스의 데이터를 담기 위한 변수
	BoundingOrientedBox m_xmBoundingBox;

public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

// 절두체 컬링을 위해 만드는 OOBB바운딩 박스의 데이터를 담기 위한 변수
	// 왜 바운딩 박스를 쓰는가? 절두체와 바운딩 박스의 충돌검사를 위해서...
	BoundingOrientedBox GetBoundingBox() { return(m_xmBoundingBox); }

// 광선과 메쉬의 교차를 검사하고 교차하는 횟수와 거리를 반환하는 함수이다.
	int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance);

protected:
//정점을 픽킹을 위하여 저장한다(정점 버퍼를 Map()하여 읽지 않아도 되도록).
	CDiffusedVertex* m_pVertices = NULL;

//메쉬의 인덱스를 저장한다(인덱스 버퍼를 Map()하여 읽지 않아도 되도록).
	UINT* m_pnIndices = NULL;
};

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
	// 직육면체의 가로, 세로, 깊이의 길이를 지정하여 직육면체 Mesh를 생성한다.
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


//Sphere, 구를 만들거야////////////////////////////////////////
//
//
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
//
///////////////////////////////////////////////////////////////