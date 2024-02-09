
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

int ObjectMgr::AddObject(CPlayer* pPlayer, int type)
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
		m_Objects[index] = pPlayer;
		m_Objects[index]->SetType(type);
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

XMFLOAT3 ObjectMgr::GetObjectsPosition(int type)
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		if (m_Objects[i] != NULL)
		{
			if (m_Objects[i]->m_type == type)
			{
				return m_Objects[i]->GetPosition();
			}
			else
			{

			}
		}
	}
	
}
