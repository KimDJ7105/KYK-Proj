#include "stdafx.h"

// 버퍼 리소스를 생성하는 함수이다. 버퍼의 힙 유형에 따라 버퍼 리소스를 생성하고 초기화 데이터가 있으면 초기화한다.
ID3D12Resource* CreateBufferResource(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList, 
	void* pData, UINT nBytes,
	D3D12_HEAP_TYPE d3dHeapType,
	D3D12_RESOURCE_STATES d3dResourceStates,
	ID3D12Resource** ppd3dUploadBuffer)												// Upload Heap에 대한 인터페이스 포인터의 포인터로 함수를 호출한 쪽의 인터페이스 포인터를 넘겨준다.
{
	ID3D12Resource* pd3dBuffer = NULL;

	// Buffer Resource를 담기 위한 Descriptor Heap
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;										// 메모리 힙의 유형
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;		// CPU페이지의 속성 설정, Unknown으로 설정
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;			// 리소스가 메모리를 할당하고 해제하는 방식을 결정
	d3dHeapPropertiesDesc.CreationNodeMask = 1;										// 단일 노드라고 설정(GPU 한개)
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;										// 노드 가시성(단일노드)

	// Heap에 넣을 Resource 생성
	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;					// Buffer Resource를 생성하는 함수니까 Buffer이다.
	d3dResourceDesc.Alignment = 0;													// 메모리 정렬 여부, 제한을 두지 않는다.
	d3dResourceDesc.Width = nBytes;													// 리소스의 가로크기, 정점버퍼의 바이트 수(크기), nBytes의 변수값을 해당 코드에 넣는다.
	d3dResourceDesc.Height = 1;														// 리소스의 높이, 1이면 버퍼리소스가 1의 높이이다.
	d3dResourceDesc.DepthOrArraySize = 1;											// 리소스의 깊이 혹은 배열크기설정, 1이면 깊이는 없고 배열이 1개
	d3dResourceDesc.MipLevels = 1;													// 리소스의 Mip레벨 수 결정, Mip레벨이 1개인 리소스
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;									// 리소스의 형식, 정점 버퍼의 Format은 UNKNOWN이여야만 한다. 
	d3dResourceDesc.SampleDesc.Count = 1;											// MSAA를 사용하지 않는다
	d3dResourceDesc.SampleDesc.Quality = 0;											// MSAA를 사용하지 않는다
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;						// 레이아웃이 주요 레이아웃이다.
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;								// 플래그는 없다.


	// Resource를 생성하는 부분, Heap Type에 따라 달라지게 만든다.
	D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;// Heap Type이 Default라면 COPY_DEST를 상태로 설정해준다.
	if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD)										// Heap Type이 Upload라면 COPY_SOURCE 형식이여야 하는데
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;				// 왜 GENERIC_READ형식일까?
	}
	else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	HRESULT hResult = pd3dDevice->CreateCommittedResource(							// 입력 버퍼 객체를 생성
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
		case D3D12_HEAP_TYPE_DEFAULT:											// 힙 타입이 결정되어있지 않다면
		{
			if (ppd3dUploadBuffer)
			{
				// 업로드 버퍼를 생성한다. 왜? Default타입일때는 Map과 Unmap함수를 사용할 수 없기 때문이다.
				d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				pd3dDevice->CreateCommittedResource(							// 입력 버퍼 객체를 생성
					&d3dHeapPropertiesDesc,
					D3D12_HEAP_FLAG_NONE,
					&d3dResourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					NULL,
					__uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

				// 업로드 버퍼를 매핑하여 초기화 데이터를 업로드 버퍼에 복사한다.
				D3D12_RANGE d3dReadRange = { 0, 0 };
				UINT8* pBufferDataBegin = NULL;
				(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);							// Resource의 포인터를 넘겨준다. Map 상태로 만들어서 다른애들이 접근 못하게 막는다.
				memcpy(pBufferDataBegin, pData, nBytes);														// 넘겨받은 메모리를 사용한다.
				(*ppd3dUploadBuffer)->Unmap(0, NULL);															// 다 사용했으니 다시 Map상태를 해제해줘서 다른애들도 사용할 수 있게 해준다.

				// 업로드 버퍼의 내용을 디폴트 버퍼에 복사한다.
				pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);									// GPU에서, 뒤의 Resource에서 앞의 Resource로 Resource 전체를 복사할 수 있는 기능이다.
				
				// (*ppd3dUploadBuffer)->Release();
				// 문제가 발생해버린다. 왜? CPU에서 바로 처리되는것이 아닌 CommandList의 명령 목록이 순차적으로 이루어진 다음에 되는거니까.

				// 복사가 완료된 다음 Resource Barrier를 통해서 
				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;									// NONE - 기본동작, BEGIN_ONLY - 시작부분만 적용, END_ONLY - 끝부분만 적용
				d3dResourceBarrier.Transition.pResource = pd3dBuffer;
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;						// 이전 State는 COPY_DEST
				d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;									// 이후 State는 (대부분 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER 이거다.)
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:											// 힙 타입이 Upload 라면
		{
			// 그냥 복.붙만 진행해줘도 된다. 하지만 Default에 만드는거보다 성능이 떨어질 수 있다. 빈번하게 바뀌는 경우에만 사용하는 Heap Type이 될 것.
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);		// Resource의 포인터를 넘겨준다. Map 상태로 만들어서 다른애들이 접근 못하게 막는다.
			memcpy(pBufferDataBegin, pData, nBytes);							// 넘겨받은 메모리를 사용한다.
			pd3dBuffer->Unmap(0, NULL);											// 다 사용했으니 다시 Map상태를 해제해줘서 다른애들도 사용할 수 있게 해준다.
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:											// 힙 타입이 Read Back 이라면
		{
			break;
		}
		}
	}
	return (pd3dBuffer);
}
