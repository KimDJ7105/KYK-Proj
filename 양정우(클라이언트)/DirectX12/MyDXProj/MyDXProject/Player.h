#pragma once
#include "stdafx.h"

#define DIR_FORWARD 0x01		// 전진
#define DIR_BACKWARD 0x02		// 후진
#define DIR_LEFT 0x04			// 왼쪽
#define DIR_RIGHT 0x08			// 오른쪽
#define DIR_UP 0x10				// 위
#define DIR_DOWN 0x20			// 아래

#include "Object.h"
#include "Camera.h"

// CPlayer
class CPlayer : public CGameObject
{
protected:
	// Vector
	XMFLOAT3 m_xmf3Position;			// Position Vector
	XMFLOAT3 m_xmf3Right;				// x - 축(Right) Vector
	XMFLOAT3 m_xmf3Up;					// y - 축(Up) Vector
	XMFLOAT3 m_xmf3Look;				// z - 축(Look) Vector

	// Rotate Value
	float m_fPitch;						//  x-축(Right)
	float m_fYaw;						//  y-축(Up)
	float m_fRoll;						// z-축(Look)

	// Move Velocity
	XMFLOAT3 m_xmf3Velocity;

	// Gravity
	XMFLOAT3 m_xmf3Gravity;

	// Player Max Velocity
	float m_fMaxVelocityXZ;				// XZ(평면)
	float m_fMaxVelocityY;				// Y(높이)

	// Friction
	float m_fFriction;					// 마찰력

	// Update Callback
	LPVOID m_pPlayerUpdatedContext;		// Player
	LPVOID m_pCameraUpdatedContext;		// Camera

	// Camera Mode
	CCamera* m_pCamera = NULL;

public:
	CPlayer();
	virtual ~CPlayer();

	// Vector
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	// Friction
	void SetFriction(float fFriction) { m_fFriction = fFriction; }

	// Gravity
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }

	// Player Max Velocity
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }

	// Move Velocity
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	
	
	
	
	void SetPosition(XMFLOAT3& xmf3Position){								// 플레이어의 위치를 xmf3Position 위치로 설정한다.
		Move(XMFLOAT3(	xmf3Position.x - m_xmf3Position.x,					// xmf3Position Vector에서 현재 플레이어의 Position Vector를 빼면
						xmf3Position.y - m_xmf3Position.y,					// 현재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다.
						xmf3Position.z - m_xmf3Position.z), false);			// 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.
	}

	// Rotate Value
	float GetPitch() { return(m_fPitch); }
	float GetYaw() { return(m_fYaw); }
	float GetRoll() { return(m_fRoll); }

	// Camera
	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

	// Move Player 
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	// Rotate Player
	void Rotate(float x, float y, float z);

	// Update (Position, Rotate data)
	void Update(float fTimeElapsed);

	// Update Callback
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }								// Player
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }	// Player
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }								// Camera
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }	// Camera

	// Shader Buffer(다른데서도 많이 쓴다. 근데 쓸데마다 주석이름이 다르니 조심하자)
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	// Change Camera
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {return(NULL);}

	// for World Transform
	virtual void OnPrepareRender();

	// if Third Person Mode, Then Render Player Mesh
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

// CPlayer - Airplane
class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(
		ID3D12Device* pd3dDevice, 
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CAirplanePlayer();

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPrepareRender();
};