#include "stdafx.h"

// ���� ���ҽ��� �����ϴ� �Լ��̴�. ������ �� ������ ���� ���� ���ҽ��� �����ϰ� �ʱ�ȭ �����Ͱ� ������ �ʱ�ȭ�Ѵ�.
ID3D12Resource* CreateBufferResource(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList, 
	void* pData, UINT nBytes,
	D3D12_HEAP_TYPE d3dHeapType,
	D3D12_RESOURCE_STATES d3dResourceStates,
	ID3D12Resource** ppd3dUploadBuffer)												// Upload Heap�� ���� �������̽� �������� �����ͷ� �Լ��� ȣ���� ���� �������̽� �����͸� �Ѱ��ش�.
{
	ID3D12Resource* pd3dBuffer = NULL;

	// Buffer Resource�� ��� ���� Descriptor Heap
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;										// �޸� ���� ����
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;		// CPU�������� �Ӽ� ����, Unknown���� ����
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;			// ���ҽ��� �޸𸮸� �Ҵ��ϰ� �����ϴ� ����� ����
	d3dHeapPropertiesDesc.CreationNodeMask = 1;										// ���� ����� ����(GPU �Ѱ�)
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;										// ��� ���ü�(���ϳ��)

	// Heap�� ���� Resource ����
	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;					// Buffer Resource�� �����ϴ� �Լ��ϱ� Buffer�̴�.
	d3dResourceDesc.Alignment = 0;													// �޸� ���� ����, ������ ���� �ʴ´�.
	d3dResourceDesc.Width = nBytes;													// ���ҽ��� ����ũ��, ���������� ����Ʈ ��(ũ��), nBytes�� �������� �ش� �ڵ忡 �ִ´�.
	d3dResourceDesc.Height = 1;														// ���ҽ��� ����, 1�̸� ���۸��ҽ��� 1�� �����̴�.
	d3dResourceDesc.DepthOrArraySize = 1;											// ���ҽ��� ���� Ȥ�� �迭ũ�⼳��, 1�̸� ���̴� ���� �迭�� 1��
	d3dResourceDesc.MipLevels = 1;													// ���ҽ��� Mip���� �� ����, Mip������ 1���� ���ҽ�
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;									// ���ҽ��� ����, ���� ������ Format�� UNKNOWN�̿��߸� �Ѵ�. 
	d3dResourceDesc.SampleDesc.Count = 1;											// MSAA�� ������� �ʴ´�
	d3dResourceDesc.SampleDesc.Quality = 0;											// MSAA�� ������� �ʴ´�
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;						// ���̾ƿ��� �ֿ� ���̾ƿ��̴�.
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;								// �÷��״� ����.


	// Resource�� �����ϴ� �κ�, Heap Type�� ���� �޶����� �����.
	D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;// Heap Type�� Default��� COPY_DEST�� ���·� �������ش�.
	if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD)										// Heap Type�� Upload��� COPY_SOURCE �����̿��� �ϴµ�
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;				// �� GENERIC_READ�����ϱ�?
	}
	else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	HRESULT hResult = pd3dDevice->CreateCommittedResource(							// �Է� ���� ��ü�� ����
		&d3dHeapPropertiesDesc,
		D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc,
		d3dResourceInitialStates,
		NULL,
		__uuidof(ID3D12Resource), (void**)&pd3dBuffer);

	if (pData)
	{
		switch (d3dHeapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:											// �� Ÿ���� �����Ǿ����� �ʴٸ�
		{
			if (ppd3dUploadBuffer)
			{
				// ���ε� ���۸� �����Ѵ�. ��? DefaultŸ���϶��� Map�� Unmap�Լ��� ����� �� ���� �����̴�.
				d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				pd3dDevice->CreateCommittedResource(							// �Է� ���� ��ü�� ����
					&d3dHeapPropertiesDesc,
					D3D12_HEAP_FLAG_NONE,
					&d3dResourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					NULL,
					__uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

				// ���ε� ���۸� �����Ͽ� �ʱ�ȭ �����͸� ���ε� ���ۿ� �����Ѵ�.
				D3D12_RANGE d3dReadRange = { 0, 0 };
				UINT8* pBufferDataBegin = NULL;
				(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);							// Resource�� �����͸� �Ѱ��ش�. Map ���·� ���� �ٸ��ֵ��� ���� ���ϰ� ���´�.
				memcpy(pBufferDataBegin, pData, nBytes);														// �Ѱܹ��� �޸𸮸� ����Ѵ�.
				(*ppd3dUploadBuffer)->Unmap(0, NULL);															// �� ��������� �ٽ� Map���¸� �������༭ �ٸ��ֵ鵵 ����� �� �ְ� ���ش�.

				// ���ε� ������ ������ ����Ʈ ���ۿ� �����Ѵ�.
				pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);									// GPU����, ���� Resource���� ���� Resource�� Resource ��ü�� ������ �� �ִ� ����̴�.
				
				// (*ppd3dUploadBuffer)->Release();
				// ������ �߻��ع�����. ��? CPU���� �ٷ� ó���Ǵ°��� �ƴ� CommandList�� ��� ����� ���������� �̷���� ������ �Ǵ°Ŵϱ�.

				// ���簡 �Ϸ�� ���� Resource Barrier�� ���ؼ� 
				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;									// NONE - �⺻����, BEGIN_ONLY - ���ۺκи� ����, END_ONLY - ���κи� ����
				d3dResourceBarrier.Transition.pResource = pd3dBuffer;
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;						// ���� State�� COPY_DEST
				d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;									// ���� State�� (��κ� D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER �̰Ŵ�.)
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:											// �� Ÿ���� Upload ���
		{
			// �׳� ��.�ٸ� �������൵ �ȴ�. ������ Default�� ����°ź��� ������ ������ �� �ִ�. ����ϰ� �ٲ�� ��쿡�� ����ϴ� Heap Type�� �� ��.
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);		// Resource�� �����͸� �Ѱ��ش�. Map ���·� ���� �ٸ��ֵ��� ���� ���ϰ� ���´�.
			memcpy(pBufferDataBegin, pData, nBytes);							// �Ѱܹ��� �޸𸮸� ����Ѵ�.
			pd3dBuffer->Unmap(0, NULL);											// �� ��������� �ٽ� Map���¸� �������༭ �ٸ��ֵ鵵 ����� �� �ְ� ���ش�.
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:											// �� Ÿ���� Read Back �̶��
		{
			break;
		}
		}
	}
	return (pd3dBuffer);
}
