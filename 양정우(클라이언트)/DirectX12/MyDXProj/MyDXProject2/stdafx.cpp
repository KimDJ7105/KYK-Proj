// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// LabProject03-1.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
// TODO: 필요한 추가 헤더는
// 이 파일이 아닌 STDAFX.H에서 참조합니다.

//UINT gnCbvSrvDescriptorIncrementSize;

ID3D12Resource *CreateBufferResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource **ppd3dUploadBuffer)
{
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COMMON;
//	if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD) d3dResourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
//	else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK) d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;

	ID3D12Resource* pd3dBuffer = NULL;
	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceInitialStates, NULL, __uuidof(ID3D12Resource), (void **)&pd3dBuffer);
	if (FAILED(hResult))
	{
		return(pd3dBuffer);
	}
	if (pData)
	{
		switch (d3dHeapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			if (ppd3dUploadBuffer)
			{
				d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void **)ppd3dUploadBuffer);

				D3D12_RANGE d3dReadRange = { 0, 0 };
				UINT8 *pBufferDataBegin = NULL;
				(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void **)&pBufferDataBegin);
				memcpy(pBufferDataBegin, pData, nBytes);
				(*ppd3dUploadBuffer)->Unmap(0, NULL);

				pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);

				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				d3dResourceBarrier.Transition.pResource = pd3dBuffer;
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8 *pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void **)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
			break;
		}
	}
	return(pd3dBuffer);
}

CGameObject **LoadGameObjectsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, int *pnGameObjects)
{
	FILE *pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[256] = { '\0' };
	char pstrFilePath[256] = { '\0' };
	strcpy_s(pstrFilePath, 256, pstrFileName);
	char *pSlash = strrchr(pstrFilePath, '/');
	*(pSlash + 1) = '\0';

	BYTE nStrLength = 0;
	UINT nReads = 0;
	int nMeshes = 0, nMaterials = 0;

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Meshes>:
	nReads = (UINT)::fread(&nMeshes, sizeof(int), 1, pFile);
	char** ppMeshNames = new char* [nMeshes];
	CMesh** ppMeshes = new CMesh* [nMeshes];
	for (int j = 0; j < nMeshes; j++)
	{
		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"Mesh Name"
		ppMeshNames[j] = new char[nStrLength + 2];
		strcpy_s(ppMeshNames[j], nStrLength + 1, pstrToken);
		ppMeshes[j] = NULL;
	}

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Materials>:
	nReads = (UINT)::fread(&nMaterials, sizeof(int), 1, pFile);
	char** ppMaterialNames = new char* [nMaterials];
	CMaterial** ppMaterials = new CMaterial* [nMaterials];
	for (int j = 0; j < nMaterials; j++)
	{
		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"Material Name"
		ppMaterialNames[j] = new char[nStrLength + 2];
		strcpy_s(ppMaterialNames[j], nStrLength + 1, pstrToken);
		ppMaterials[j] = NULL;
	}

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<GameObjects>:
	nReads = (UINT)::fread(pnGameObjects, sizeof(int), 1, pFile);

	CGameObject **ppGameObjects = new CGameObject*[*pnGameObjects];

	CGameObject *pGameObject = NULL;
	for (int i = 0; i < *pnGameObjects; i++)
	{
		while (1)
		{
			::ReadUnityBinaryString(pFile, pstrToken, &nStrLength);

			if (!strcmp(pstrToken, "<GameObject>:"))
			{
				pGameObject = new CGameObject(pd3dDevice, pd3dCommandList, 0);

				::ReadUnityBinaryString(pFile, pGameObject->m_pstrName, &nStrLength);

				nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile);
			}
			else if (!strcmp(pstrToken, "</GameObject>"))
			{
				break;
			}
			else if (!strcmp(pstrToken, "<Mesh>:"))
			{
				::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"Mesh Name"

				int nMeshIndex = -1;
				for (int j = 0; j < nMeshes; j++)
				{
					if (!strcmp(&pstrToken[1], ppMeshNames[j]))
					{
						nMeshIndex = j;
						break;
					}
				}

				if ((pstrToken[0] == '@') && (nMeshIndex != -1) && ppMeshes[nMeshIndex])
				{
					if (ppMeshes[nMeshIndex]) pGameObject->SetMesh(ppMeshes[nMeshIndex]);
				}
				else
				{
//					strcpy_s(pSlash + 1, 256 - (strlen(pstrFilePath) - 9 - 1), &pstrToken[1]);
					*(pSlash + 1) = '\0';
					strcat_s(pstrFilePath, &pstrToken[1]);
					strcat_s(pstrFilePath, ".bin");

					CMesh* pMesh = new CMeshFromFile(pd3dDevice, pd3dCommandList, pstrFilePath);
					ppMeshes[nMeshIndex] = pMesh;
					pGameObject->SetMesh(pMesh);
				}

				::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Mesh>
			}
			else if (!strcmp(pstrToken, "<Materials>:"))
			{
				nReads = (UINT)::fread(&pGameObject->m_nMaterials, sizeof(int), 1, pFile);

				pGameObject->m_ppMaterials = new CMaterial * [pGameObject->m_nMaterials];

				for (UINT k = 0; k < pGameObject->m_nMaterials; k++)
				{
					pGameObject->m_ppMaterials[k] = NULL;

					::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //<Material>:
					::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"Material Name"

					int nMaterialIndex = -1;
					for (int j = 0; j < nMaterials; j++)
					{
						if (!strcmp(&pstrToken[1], ppMaterialNames[j]))
						{
							nMaterialIndex = j;
							break;
						}
					}
					if ((pstrToken[0] == '@') && (nMaterialIndex != -1) && ppMaterials[nMaterialIndex])
					{
						pGameObject->SetMaterial(k, ppMaterials[nMaterialIndex]);
					}
					else
					{
						ppMaterials[nMaterialIndex] = pGameObject->m_ppMaterials[k] = new CMaterial();
						strcpy_s(pGameObject->m_ppMaterials[k]->m_pstrName, 256, &pstrToken[1]);
						pGameObject->m_ppMaterials[k]->LoadMaterialFromFile(pd3dDevice, pd3dCommandList, pFile);
					}

					::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Material>
				}

				::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</Materials>
			}
		}

		ppGameObjects[i] = pGameObject;
	}

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //</GameObjects>

	::fclose(pFile);

	for (int i = 0; i < nMeshes; i++)
	{
		if (ppMeshNames[i]) delete ppMeshNames[i];
//		if (ppMeshes[i]) ppMeshes[i]->Release();
	}
	if (ppMeshNames) delete[] ppMeshNames;
	if (ppMeshes) delete[] ppMeshes;

	for (int i = 0; i < nMaterials; i++)
	{
		if (ppMaterialNames[i]) delete ppMaterialNames[i];
//		if (ppMaterials[i]) ppMaterials[i]->Release();
	}
	if (ppMaterialNames) delete[] ppMaterialNames;
	if (ppMaterials) delete[] ppMaterials;

	return(ppGameObjects);
}
