#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"

#include "Engine.h"
#include "Material.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "Light.h"

#include "TestCameraScript.h"
#include "Resources.h"
#include "ParticleSystem.h"

void SceneManager::Update()
{
	if (_activeScene == nullptr)
		return;

	// TODO: 플레이어가 입장했을때 오브젝트를 추가함
	/*if (판단변수 == true)
	{
		add object;
		add object to object manager;
		set object OTHER_PLAYER
		판단변수 false
	}*/

	// 플레이어의 좌표 확인
	Vec3 temp = _player->GetTransform()->GetLocalPosition();
	
	// other player의 좌표 확인
	for (auto& otherPlayer : _otherPlayer)
	{
		Vec3 temp2 = otherPlayer->GetTransform()->GetLocalPosition();
	}

	_activeScene->Update();
	_activeScene->LateUpdate();
	_activeScene->FinalUpdate();
}

// TEMP
void SceneManager::Render()
{
	if (_activeScene)
		_activeScene->Render();
}

void SceneManager::LoadScene(std::wstring sceneName)
{
	// TODO : 기존 Scene 정리
	// TODO : 파일에서 Scene 정보 로드

	_activeScene = LoadTestScene();

	_activeScene->Awake();
	_activeScene->Start();
}

void SceneManager::SetLayerName(uint8 index, const wstring& name)
{
	// 기존 데이터 삭제
	const wstring& prevName = _layerNames[index];
	_layerIndex.erase(prevName);

	_layerNames[index] = name;
	_layerIndex[name] = index;
}

uint8 SceneManager::LayerNameToIndex(const wstring& name)
{
	auto findIt = _layerIndex.find(name);
	if (findIt == _layerIndex.end())
		return 0;

	return findIt->second;
}

shared_ptr<Scene> SceneManager::LoadTestScene()
{
#pragma region LayerMask
	SetLayerName(0, L"Default");
	SetLayerName(1, L"UI");
	//...
	//...
	//...
#pragma endregion

#pragma region ComputeShader
	{
		shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"ComputeShader");

		// UAV 용 Texture 생성
		shared_ptr<Texture> texture = GET_SINGLE(Resources)->CreateTexture(L"UAVTexture",
			DXGI_FORMAT_R8G8B8A8_UNORM, 1024, 1024,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"ComputeShader");
		material->SetShader(shader);
		material->SetInt(0, 1);
		GEngine->GetComputeDescHeap()->SetUAV(texture->GetUAVHandle(), UAV_REGISTER::u0);

		// 쓰레드 그룹 (1 * 1024 * 1)
		material->Dispatch(1, 1024, 1);
	}
#pragma endregion

	shared_ptr<Scene> scene = make_shared<Scene>();

	// 메인카메라를 가장 먼저 추가(안그럼 버그남)
#pragma region Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Main_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>());
		camera->AddComponent(make_shared<TestCameraScript>());
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));

		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, true); // UI는 안 찍음

		camera->SetObjectType(GAMEOBJECT_TYPE::PLAYER);
		scene->AddGameObject(camera);

		_player = camera;//카메라를 플레이어로 설정
		//camera->GetTransform()->GetLocalPosition()
	}
#pragma endregion

#pragma region UI_Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Orthographic_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>()); // Near=1, Far=1000, 800*600
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		camera->GetCamera()->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskAll(); // 다 끄고
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, false); // UI만 찍음
		scene->AddGameObject(camera);
	}
#pragma endregion

#pragma region SkyBox
	{
		shared_ptr<GameObject> skybox = make_shared<GameObject>();
		skybox->AddComponent(make_shared<Transform>());
		skybox->SetCheckFrustum(false);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
			meshRenderer->SetMesh(sphereMesh);
		}
		{
			/*원래 쓰던거
			shared_ptr<Shader> shader = make_shared<Shader>();
			shader->Init(L"..\\Resources\\Shader\\skybox.hlsl",
				{ RASTERIZER_TYPE::CULL_NONE, DEPTH_STENCIL_TYPE::LESS_EQUAL });
				*/
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Skybox");

			/*원래 쓰던거
			shared_ptr<Texture> texture = make_shared<Texture>();
			texture->Init(L"..\\Resources\\Texture\\Sky01.jpg");*/
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Sky01", L"..\\Resources\\Texture\\Sky01.jpg");

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		skybox->AddComponent(meshRenderer);
		scene->AddGameObject(skybox);
	}
