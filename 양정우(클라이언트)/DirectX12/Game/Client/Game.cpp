#include "pch.h"
#include "Game.h"
#include "Engine.h"
#include "SceneManager.h"


void Game::Init(const WindowInfo& info)
{
	GEngine->Init(info);

	GET_SINGLE(SceneManager)->LoadScene(L"TestScene");
}

void Game::Update()
{
	GEngine->Update();
}

void Game::CreateAvatar(int object_type, int object_id, float x, float y, float z, int animation_id, float direction)
{
	GET_SINGLE(SceneManager)->CreateAvatar();
}

void Game::CreateObject(int object_type, int object_id, float x, float y, float z, int animation_id, float direction)
{
	GET_SINGLE(SceneManager)->CreateObject();
}

void Game::ChangeObjectLocation(int object_id, float x, float y, float z, float direction)
{
	GET_SINGLE(SceneManager)->ChangeObjectLocation();
}
