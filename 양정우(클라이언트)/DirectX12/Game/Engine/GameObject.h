#pragma once
#include "Component.h"
#include "Object.h"

class Transform;
class MeshRenderer;
class Camera;
class Light;
class MonoBehaviour;
class ParticleSystem;
class Terrain;
class BaseCollider;
class Animator;

enum class GAMEOBJECT_TYPE
{
	DEFAULT,
	PLAYER,
	OTHER_PLAYER,

	TYPE_END
};

class GameObject : public Object, public std::enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	shared_ptr<Component> GetFixedComponent(COMPONENT_TYPE type);

	shared_ptr<Transform> GetTransform();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<Camera> GetCamera();
	shared_ptr<Light> GetLight();
	shared_ptr<ParticleSystem> GetParticleSystem();
	shared_ptr<Terrain> GetTerrain();
	shared_ptr<BaseCollider> GetCollider();
	shared_ptr<Animator> GetAnimator();

	void AddComponent(shared_ptr<Component> component);

	void SetCheckFrustum(bool checkFrustum) { _chechFrustum = checkFrustum; }
	bool GetCheckFrustum() { return _chechFrustum; }

	void SetLayerIndex(uint8 layer) { _layerIndex = layer; }
	uint8 GetLayerIndex() { return _layerIndex; }

	void SetStatic(bool flag) { _static = flag; }
	bool IsStatic() { return _static; }

private:
	std::array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components;
	std::vector<shared_ptr<MonoBehaviour>> _scripts;

	bool _chechFrustum = true; // 디폴트로 프러스텀 컬링판단을 한다.
	uint8 _layerIndex = 0;
	bool _static = true;

	//내가 짠 코드--------------------------------
private:
	GAMEOBJECT_TYPE _type = GAMEOBJECT_TYPE::DEFAULT;

public:
	void SetObjectType(GAMEOBJECT_TYPE type) { _type = type; }
	GAMEOBJECT_TYPE GetObjectType() { return _type; }
};

