#pragma once


class ObjectMgr
{
public:

	ObjectMgr();
	~ObjectMgr();

	int AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignagure, XMFLOAT3 xmf3Position, int type);
	bool DeleteObject(int id);

	void DrawAllObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:

	CGameObject* m_Objects[MAX_NUM_OBJECT];

	CPlayer* m_pPlayer = NULL;
};