#pragma endregion

#pragma region Sphere(Object)
	{
		//shared_ptr<GameObject> obj = make_shared<GameObject>();
		//obj->AddComponent(make_shared<Transform>());
		//obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		//obj->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 500.f));
		//obj->SetStatic(false);	// false로 하여 그림자의 적용을 받는다
		//shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		//{
		//	shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
		//	meshRenderer->SetMesh(sphereMesh);
		//}
		//{
		//	/*shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Deferred");
		//	shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Leather", L"..\\Resources\\Texture\\Leather.jpg");
		//	shared_ptr<Texture> texture2 = GET_SINGLE(Resources)->Load<Texture>(L"Leather_Normal", L"..\\Resources\\Texture\\Leather_Normal.jpg");
		//	shared_ptr<Material> material = make_shared<Material>();
		//	material->SetShader(shader);
		//	material->SetTexture(0, texture);
		//	material->SetTexture(1, texture2);
		//	meshRenderer->SetMaterial(material);*/
		//	// Resource.cpp GameObject부분으로 이동

		//	shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
		//	//material->SetInt(0, 1);
		//	meshRenderer->SetMaterial(material);
		//	/*material->SetInt(0, 0);
		//	meshRenderer->SetMaterial(material->Clone());*/
		//}
		//obj->AddComponent(meshRenderer);
		//scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region Plane
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(1000.f, 1.f, 1000.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0.f, -100.f, 500.f));
		obj->SetStatic(true);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadCubeMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject")->Clone();
			material->SetInt(0, 0);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region Cube
	{
		/*shared_ptr<GameObject> cube = make_shared<GameObject>();
		cube->AddComponent(make_shared<Transform>());
		cube->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		cube->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 150.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> cubeMesh = GET_SINGLE(Resources)->LoadCubeMesh();
			meshRenderer->SetMesh(cubeMesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Deferred");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Leather", L"..\\Resources\\Texture\\Leather.jpg");
			shared_ptr<Texture> texture2 = GET_SINGLE(Resources)->Load<Texture>(L"Leather_Normal", L"..\\Resources\\Texture\\Leather_Normal.jpg");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			material->SetTexture(1, texture2);
			meshRenderer->SetMaterial(material);
		}
		cube->AddComponent(meshRenderer);
		scene->AddGameObject(cube);
		_otherPlayer.push_back(cube);*/
	}
#pragma endregion

#pragma region UI(Sample)
	{
		//shared_ptr<GameObject> sphere = make_shared<GameObject>();
		//sphere->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		//sphere->AddComponent(make_shared<Transform>());
		//sphere->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		//sphere->GetTransform()->SetLocalPosition(Vec3(0, 0, 500.f));
		//shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		//{
		//	shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
		//	meshRenderer->SetMesh(mesh);
		//}
		//{
		//	shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");
		//	shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Monkey", L"..\\Resources\\Texture\\CodingMonkey.jpg");
		//	shared_ptr<Material> material = make_shared<Material>();
		//	material->SetShader(shader);
		//	material->SetTexture(0, texture);
		//	meshRenderer->SetMaterial(material);
		//}
		//sphere->AddComponent(meshRenderer);
		//scene->AddGameObject(sphere);
	}
#pragma endregion

