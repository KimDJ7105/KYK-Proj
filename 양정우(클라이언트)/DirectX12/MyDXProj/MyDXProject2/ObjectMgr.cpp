
#include "stdafx.h"

#include <iostream>
#include "ObjectMgr.h"
#include "Object.h"
#include "Player.h"
#include "Scene.h"

ObjectMgr::ObjectMgr()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		m_Objects[i] = NULL;
	}

}

ObjectMgr::~ObjectMgr()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		if (m_Objects[i] != NULL)
		{
			delete m_Objects[i];
			m_Objects[i] = NULL;
		}
	}
}

int ObjectMgr::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignagure, XMFLOAT3 xmf3Position, int type)
{
	int index = -1;
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		if (m_Objects[i] == NULL)
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
	{
		m_Objects[index] = new CBoxPlayer(pd3dDevice, pd3dCommandList, pd3dRootSignagure);
		m_Objects[index]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_Objects[index]->SetPosition(xmf3Position);
		m_Objects[index]->SetType(PLAYER_TYPE);
		
		return index;
	}

	std::cout << "No more empty object slot!" << std::endl;
	return index;
}

bool ObjectMgr::DeleteObject(int id)
{
	return false;
}

void ObjectMgr::DrawAllObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	
	//for (int i = 0; i < MAX_NUM_OBJECT; i++)
	//{
	//	if (m_Objects[i] != NULL)
	//	{
	//		m_Objects[i]->Draw(renderer);		// 드로우를 할때도 포인터를 넘길것이다. 이러면 중복되는 코드를 막을 수 있다.
	//	}
	//}
}
