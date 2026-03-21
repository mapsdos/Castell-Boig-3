#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE

#include "Sprite.h"
#include "TileMap.h"

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
	glm::ivec2 getPosition() const;

	bool isGodMode() const { return godMode; }

	void stopJumping();

private:
	void handleClimbing();
	bool handleHorizontalMovement();
	void updateJumpLogic(bool moving);
	void updateGravityLogic(bool moving);
	void updateGodModeLogic(int deltaTime);

private:
	bool bJumping;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Texture spritesheetStand;
	Sprite* sprite;
	Sprite* spriteStand;
	TileMap* map;

	int lives;
	Texture heartTexture;
	Sprite* heartSprites[3];

	bool godMode = false;
	bool godModeActivating = false;
	float godModeActivationTimer = 0.f;
	bool facingLeft = false;
	bool prevGKeyDown = false;
	int  godHoverOffset = 0;   // píxeles visuales por encima del suelo

	static const float GOD_ACTIVATE_DURATION; // ms que dura la animación de activación
	static const int   HOVER_PIXELS;          // píxeles de levitación visual
};

#endif // _PLAYER_INCLUDE