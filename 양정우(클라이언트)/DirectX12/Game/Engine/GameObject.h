#pragma once
#include "Component.h"
#include "Object.h"

class Transform;
class MeshRenderer;
class MonoBehaviour;
class Camera;
class Light;

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

	void AddComponent(shared_ptr<Component> component);

	void SetCheckFrustum(bool checkFrustum) { _chechFrustum = checkFrustum; }
	bool GetCheckFrustum() { return _chechFrustum; }

	void SetObjectType(GAMEOBJECT_TYPE type) { _type = type; }
	GAMEOBJECT_TYPE GetObjectType() { return _type; }

private:
	std::array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components;
	std::vector<shared_ptr<MonoBehaviour>> _scripts;

	bool _chechFrustum = true;

	GAMEOBJECT_TYPE _type = GAMEOBJECT_TYPE::DEFAULT;
};

