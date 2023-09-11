#pragma once

#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

public:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float						m_fFriction = 125.0f;

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	CCamera*					m_pCamera = NULL;

	int							hp = 10;

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void Move(DWORD dwDirection, float fDistance);
	void Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

	void SetCameraOffset(XMFLOAT3& xmf3CameraOffset);

	void Update(float fTimeElapsed = 0.016f);


	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
	XMFLOAT3 return_position();
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() { return(m_pCamera); }
};

#define BULLETS					50

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();

	float						m_fBulletEffectiveRange = 150.0f;
	CBulletObject*				m_ppBullets[BULLETS];

	void FireBullet(CGameObject* pLockedObject);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};

class CTankPlayer : public CPlayer
{
public:
	CTankPlayer();
	virtual ~CTankPlayer();

	CGameObject*				m_pTurret = NULL;
	CGameObject*				m_pGun = NULL;

	float						m_fBulletEffectiveRange = 150.0f;
	float						reload = 0.0f;
	CBulletObject*				m_ppBullets[BULLETS];

	void RotateTurret(float fAngle) { m_pTurret->Rotate(0.0f, fAngle, 0.0f); }
	void RotateGun(float fAngle) { m_pGun->Rotate(fAngle, 0.0f, 0.0f); }

	void FireBullet(CGameObject* pLockedObject);

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];
	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 2.0f;
	float						m_fExplosionSpeed = 30.0f;
	float						m_fExplosionRotation = 720.0f;
	CMesh* m_pExplosionMesh;
	XMFLOAT3				m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
};
