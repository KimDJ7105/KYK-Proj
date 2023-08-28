#pragma once
#include "GameObject.h"
#include "Camera.h"

class Cenemy : public CGameObject
{
public:
	Cenemy();
	virtual ~Cenemy();

public:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3					m_xmf3TargetLook = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float						m_fFriction = 125.0f;

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	float						reload = 10.0f;

	int						hp = 5;

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);
	void TargetLookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void Move(DWORD dwDirection, float fDistance);
	void Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

	void Update(float fTimeElapsed = 0.016f);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime, XMFLOAT3 position);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};

#define BULLETS					50

class CTankenemy : public Cenemy
{
public:
	CTankenemy();
	virtual ~CTankenemy();

	CGameObject* m_pTurret = NULL;
	CGameObject* m_pGun = NULL;

	float						m_fBulletEffectiveRange = 150.0f;
	CBulletObject* m_ppBullets[BULLETS];

	void RotateTurret(float fAngle) { m_pTurret->Rotate(0.0f, fAngle, 0.0f); }
	void RotateGun(float fAngle) { m_pGun->Rotate(fAngle, 0.0f, 0.0f); }

	void FireBullet(CGameObject* pLockedObject);

	virtual void Animate(float fElapsedTime, XMFLOAT3 position);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];
	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 2.0f;
	float						m_fExplosionSpeed = 30.0f;
	float						m_fExplosionRotation = 720.0f;
	CMesh* m_pExplosionMesh;
	XMFLOAT3				m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
	void PrepareExplosion();
};