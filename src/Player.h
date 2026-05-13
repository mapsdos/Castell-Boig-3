#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE

#include "Sprite.h"
#include "TileMap.h"
#include "Bullet.h"
#include "Bomb.h"

class LevelScene;

class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
	void update(int deltaTime, bool wait);
	void render(const glm::mat4& modelview);

	void setTileMap(TileMap* tileMap);
	void setPosition(const glm::vec2& pos);

	void setLives(int lives);
	void heal(int amount);
	int getLives() { return lives; }
	void updateHeartPositions();
	void receiveDamage(int amount);
	glm::ivec2 getPosition() const {return posPlayer;};

	bool isGodMode() const { return godMode; }
	Sprite* getSprite() const { return sprite; }
	void stopJumping();
	void startHurtAnimation(bool enemyToRight);
	void updateHurtLogic(int deltaTime);
	bool isHurting()    const { return bHurting; }
	bool isInvincible() const { return invincibilityTimer > 0.f; }
	static const float HURT_DURATION;         // 625ms = 5 frames @ 8fps
	static const float INVINCIBILITY_DURATION;

	void startDoorEnterAnimation();
	void addBullet() { ++bullets; };
	void addBomb() { ++bombs; };
	void addKey(int numOfKeys) { keys += numOfKeys; };

	int getKeyCount() const { return keys; }
	int getBulletCount() const { return bullets; }
	int getBombCount() const { return bombs; }

	void playerEvent(LevelScene* levelScene, int deltaTime);
private:
	void handleClimbing();
	bool handleHorizontalMovement();
	void updateFloatingLogic();
	void updateGravityLogic(bool moving);
	void updateGodModeLogic(int deltaTime);

private:

	ShaderProgram* program;
	bool bFloating = false;
	bool bJumping = false;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Texture spritesheetStand;
	Sprite* sprite;
	Sprite* spriteStand;
	TileMap* map;

	bool  bHurting = false;
	float knockbackVelX = 0.f;
	float hurtTimer = 0.f;
	float invincibilityTimer = 0.f;

	int lives;
	Texture heartTexture;
	Sprite* heartSprites[3];

	int bullets, bombs, keys;
	int actionTimer;

	bool godMode = false;
	bool godModeActivating = false;
	float godModeActivationTimer = 0.f;
	bool facingLeft = false;
	bool prevGKeyDown = false;
	int  godHoverOffset = 0;   // píxeles visuales por encima del suelo

	static const float GOD_ACTIVATE_DURATION; // ms que dura la animación de activación
	static const int   HOVER_PIXELS;          // píxeles de levitación visual
	static const float SHOOT_DURATION;        // ms que dura la animación de disparo

	float walkStepTimer = 0.0f;
	const float STEP_INTERVAL = 650.0f;

	// Shooting animation state
	bool bShooting = false;
	float shootTimer = 0.f;
	bool pendingBullet = false;
};

#endif // _PLAYER_INCLUDE