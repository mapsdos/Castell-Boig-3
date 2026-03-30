#include <cmath>
#include <iostream>
#include "GraphicsConfig.h"
#include "Player.h"
#include "Game.h"

#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT     96
#define FALL_STEP       4

const float Player::GOD_ACTIVATE_DURATION = 875.f; // 7 frames × (1000/8) ms
const int   Player::HOVER_PIXELS = 4;
const float Player::HURT_DURATION = 625.f;  // 5 frames × (1000/8) ms
const float Player::INVINCIBILITY_DURATION = 1500.f;

enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT,
	ASCEND, DESCEND, ENTER, CLIMB, HANG,
	GOD_ACTIVATE_RIGHT,  // frames 32-38
	GOD_MOVE_RIGHT,      // frame 38 (estático)
	GOD_ACTIVATE_LEFT,   // frames 39-45
	GOD_MOVE_LEFT,        // frame 45 (estático)
	HURT_FROM_RIGHT,
	HURT_FROM_LEFT
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
	updateHeartPositions();
}

void Player::updateHeartPositions() {
	for (int i = 0; i < 3; i++) {
		if (i < lives) {
			if (heartSprites[i]->animation() != 0)
				heartSprites[i]->changeAnimation(0);
		}
		else {
			if (heartSprites[i]->animation() != 1)
				heartSprites[i]->changeAnimation(1);
		}
		heartSprites[i]->setPosition(glm::vec2(10 + i * 15, 10));
	}
}

void Player::receiveDamage(int amount)
{
	if (godMode) return; // inmune
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
	int   numFrames = 56;
	float frameWidthUV = 1.0f / numFrames;
	float frameHeightUV = 1.0f;

	bJumping = false;
	spritesheet.loadFromFile("assets/images/sprites bob.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(
		glm::ivec2(widthFrame, heightFrame),
		glm::vec2(frameWidthUV, frameHeightUV),
		&spritesheet, &shaderProgram
	);
	sprite->setNumberAnimations(15);

	// ── Animaciones originales ─────────────────────────────────────────────
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

	// ── God mode – derecha (frames 32-38) ─────────────────────────────────
	sprite->setAnimationSpeed(GOD_ACTIVATE_RIGHT, 8);
	for (int i = 32; i < 39; i++)
		sprite->addKeyframe(GOD_ACTIVATE_RIGHT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(GOD_MOVE_RIGHT, 8);
	sprite->addKeyframe(GOD_MOVE_RIGHT, glm::vec2(38 * frameWidthUV, 0.f)); // frame 38

	// ── God mode – izquierda (frames 39-45) ───────────────────────────────
	sprite->setAnimationSpeed(GOD_ACTIVATE_LEFT, 8);
	for (int i = 39; i < 46; i++)
		sprite->addKeyframe(GOD_ACTIVATE_LEFT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(GOD_MOVE_LEFT, 8);
	sprite->addKeyframe(GOD_MOVE_LEFT, glm::vec2(45 * frameWidthUV, 0.f)); // frame 45

	sprite->setAnimationSpeed(HURT_FROM_RIGHT, 8);
	for (int i = 46; i < 51; i++)
		sprite->addKeyframe(HURT_FROM_RIGHT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->setAnimationSpeed(HURT_FROM_LEFT, 8);
	for (int i = 51; i < 56; i++)
		sprite->addKeyframe(HURT_FROM_LEFT, glm::vec2(i * frameWidthUV, 0.f));

	sprite->changeAnimation(STAND_RIGHT);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

void Player::startHurtAnimation(bool enemyToRight)
{
	bHurting = true;
	hurtTimer = 0.f;
	bJumping = false;
	// enemyToRight → el enemigo está a la derecha → el personaje sale hacia la izquierda
	knockbackVelX = enemyToRight ? -0.10f : 0.10f;
	sprite->changeAnimation(enemyToRight ? HURT_FROM_RIGHT : HURT_FROM_LEFT);
	invincibilityTimer = INVINCIBILITY_DURATION;
}

void Player::updateHurtLogic(int deltaTime)
{
	hurtTimer += (float)deltaTime;

	// Si la animación termina, forzar el último frame
	if (hurtTimer >= HURT_DURATION) {
		// 5 keyframes: forzar el último (índice 4)
		sprite->setFrame(sprite->animation(), 4);
		bHurting = false;
		return;
	}

	sprite->update(deltaTime);

	// Knockback horizontal con colisión
	int dx = (int)(knockbackVelX * deltaTime);
	posPlayer.x += dx;
	if (knockbackVelX < 0.f && map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32)))
		posPlayer.x -= dx;
	else if (knockbackVelX > 0.f && map->collisionMoveRight(posPlayer, glm::ivec2(24, 32)))
		posPlayer.x -= dx;

	// Gravedad durante el knockback
	posPlayer.y += FALL_STEP;
	map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);

	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

void Player::update(int deltaTime, bool wait)
{
	if (invincibilityTimer > 0.f)
		invincibilityTimer -= (float)deltaTime;

	sprite->update(deltaTime);
	for (int i = 0; i < 3; i++) heartSprites[i]->update(deltaTime);

	// ── Toggle god mode en flanco de subida de G ───────────────────────────
	bool gKeyDown = Game::instance().getKey(GLFW_KEY_G);
	if (gKeyDown && !prevGKeyDown)
	{
		godMode = !godMode;
		if (godMode)
		{
			bJumping = false;
			godModeActivating = true;
			godModeActivationTimer = 0.f;
			godHoverOffset = 0;
			sprite->changeAnimation(facingLeft ? GOD_ACTIVATE_LEFT : GOD_ACTIVATE_RIGHT);
		}
		else
		{
			godModeActivating = false;
			godHoverOffset = 0;
			sprite->changeAnimation(facingLeft ? STAND_LEFT : STAND_RIGHT);
		}
	}
	prevGKeyDown = gKeyDown;

	if (!wait)
	{
		if (godMode)
		{
			updateGodModeLogic(deltaTime);
		}
		else
		{
			int tileId = map->getTileIdAt(posPlayer + glm::ivec2(12, 30));
			if (tileId == 3) handleClimbing();
			else {
				bool moving = handleHorizontalMovement();
				if (bJumping) updateJumpLogic(moving);
				else          updateGravityLogic(moving);
			}
		}
	}

	// Posición del sprite: desplazamiento visual de levitación solo en god mode
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer) - glm::vec2(0.f, (float)godHoverOffset));
}

