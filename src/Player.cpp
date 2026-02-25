#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, ASCEND, DESCEND, ENTER, CLIMB
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;

	for (int i = 0; i < 3; i++) {
		if (heartSprites[i] != NULL)
			delete heartSprites[i];
	}
}

void Player::setLives(int lives) {
	if (lives >= 0 && lives <= 3) this->lives = lives;
	else if (lives == 0) //Implementar mort
	updateHeartPositions();
}

void Player::updateHeartPositions() {
	for (int i = 0; i < 3; i++) {
		if (i < lives) {
			heartSprites[i]->changeAnimation(0); // Cor ple
		} 
		else {
			heartSprites[i]->changeAnimation(1); // Cor buit
		}
		glm::vec2 heartPos = glm::vec2(10 + i * 15, 10);
		heartSprites[i]->setPosition(heartPos);
	}
}

void Player::receiveDamage(int amount)
{
	setLives(lives - amount);
}

void Player::heal(int amount)
{
	setLives(lives + amount);
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	lives = 3;
	heartTexture.loadFromFile("assets/images/hearts.png", TEXTURE_PIXEL_FORMAT_RGBA);
	float heartWidthUV = 12.0f / 24.0f;
	float heartHeightUV = 12.0f / 12.0f;

	for (int i = 0; i < 3; i++) {
		heartSprites[i] = Sprite::createSprite(
			glm::ivec2(12, 12),                       
			glm::vec2(heartWidthUV, heartHeightUV),   
			&heartTexture,
			&shaderProgram
		);

		heartSprites[i]->setNumberAnimations(2);

		//Cor ple
		heartSprites[i]->setAnimationSpeed(0, 1);
		heartSprites[i]->addKeyframe(0, glm::vec2(0.f, 0.f));

		//Cor buit
		heartSprites[i]->setAnimationSpeed(1, 1);
		heartSprites[i]->addKeyframe(1, glm::vec2(heartWidthUV, 0.f));  // (0.5, 0)

		//Posar com a default es cors plens
		heartSprites[i]->changeAnimation(0);
	}

	float widthFrame = 24.0f;
	float heightFrame = 32.0f;
	int numFrames = 31;
	float frameWidthUV = widthFrame / (widthFrame*numFrames);
	float frameHeightUV = heightFrame / heightFrame;

	bJumping = false;
	spritesheet.loadFromFile("assets/images/sprites bob.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(widthFrame, heightFrame), glm::vec2(frameWidthUV, frameHeightUV), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(8);
	
	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.f, 0.f));

	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, glm::vec2(frameWidthUV, 0.f));
		
	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	for (int i = 2; i < 10; i++) {
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(i * frameWidthUV, 0.f));
	}

	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	for (int i = 10; i < 18; i++) {
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(i * frameWidthUV, 0.f));
	}

	//bob ascending
	sprite->setAnimationSpeed(ASCEND, 8);
	for (int i = 18; i < 22; i++) {
		sprite->addKeyframe(ASCEND, glm::vec2(i * frameWidthUV, 0.f));
	}
	
	//bob descending
	sprite->setAnimationSpeed(DESCEND, 8);
	for (int i = 22; i < 26; i++) {
		sprite->addKeyframe(DESCEND, glm::vec2(i * frameWidthUV, 0.f));
	}

	//bob door
	sprite->setAnimationSpeed(ENTER, 8);
	for (int i = 26; i < 29; i++) {
		sprite->addKeyframe(ENTER, glm::vec2(i * frameWidthUV, 0.f));
	}

	//bob stairs
	sprite->setAnimationSpeed(CLIMB, 8);
	for (int i = 29; i < 31; i++) {
		sprite->addKeyframe(CLIMB, glm::vec2(i * frameWidthUV, 0.f));
	}

	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);

	for (int i = 0; i < 3; i++) {
		heartSprites[i]->update(deltaTime);
	}

	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if(sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if(sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
	}
	
	if(bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			sprite->changeAnimation(ASCEND);
			bJumping = false;
			posPlayer.y = startY;
		}
		else
		{
			sprite->changeAnimation(DESCEND);
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			if(Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
			}
		}
	}
	updateHeartPositions();
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();

	for (int i = 0; i < 3; i++) {
		heartSprites[i]->render();
	}
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}