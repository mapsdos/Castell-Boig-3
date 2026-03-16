#include <cmath>
#include <iostream>
#include "GraphicsConfig.h"
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, ASCEND, DESCEND, ENTER, CLIMB, HANG
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
			if (heartSprites[i]->animation() != 0)
				heartSprites[i]->changeAnimation(0); // Cor ple
		}
		else {
			if (heartSprites[i]->animation() != 1)
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

void Player::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
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

		heartSprites[i]->setAnimationSpeed(0, 1);
		heartSprites[i]->addKeyframe(0, glm::vec2(0.f, 0.f));

		heartSprites[i]->setAnimationSpeed(1, 1);
		heartSprites[i]->addKeyframe(1, glm::vec2(heartWidthUV, 0.f));

		heartSprites[i]->changeAnimation(0);
	}

	updateHeartPositions();

	float widthFrame = 24.0f;
	float heightFrame = 32.0f;
	int numFrames = 32;
	float frameWidthUV = widthFrame / (widthFrame * numFrames);
	float frameHeightUV = heightFrame / heightFrame;

	bJumping = false;
	spritesheet.loadFromFile("assets/images/sprites bob.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(widthFrame, heightFrame), glm::vec2(frameWidthUV, frameHeightUV), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(9);

	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.f, 0.f));

	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, glm::vec2(frameWidthUV, 0.f));

	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	for (int i = 2; i < 10; i++)
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	for (int i = 10; i < 18; i++)
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(ASCEND, 8);
	for (int i = 18; i < 22; i++)
		sprite->addKeyframe(ASCEND, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(DESCEND, 8);
	for (int i = 22; i < 26; i++)
		sprite->addKeyframe(DESCEND, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(ENTER, 8);
	for (int i = 26; i < 29; i++)
		sprite->addKeyframe(ENTER, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(CLIMB, 8);
	for (int i = 29; i < 31; i++)
		sprite->addKeyframe(CLIMB, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(HANG, 8);
	sprite->addKeyframe(HANG, glm::vec2(31 * frameWidthUV, 0.f));

	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

void Player::update(int deltaTime, bool wait) {
	sprite->update(deltaTime);
	for (int i = 0; i < 3; i++) heartSprites[i]->update(deltaTime);
	if (!wait)
	{
		int tileId = map->getTileIdAt(posPlayer + glm::ivec2(12, 30));

		if (tileId == 3) {
			handleClimbing();
		}
		else {
			bool moving = handleHorizontalMovement();
			if (bJumping) updateJumpLogic(moving);
			else updateGravityLogic(moving);
		}
	}
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

void Player::handleClimbing() {
	bJumping = false;
	bool movingVertically = false;

	// Vertical Movement
	if (Game::instance().getKey(GLFW_KEY_UP)) {
		int tileAbove = map->getTileIdAt(posPlayer + glm::ivec2(12, 8)); // Check head level
		if (tileAbove == 3) {
			posPlayer.y -= 2;
			movingVertically = true;
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_DOWN)) {
		posPlayer.y += 2;
		// Check floor so we don't climb into the basement
		map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);
		movingVertically = true;
	}

	int targetAnim = movingVertically ? CLIMB : HANG;
	if (sprite->animation() != targetAnim) {
		sprite->changeAnimation(targetAnim);
	}

	// Allow limited horizontal movement on vines
	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		posPlayer.x -= 1;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) posPlayer.x += 1;
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		posPlayer.x += 1;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32))) posPlayer.x -= 1;
	}
}

bool Player::handleHorizontalMovement() {
	bool moving = false;

	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		moving = true;
		if (sprite->animation() != MOVE_LEFT) sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) {
			posPlayer.x += 2;
			if (!bJumping && sprite->animation() != STAND_LEFT) sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		moving = true;
		if (sprite->animation() != MOVE_RIGHT) sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32))) {
			posPlayer.x -= 2;
			if (!bJumping && sprite->animation() != STAND_RIGHT) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	return moving;
}

void Player::updateJumpLogic(bool moving) {
	jumpAngle += JUMP_ANGLE_STEP;
	posPlayer.y = startY - (int)(JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

	if (jumpAngle > 90) {
		if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y)) {
			stopJumping();
		}
	}

	if (jumpAngle >= 180) stopJumping();

	if (bJumping) {
		// FIX: Guard the jump animations
		int targetAnim = (jumpAngle < 90) ? ASCEND : DESCEND;
		if (sprite->animation() != targetAnim) sprite->changeAnimation(targetAnim);
	}
	else {
		// Transition back
		if (moving) {
			int moveAnim = Game::instance().getKey(GLFW_KEY_LEFT) ? MOVE_LEFT : MOVE_RIGHT;
			if (sprite->animation() != moveAnim) sprite->changeAnimation(moveAnim);
		}
		else {
			int standAnim = (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT) ? STAND_LEFT : STAND_RIGHT;
			if (sprite->animation() != standAnim) sprite->changeAnimation(standAnim);
		}
	}
}

// Logic for gravity and jumping
void Player::updateGravityLogic(bool moving) {
	posPlayer.y += FALL_STEP;

	if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y)) {
		if (!moving) {
			int standAnim = (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT) ? STAND_LEFT : STAND_RIGHT;
			if (sprite->animation() != standAnim) sprite->changeAnimation(standAnim);
		}

		if (Game::instance().getKey(GLFW_KEY_UP)) {
			bJumping = true;
			jumpAngle = 0;
			startY = posPlayer.y;
		}
	}
	else {
		// Falling off a ledge
		if (sprite->animation() != DESCEND) sprite->changeAnimation(DESCEND);
	}
}

void Player::render(const glm::mat4& modelview)
{
	sprite->render(modelview);

	glm::mat4 identity = glm::mat4(1.0f);
	for (int i = 0; i < 3; i++)
		heartSprites[i]->render(identity);
}

void Player::setTileMap(TileMap* tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2& pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

glm::ivec2 Player::getPosition() const
{
	return posPlayer;
}

void Player::stopJumping()
{
	bJumping = false;
	jumpAngle = 0;
}