#pragma once

class SESSION;

class Game
{
public:

	void Init(const WindowInfo& info);
	void Update();

	void CreateAvatar(int object_type, int object_id, float x, float y, float z, int animation_id, float direction);
	void CreateObject(int object_type, int object_id, float x, float y, float z, int animation_id, float direction);
	void ChangeObjectLocation(int object_id, float x, float y, float z, float direction);

	void MoveThisSession(tcp::socket& sock);

	SESSION* GetGameSession();
};

