#include <cmath>
#include <iostream>
#include "GraphicsConfig.h"
#include "Player.h"
#include "Game.h"
#include "SFX.h"
#include "LevelScene.h"

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
	program = nullptr;
	sprite = nullptr;
	map = nullptr;
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
	Game::instance().loseLives();
}

void Player::heal(int amount)
{
	setLives(lives + amount);
}

void Player::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
	SFX::instance().loadSound("walk", "assets/audio/robert-walking.wav");
	walkStepTimer = 0.0f;
	program = &shaderProgram;
	lives = Game::instance().getLives();
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

	bullets = bombs = keys =  actionTimer = 0;

	bFloating = false;
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

	// Durante la animación de hurt, solo actualizar hearts, no procesar input/movimiento
	for (int i = 0; i < 3; i++) heartSprites[i]->update(deltaTime);
	
	if (bHurting) {
		return;  // updateHurtLogic es llamado por LevelScene
	}

	sprite->update(deltaTime);

	// --- Toggle god mode (G Key) ---
	bool gKeyDown = Game::instance().getKey(GLFW_KEY_G);
	if (gKeyDown && !prevGKeyDown) {
		godMode = !godMode;
		bFloating = false; // Reset state on toggle
		if (godMode) {
			godModeActivating = true;
			godModeActivationTimer = 0.f;
			sprite->changeAnimation(facingLeft ? GOD_ACTIVATE_LEFT : GOD_ACTIVATE_RIGHT);
		}
		else {
			godModeActivating = false;
			godHoverOffset = 0;
			bFloating = false;
			sprite->changeAnimation(facingLeft ? STAND_LEFT : STAND_RIGHT);
		}
	}
	prevGKeyDown = gKeyDown;

	if (!wait)
	{
		// 1. Pre-check: Are we touching a floor right now?
		int tileAtCenter = map->getTileIdAt(posPlayer + glm::ivec2(12, 16));
		int tileUnderFeet = map->getTileIdAt(posPlayer + glm::ivec2(12, 32));

		// 2. Determine if we should be climbing
		if (tileAtCenter == 3 && (Game::instance().getKey(GLFW_KEY_UP) || tileUnderFeet != 1)) {
			bFloating = false; // Cancel bubble-tile floating if grabbing a vine
			handleClimbing();

			// God Mode Visual override for vines
			if (godMode && !godModeActivating) {
				int godAnim = facingLeft ? GOD_MOVE_LEFT : GOD_MOVE_RIGHT;
				if (sprite->animation() != godAnim) sprite->changeAnimation(godAnim);
			}
		}
		else if (godMode) {
			updateGodModeLogic(deltaTime);
		}
		else {
			// Normal Mode: Bubble Tile vs. Regular Movement
			if (tileUnderFeet == 6 && Game::instance().getKey(GLFW_KEY_UP)) {
				bFloating = true;
			}

			if (bFloating) {
				updateFloatingLogic();
			}
			else {
				walkStepTimer += deltaTime;
				bool moving = handleHorizontalMovement();
				updateGravityLogic(moving);
			}
		}
	}
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer) - glm::vec2(0.f, (float)godHoverOffset));
}

void Player::updateGodModeLogic(int deltaTime)
{
	if (godModeActivating)
	{
		godModeActivationTimer += deltaTime;
		if (godModeActivationTimer >= GOD_ACTIVATE_DURATION)
		{
			godModeActivationTimer = 0.f; // Safety reset
			godModeActivating = false;
			sprite->changeAnimation(facingLeft ? GOD_MOVE_LEFT : GOD_MOVE_RIGHT);
		}
		return;
	}

	// 1. Horizontal Movement (Standard)
	bool movingSide = false;
	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		facingLeft = true;
		movingSide = true;
		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) posPlayer.x += 2;
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		facingLeft = false;
		movingSide = true;
		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32))) posPlayer.x -= 2;
	}

	// 2. Animation Lock
	int godAnim = facingLeft ? GOD_MOVE_LEFT : GOD_MOVE_RIGHT;
	if (sprite->animation() != godAnim) sprite->changeAnimation(godAnim);

	// 3. Jump Pad vs Gravity Logic
	int tileUnderFeet = map->getTileIdAt(posPlayer + glm::ivec2(12, 32));

	// Start Floating (Bubble Lift) if on pad and pressing UP
	if (tileUnderFeet == 6 && Game::instance().getKey(GLFW_KEY_UP)) {
		bFloating = true;
	}

	if (bFloating) {
		// Exit condition: Moving sideways without holding UP (Matches your normal mode)
		if (movingSide && !Game::instance().getKey(GLFW_KEY_UP)) {
			bFloating = false;
		}
		else {
			posPlayer.y -= 4; // Constant upward lift
			godHoverOffset = 0;

			int headY;
			if (map->collisionMoveUp(posPlayer, glm::ivec2(24, 32), &headY)) {
				posPlayer.y = (float)headY;
				bFloating = false;
			}
		}
	}
	else {
		// Normal God Mode Gravity + Hover
		posPlayer.y += FALL_STEP;
		bool onGround = map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);

		if (onGround) {
			godHoverOffset = HOVER_PIXELS;

			// Standard Sine Jump (Not the pad lift)
			if (Game::instance().getKey(GLFW_KEY_UP) && tileUnderFeet != 6) {
				// If you still want the normal God Jump when NOT on a pad:
				// You'd need a separate boolean like 'bJumping' 
				// but for now, this lets the Pad take priority.
			}
		}
		else {
			godHoverOffset = 0;
		}
	}
}

