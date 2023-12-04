#pragma once
#include "stdafx.h"

// Vertex - Position
class CVertex	// 정점을 표현하기 위한 클래스를 선언한다
{
protected:
	XMFLOAT3 m_xmf3Position;	// 정점의 위치 벡터이다(모든 정점은 최소한 위치 벡터를 가져야 한다)

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }
};
// Vertex - Color
class CDiffusedVertex : public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse;	//정점의 색상이다

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
	//인스턴싱(Instancing)을 위하여 메쉬는 게임 객체들에 공유될 수 있다.
	//다음 참조값(Reference Count)은 메쉬가 공유되는 게임 객체의 개수를 나타낸다.	
	int m_nReferences = 0;

public:
	//메쉬가 게임 객체에 공유될 때마다 참조값을 1씩 증가시킨다. 
	void AddRef() { m_nReferences++; }
	//메쉬를 공유하는 게임 객체가 소멸될 때마다 참조값을 1씩 감소시킨다.
	//참조값이 0이되면 메쉬를 소멸시킨다. 
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

	//D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;											// 입력 버퍼에 대한 View를 생성 하기 위한 변수
	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dVertexBufferViews = NULL;
	// 이제 모델을 불러와야 해서 배열형식으로 변경하였다.
	// 버퍼뷰의 갯수는 생성시 결정해줘야 한다.(단일은 1개, 모델로드는 대부분3개)

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
	//메쉬를 렌더링한다.
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
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CModelMesh : public CMesh
{
public:
	CModelMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
	virtual ~CModelMesh();
};

//Mesh를 이용해서 만든 오브젝트들////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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