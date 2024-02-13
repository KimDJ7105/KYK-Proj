#pragma once

using std::shared_ptr;

class Scene;
class GameObject;

class SceneManager
{
	DECLARE_SINGLE(SceneManager);

public:
	void Update();
	void Render();
	void LoadScene(std::wstring sceneName);

public:
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }

private:
	shared_ptr<Scene> LoadTestScene();

private:
	shared_ptr<Scene> _activeScene;

	shared_ptr<GameObject> _player;

	std::vector<shared_ptr<GameObject>> _otherPlayer;
	// TODO: bool ���� �÷��̾ ���������� �˼��ִ� �Ǵܺ���
};

