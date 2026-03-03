#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime, bool wait);
	void render(const glm::mat4 &modelview);
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);

	void setLives(int lives);
	void receive(int amount);
	void heal(int amount);
	int getLives() { return lives; }
	void updateHeartPositions();
	void receiveDamage(int amount);
	glm::ivec2 getPosition() const;

	void stopJumping();
	private:
    void handleClimbing();
    bool handleHorizontalMovement();
    void updateJumpLogic(bool moving);
    void updateGravityLogic(bool moving);
	
private:
	bool bJumping;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Texture spritesheetStand;
	Sprite *sprite;
	Sprite* spriteStand;
	TileMap *map;

	int lives;
	Texture heartTexture;
	Sprite* heartSprites[3];
};


#endif // _PLAYER_INCLUDE