void Player::handleClimbing()
{
	bFloating = false;
	bool movingVertically = false;

	// Movement logic (Keep this active so we can actually move)
	if (Game::instance().getKey(GLFW_KEY_UP)) {
		int tileAbove = map->getTileIdAt(posPlayer + glm::ivec2(12, 8));
		if (tileAbove == 3) { posPlayer.y -= 2; movingVertically = true; }
	}
	else if (Game::instance().getKey(GLFW_KEY_DOWN)) {
		posPlayer.y += 2;
		map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y);
		movingVertically = true;
	}

	// --- ANIMATION GUARD ---
	// Only change to CLIMB/HANG if we are NOT in god mode
	if (!godMode) {
		int targetAnim = movingVertically ? CLIMB : HANG;
		if (sprite->animation() != targetAnim) sprite->changeAnimation(targetAnim);
	}

	// Horizontal movement on vines
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
		if (walkStepTimer >= STEP_INTERVAL) {
			SFX::instance().playSound("walk", 35.f); // Play the alias you loaded in init
			walkStepTimer = 0.0f;
		}
		facingLeft = true;
		moving = true;
		// ONLY change to normal MOVE animation if NOT in god mode
		if (!godMode && sprite->animation() != MOVE_LEFT) sprite->changeAnimation(MOVE_LEFT);

		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(24, 32))) {
			posPlayer.x += 2;
			if (!godMode && !bFloating && sprite->animation() != STAND_LEFT) sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT)) {
		if (walkStepTimer >= STEP_INTERVAL) {
			SFX::instance().playSound("walk", 35.f); // Play the alias you loaded in init
			walkStepTimer = 0.0f;
		}
		facingLeft = false;
		moving = true;
		// ONLY change to normal MOVE animation if NOT in god mode
		if (!godMode && sprite->animation() != MOVE_RIGHT) sprite->changeAnimation(MOVE_RIGHT);

		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(24, 32))) {
			posPlayer.x -= 2;
			if (!godMode && !bFloating && sprite->animation() != STAND_RIGHT) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	return moving;
}

void Player::updateFloatingLogic()
{
	// Exit if moving sideways
	if ((Game::instance().getKey(GLFW_KEY_LEFT) || Game::instance().getKey(GLFW_KEY_RIGHT)) && !Game::instance().getKey(GLFW_KEY_UP)) {
		bFloating = false;
		return;
	}

	posPlayer.y -= 4; // Constant upward speed
	if (sprite->animation() != ASCEND) sprite->changeAnimation(ASCEND);

	int headY;
	if (map->collisionMoveUp(posPlayer, glm::ivec2(24, 32), &headY)) {
		posPlayer.y = headY;
		bFloating = false;
	}
}

void Player::updateGravityLogic(bool moving)
{
	posPlayer.y += FALL_STEP;

	if (map->collisionMoveDown(posPlayer, glm::ivec2(24, 32), &posPlayer.y)) {
		if (!moving) {
			int standAnim = facingLeft ? STAND_LEFT : STAND_RIGHT;
			if (sprite->animation() != standAnim) sprite->changeAnimation(standAnim);
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

void Player::stopJumping()
{
	bFloating = false;
	jumpAngle = 0;
}

void Player::playerEvent(LevelScene* levelScene, int deltaTime)
{
	if (actionTimer > 0)
	{
		actionTimer -= deltaTime;
	}
	else
	{
		if (Game::instance().getKey(GLFW_KEY_S))
		{
			if (bullets > 0)
			{
				--bullets;
				Bullet* bullet = new Bullet();
				// Spawn slightly in front of Spongebob
				glm::vec2 spawnPos = glm::vec2(posPlayer.x + tileMapDispl.x + (!facingLeft ? 24 : 0),
					posPlayer.y + tileMapDispl.y + 12);

				bullet->init(spawnPos, *program, !facingLeft, "assets/images/bubble-pixel-art.png");

				// Adding it to the vector makes it "exist" for the update and render loops
				levelScene->addBullet(bullet);
			}
			actionTimer = 2000;
		}
		if (Game::instance().getKey(GLFW_KEY_B))
		{
			if (bombs > 0)
			{
				--bombs;
				Bomb* bomb = new Bomb();
				glm::vec2 spawnPos = glm::vec2(posPlayer.x + 32, posPlayer.y + 16);
				bomb->init(spawnPos, *program);
				bomb->planted();

				// Adding it to the vector makes it "exist" for the update and render loops
				levelScene->addBomb(bomb);
			}
			actionTimer = 2000;
		}
	}
}