#pragma region Deferred_Rendering_Info

	for (int32 i = 0; i < 6; i++)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f + (i * 120), 250.f, 500.f));			// 1에서 1000사이 아무 값
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture;
			if (i < 3)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->GetRTTexture(i);
			else if (i < 5)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->GetRTTexture(i - 3);
			else
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->GetRTTexture(0);
				//texture = GET_SINGLE(Resources)->Get<Texture>(L"UAVTexture");//compute shader

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region Directional Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->AddComponent(make_shared<Transform>());
		light->GetTransform()->SetLocalPosition(Vec3(0.f, 1000.f, 500.f));
		light->AddComponent(make_shared<Light>());
		light->GetLight()->SetLightDirection(Vec3(0.f, -1.f, 0.f));
		light->GetLight()->SetLightType(LIGHT_TYPE::DIRECTIONAL_LIGHT);
		light->GetLight()->SetDiffuse(Vec3(1.f, 1.f, 1.f));		// WHITE
		light->GetLight()->SetAmbient(Vec3(0.f, 0.1f, 0.1f));
		light->GetLight()->SetSpecular(Vec3(0.1f, 0.1f, 0.1f));

		scene->AddGameObject(light);
	}
#pragma endregion

#pragma region Point Light
	{
		//shared_ptr<GameObject> light = make_shared<GameObject>();
		//light->AddComponent(make_shared<Transform>());
		//light->GetTransform()->SetLocalPosition(Vec3(0.f, 100.f, 150.f));
		//light->AddComponent(make_shared<Light>());
		////light->GetLight()->SetLightDirection(Vec3(-1.f, -1.f, 0.f));
		//light->GetLight()->SetLightType(LIGHT_TYPE::POINT_LIGHT);
		//light->GetLight()->SetDiffuse(Vec3(0.f, 0.5f, 0.f));		// GREEN
		//light->GetLight()->SetAmbient(Vec3(0.f, 0.3f, 0.f));
		//light->GetLight()->SetSpecular(Vec3(0.0f, 0.3f, 0.f));
		//light->GetLight()->SetLightRange(200.f);
		////light->GetLight()->SetLightAngle(XM_PI / 4);
		//scene->AddGameObject(light);
	}
#pragma endregion

#pragma region Spot Light
	{
		//shared_ptr<GameObject> light = make_shared<GameObject>();
		//light->AddComponent(make_shared<Transform>());
		//light->GetTransform()->SetLocalPosition(Vec3(75.f, 0.f, 150.f));
		//light->AddComponent(make_shared<Light>());
		//light->GetLight()->SetLightDirection(Vec3(-1.f, 0.f, 0.f));
		//light->GetLight()->SetLightType(LIGHT_TYPE::SPOT_LIGHT);
		//light->GetLight()->SetDiffuse(Vec3(0.0f, 0.f, 0.5f));		// Blue
		//light->GetLight()->SetAmbient(Vec3(0.f, 0.f, 0.1f));
		//light->GetLight()->SetSpecular(Vec3(0.f, 0.f, 0.1f));
		//light->GetLight()->SetLightRange(200.f);
		//light->GetLight()->SetLightAngle(XM_PI / 4);
		//scene->AddGameObject(light);
	}
#pragma endregion

#pragma region ParticleSystem
	{
		/*shared_ptr<GameObject> particle = make_shared<GameObject>();
		particle->AddComponent(make_shared<Transform>());
		particle->AddComponent(make_shared<ParticleSystem>());
		particle->SetCheckFrustum(false);
		particle->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 100.f));
		scene->AddGameObject(particle);*/
	}
#pragma endregion

#pragma region Tessellation Test
	{
		shared_ptr<GameObject> gameObject = make_shared<GameObject>();
		gameObject->AddComponent(make_shared<Transform>());
		gameObject->GetTransform()->SetLocalPosition(Vec3(0, 0, 300));
		gameObject->GetTransform()->SetLocalScale(Vec3(100, 100, 100));
		gameObject->GetTransform()->SetLocalRotation(Vec3(0, 0, 0));

		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
			meshRenderer->SetMaterial(GET_SINGLE(Resources)->Get<Material>(L"Tessellation"));
		}
		gameObject->AddComponent(meshRenderer);

		scene->AddGameObject(gameObject);
	}
#pragma endregion

	return scene;
}