void Player::updateGodModeLogic(int deltaTime)
{
	// ── Fase de activación: esperar a que termine la animación ─────────────
	if (godModeActivating)
	{
		godModeActivationTimer += deltaTime;
		if (godModeActivationTimer >= GOD_ACTIVATE_DURATION)
		{
			godModeActivating = false;
			sprite->changeAnimation(facingLeft ? GOD_MOVE_LEFT : GOD_MOVE_RIGHT);
		}
		return; // sin movimiento durante la activación
	}

	// ── Movimiento horizontal ──────────────────────────────────────────────
	bool moving = false;
	if (Game::instance().getKey(GLFW_KEY_LEFT))
	{
		facingLeft = true;
		moving = true;
		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32)))
			posPlayer.x += 2;
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		facingLeft = false;
		moving = true;
		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32)))
			posPlayer.x -= 2;
	}

	// ── Animación: siempre el frame estático del god mode ─────────────────
	int godAnim = facingLeft ? GOD_MOVE_LEFT : GOD_MOVE_RIGHT;
	if (sprite->animation() != godAnim)
		sprite->changeAnimation(godAnim);

	// ── Física: idéntica al modo normal ───────────────────────────────────
	if (bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		posPlayer.y = startY - (int)(JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

		if (jumpAngle > 90)
		{
			if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y))
				stopJumping();
		}
		if (jumpAngle >= 180) stopJumping();
	}
	else
	{
		posPlayer.y += FALL_STEP;
		bool onGround = map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);

		if (onGround)
		{
			// Levitación: offset visual solo cuando está en el suelo y moviéndose
			godHoverOffset = HOVER_PIXELS;

			if (Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
				godHoverOffset = 0; // sin levitación mientras salta
			}
		}
		else
		{
			godHoverOffset = 0; // en el aire no hay offset
		}
	}
}

void Player::handleClimbing()
{
	bJumping = false;
	bool movingVertically = false;

	if (Game::instance().getKey(GLFW_KEY_UP)) {
		int tileAbove = map->getTileIdAt(posPlayer + glm::ivec2(12, 8));
		if (tileAbove == 3) { posPlayer.y -= 2; movingVertically = true; }
	}
	else if (Game::instance().getKey(GLFW_KEY_DOWN)) {
		posPlayer.y += 2;
		map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);
		movingVertically = true;
	}

	int targetAnim = movingVertically ? CLIMB : HANG;
	if (sprite->animation() != targetAnim) sprite->changeAnimation(targetAnim);

	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		facingLeft = true;
		posPlayer.x -= 1;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) posPlayer.x += 1;
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		facingLeft = false;
		posPlayer.x += 1;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32))) posPlayer.x -= 1;
	}
}

bool Player::handleHorizontalMovement()
{
	bool moving = false;

	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		facingLeft = true;
		moving = true;
		if (sprite->animation() != MOVE_LEFT) sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) {
			posPlayer.x += 2;
			if (!bJumping && sprite->animation() != STAND_LEFT) sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		facingLeft = false;
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

void Player::updateJumpLogic(bool moving)
{
	jumpAngle += JUMP_ANGLE_STEP;
	posPlayer.y = startY - (int)(JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

	if (jumpAngle > 90) {
		if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y))
			stopJumping();
	}
	if (jumpAngle >= 180) stopJumping();

	if (bJumping) {
		int targetAnim = (jumpAngle < 90) ? ASCEND : DESCEND;
		if (sprite->animation() != targetAnim) sprite->changeAnimation(targetAnim);
	}
	else {
		if (moving) {
			int moveAnim = Game::instance().getKey(GLFW_KEY_LEFT) ? MOVE_LEFT : MOVE_RIGHT;
			if (sprite->animation() != moveAnim) sprite->changeAnimation(moveAnim);
		}
		else {
			int standAnim = (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT)
				? STAND_LEFT : STAND_RIGHT;
			if (sprite->animation() != standAnim) sprite->changeAnimation(standAnim);
		}
	}
}

void Player::updateGravityLogic(bool moving)
{
	posPlayer.y += FALL_STEP;

	if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y)) {
		if (!moving) {
			int standAnim = (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT)
				? STAND_LEFT : STAND_RIGHT;
			if (sprite->animation() != standAnim) sprite->changeAnimation(standAnim);
		}
		if (Game::instance().getKey(GLFW_KEY_UP)) {
			bJumping = true;
			jumpAngle = 0;
			startY = posPlayer.y;
		}
	}
	else {
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

void Player::startDoorEnterAnimation()
{
	sprite->changeAnimation(ENTER);
}

void Player::setTileMap(TileMap* tileMap) { map = tileMap; }

void Player::setPosition(const glm::vec2& pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer));
}

glm::ivec2 Player::getPosition() const { return posPlayer; }

void Player::stopJumping()
{
	bJumping = false;
	jumpAngle = 0;
}