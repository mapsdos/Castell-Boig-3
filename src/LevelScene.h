#ifndef _LEVEL_SCENE_INCLUDE
#define _LEVEL_SCENE_INCLUDE

#include <vector>
#include <string>
#include "Scene.h"
#include "TileMap.h"
#include "Player.h"
#include "Entity.h"
#include "Key.h"
#include "BubbleGun.h"
#include "Bomb.h"
#include "Stairs.h"
#include "Enemy.h"
#include "Weight.h"
#include "Clock.h"

class Door;

class LevelScene : public Scene
{
public:
	LevelScene();
	LevelScene(string setPath, Player* setPlayer);
	~LevelScene();

	// "override" tells the compiler these replace the base Scene versions
	void init() override;
	void init(string levelPath);
	void update(int deltaTime) override;
	void render() override;
	glm::vec2 findDoorPosition(int numDoor);
	void setCooldown();
	void setPlayer(Player* newPlayer);
	TileMap* getMap() const { return map; }
	void setDoorNum(int numDoor);
	void LoadEnemies();

	int totalNumKeys();
	void collectKeys();
	int numKeys() { return keys.size(); };
	void clearKeys() { keys.clear(); };
	void addBullet(Bullet* bullet) { bulletsFired.push_back(bullet); };
	void addBomb(Bomb* bomb) { bombsPlaced.push_back(bomb); };

private:
	void clearLevel();

private:
	TileMap* map;
	Player* player;
	std::vector<Key*> keys;
	std::vector<Entity*> items;
	std::vector<Entity*> initialItems;
	std::vector<Stairs*> stairs;
	std::vector<Door*> doors;
	std::vector<Enemy*> enemies;
	std::vector<Bullet*> bulletsFired;
	std::vector<Bomb*> bombsPlaced;
	std::vector<Weight*> weights;
	float stairCooldown = 0.0f;
	const float STAIR_DELAY = 100.0f;
	int doorNum;
	bool   enteringDoor = false;
	float  enterAnimTimer = 0.f;
	const float ENTER_ANIM_DURATION = 375.f; // 3 frames × (1000/8) ms
	Door* pendingDoor = nullptr;
	// ── Hit / Death state ──────────────────────────────────────────────────
	bool  playerHurting = false;
	bool  hurtPaused = false;      // pausa en el último frame de la animación hurt
	float hurtPauseTimer = 0.f;
	bool  fadingOut = false;
	bool  youDied = false;          // muestra filtro gris + "you died"
	bool  youDiedFading = false;    // fade out después de "you died"
	float fadeAlpha = 0.f;
	float fadeTimer = 0.f;
	float youDiedTimer = 0.f;

	const float HURT_PAUSE_DURATION = 500.f;  // pausa en último frame antes del fade
	const float FADE_DURATION = 1000.f;
	const float YOU_DIED_DURATION = 2000.f;   // tiempo mostrando "you died" con filtro gris
	const float YOU_DIED_FADE_DURATION = 1000.f;  // fade out después de "you died"

	// Overlay sprite (1×1 pixel blanco escalado a pantalla)
	Sprite* fadeSprite = nullptr;
	Texture  fadeTexture;
	Sprite* youDiedSprite = nullptr;
	Texture  youDiedTexture;
	bool stoppedTime;
	int timeStopped;
	string path;
};

#endif