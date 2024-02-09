#pragma once


class ObjectMgr
{
public:

	ObjectMgr();
	~ObjectMgr();

	int AddObject(CPlayer* pPlayer, int type);
	bool DeleteObject(int id);

	void DrawAllObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	XMFLOAT3 GetObjectsPosition(int id);

private:

	CGameObject* m_Objects[MAX_NUM_OBJECT];

	CPlayer* m_pPlayer = NULL;
};
