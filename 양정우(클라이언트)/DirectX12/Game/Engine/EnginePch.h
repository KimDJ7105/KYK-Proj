#pragma once

// std::byte 사용하지 않음
#define _HAS_STD_BYTE 0

// 각종 include
#include <windows.h>
#include <tchar.h>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

#include "d3dx12.h"
#include "SimpleMath.h"
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>

// 각종 lib
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
#else
#pragma comment(lib, "DirectXTex\\DirectXTex.lib")
#endif

// 각종 typedef
using int8		= __int8;
using int16		= __int16;
using int32		= __int32;
using int64		= __int64;
using uint8		= unsigned __int8;
using uint16	= unsigned __int16;
using uint32	= unsigned __int32;
using uint64	= unsigned __int64;
using Vec2		= DirectX::SimpleMath::Vector2;
using Vec3		= DirectX::SimpleMath::Vector3;
using Vec4		= DirectX::SimpleMath::Vector4;
using Matrix	= DirectX::SimpleMath::Matrix;

// Constant Buffer View의 Register 형태
enum class CBV_REGISTER : uint8
{
	b0,
	b1,
	b2,
	b3,
	b4,

	END
};

enum class SRV_REGISTER : uint8
{
	t0 = static_cast<uint8>(CBV_REGISTER::END),	// CBV가 끝나는 시점에서 시작한다.
	t1,
	t2,
	t3,
	t4,

	END
};

enum
{
	SWAP_CHAIN_BUFFER_COUNT = 2,										// 스왑체인 버퍼의 갯수 지정
	CBV_REGISTER_COUNT = CBV_REGISTER::END,								// CBV의 Register 갯수
	SRV_REGISTER_COUNT = static_cast<uint8>(SRV_REGISTER::END) - CBV_REGISTER_COUNT,
	REGISTER_COUNT = CBV_REGISTER_COUNT + SRV_REGISTER_COUNT,			// 각 디스크립터 힙의 그룹마다의 레지스터 갯수


};

// 윈도우 실행시 어떤식으로 동작할지
enum
{
	WINDOW_WIDTH = 800,
	WINDOW_HEIGHT = 600,
	IS_WINDOW_MODE = true
};

struct WindowInfo
{
	HWND		hwnd;		// 출력 윈도우
	int32		width;		// 윈도우의 너비
	int32		height;		// 윈돋우의 높이
	bool		windowed;	// 창보드 or 전제화면

};

// 정점 정보
struct Vertex
{
	Vertex() {}

	Vertex(Vec3 p, Vec2 u, Vec3 n, Vec3 t)
		: pos(p), uv(u), normal(n), tangent(t)
	{
	}


	Vec3 pos;
	Vec2 uv;
	Vec3 normal;
	Vec3 tangent;

};

#define DECLARE_SINGLE(type)		\
private:							\
	type() {}						\
	~type() {}						\
public:								\
	static type* GetInstance()		\
	{								\
		static type instance;		\
		return &instance;			\
	}								\

#define GET_SINGLE(type)	type::GetInstance()

// 아래 정의를 사용하려면 Engine.h를 include해주어야 한다.
// 혹은 현재 위치(헤더)에 Engine.h를 include해주어도 상관없다.
#define DEVICE				GEngine->GetDevice()->GetDevice()		// 바로 작업이 이루어짐
#define CMD_LIST			GEngine->GetCmdQueue()->GetCmdList()	// 예약했다가 나중에 이루어짐
#define ROOT_SIGNATURE		GEngine->GetRootSignature()->GetSignature()
#define RESOURCE_CMD_LIST	GEngine->GetCmdQueue()->GetResourceCmdList()

#define INPUT				GET_SINGLE(Input)
#define DELTA_TIME			GET_SINGLE(Timer)->GetDeltaTime()

#define CONST_BUFFER(type)	GEngine->GetConstantBuffer(type)

struct TransformParams
{
	Matrix matWorld;
	Matrix matView;
	Matrix matProjection;
	Matrix matWV;
	Matrix matWVP;
};

extern unique_ptr<class Engine> GEngine;		// 앞으로 이런 애가 나올것이라고 미리 선언해줌
