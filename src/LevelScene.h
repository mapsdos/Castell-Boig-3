#ifndef _LEVEL_SCENE_INCLUDE
#define _LEVEL_SCENE_INCLUDE

#include <vector>
#include <string>
#include "Scene.h"
#include "TileMap.h"
#include "Player.h"
#include "Entity.h"
#include "Stairs.h"
#include "Enemy.h"

class Door;

class LevelScene : public Scene
{
public:
	LevelScene();
	~LevelScene();

	// "override" tells the compiler these replace the base Scene versions
	void init() override;
	void init(string path);
	void update(int deltaTime) override;
	void render() override;
	glm::vec2 findDoorPosition(int numDoor);
	void setCooldown();
	void setPlayer(Player* newPlayer);
	TileMap* getMap() const { return map; }
	void LoadEnemies();
	void setDoorNum(int numDoor);

private:
	TileMap* map;
	Player* player;
	std::vector<Entity*> items;
	std::vector<Stairs*> stairs;
	std::vector<Door*> doors;
	std::vector<Enemy*> enemies;
	float stairCooldown = 0.0f;
	const float STAIR_DELAY = 100.0f;
	int doorNum;
	bool   enteringDoor = false;
	float  enterAnimTimer = 0.f;
	const float ENTER_ANIM_DURATION = 375.f; // 3 frames × (1000/8) ms
	Door* pendingDoor = nullptr;
};

#endif