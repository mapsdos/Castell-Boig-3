#ifndef _LEVEL_SCENE_INCLUDE
#define _LEVEL_SCENE_INCLUDE

#include "Scene.h"
#include "TileMap.h"
#include "Player.h"

class LevelScene : public Scene
{
public:
	LevelScene();
	~LevelScene();

	// "override" tells the compiler these replace the base Scene versions
	void init() override;
	void update(int deltaTime) override;
	void render() override;

private:
	TileMap* map;
	Player* player;

};

#endif