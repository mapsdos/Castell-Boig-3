#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelScene.h"
#include "Key.h"
#include "Door.h"
#include "KeyDoor.h"
#include "Game.h"
#include "Patroller.h"
#include "Shooter.h"
#include "Follower.h"
#include "Explosion.h"
#include "SFX.h"

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25

namespace {
	template<typename EntityType> void createEntity(std::vector<Enemy*>& entities, TileMap* map, ShaderProgram& texProgram, int i, int j) {
		EntityType* entity = new EntityType();
		entity->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		entity->setPosition(glm::vec2(i * map->getTileSize() - SCREEN_X, (j * map->getTileSize()) - SCREEN_Y));
		entity->setTileMap(map);
		entities.push_back(entity);
	}
}


LevelScene::LevelScene()
{
	map = NULL;
	player = NULL;
	doorNum = 0;
	path = "";
	keyIcon = nullptr;
	bubbleIcon = nullptr;
	bombIcon = nullptr;
	numberSprite = nullptr;
}

LevelScene::LevelScene(string setPath, Player* setPlayer)
{
	map = NULL;
	player = setPlayer;
	doorNum = 0;
	path = setPath;
	keyIcon = nullptr;
	bubbleIcon = nullptr;
	bombIcon = nullptr;
	numberSprite = nullptr;
}

LevelScene::~LevelScene()
{
	texProgram.free();
	if (map != nullptr)
		delete map;
	if (player != NULL)
		delete player;
}

void LevelScene::init()
{
	clearLevel();
	initShaders();

	SFX::instance().init();

	// 2. Load the actual files (Make sure the paths match your folders!)
	SFX::instance().loadSound("explosion", "assets/audio/explosion-sfx.wav");
	map = TileMap::createTileMap(path, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	vector<string> const& roomFiles = map->getRoomFiles();
	unsigned roomSize = roomFiles.size();
	doorNum = 0;
	stoppedTime = false;
	timeStopped = 0;
	firstCall = true;

	// Reset death/hurt states
	playerHurting = false;
	hurtPaused = false;
	hurtPauseTimer = 0.f;
	fadingOut = false;
	fadeTimer = 0.f;
	fadeAlpha = 0.f;
	youDied = false;
	youDiedTimer = 0.f;
	youDiedFading = false;

	explosionTexture.loadFromFile("assets/images/explosion1.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionTexture.setMinFilter(GL_NEAREST);
	explosionTexture.setMagFilter(GL_NEAREST);

	keyHudTex.loadFromFile("assets/images/pixel-key.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bubbleHudTex.loadFromFile("assets/images/bubble-blower-stick-pixel-art.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombHudTex.loadFromFile("assets/images/pixel-art-bomb-off.png", TEXTURE_PIXEL_FORMAT_RGBA);
	numbersTex.loadFromFile("assets/images/numbers.png", TEXTURE_PIXEL_FORMAT_RGBA);

	// Set filters to NEAREST for sharp pixel art
	keyHudTex.setMinFilter(GL_NEAREST); keyHudTex.setMagFilter(GL_NEAREST);
	bubbleHudTex.setMinFilter(GL_NEAREST); bubbleHudTex.setMagFilter(GL_NEAREST);
	bombHudTex.setMinFilter(GL_NEAREST); bombHudTex.setMagFilter(GL_NEAREST);
	numbersTex.setMinFilter(GL_NEAREST); numbersTex.setMagFilter(GL_NEAREST);

	// 2. Create the HUD Sprites
	glm::vec2 fullUV(1.0f, 1.0f); // Icons are full single images
	keyIcon = Sprite::createSprite(glm::ivec2(16, 16), fullUV, &keyHudTex, &texProgram);
	bubbleIcon = Sprite::createSprite(glm::ivec2(16, 16), fullUV, &bubbleHudTex, &texProgram);
	bombIcon = Sprite::createSprite(glm::ivec2(16, 16), fullUV, &bombHudTex, &texProgram);

	// 3. Create the Numbers Sprite (10 frames side-by-side in one file)
	// The spritesheet width is divided into 10 frames, so UV step is 0.1
	numberSprite = Sprite::createSprite(glm::ivec2(12, 12), glm::vec2(0.1f, 1.0f), &numbersTex, &texProgram);
	numberSprite->setNumberAnimations(10);
	for (int i = 0; i < 10; i++) {
		// Frame 0 is the '0', Frame 1 is the '1', etc.
		numberSprite->addKeyframe(i, glm::vec2(i * 0.1f, 0.0f));
	}

	glm::ivec2 size = map->getMapSize();
	for (int j = 0; j < size.y; j++) {
		for (int i = 0; i < size.x; i++) {
			// Check the map data for the key ID (2)
			int tileId = map->getTileIdAt(glm::ivec2(i * map->getTileSize(), j * map->getTileSize()));
			// Calculate pixel position: (Column * TileSize, Row * TileSize)
				// Add SCREEN_X/Y if your map has an offset
				// In LevelScene.cpp init()
			float x = static_cast<float>(SCREEN_X + (i * map->getTileSize()));
			// Subtract 16 (or the difference between sprite height and tile height) 
			// to pull the key "up" out of the floor
			float y = static_cast<float>(SCREEN_Y + (j * map->getTileSize()) - (32 - map->getTileSize()));
			switch (tileId)
			{
			case 5:
			{
				Key* newKey = new Key();
				// Keys are 20x13, position them lower (+12 pixels)
				float keyY = y + 12.f;
				newKey->init(glm::vec2(x, keyY), texProgram);
				keys.push_back(newKey);
				break;
			}
			case 4:
				// Inside the tile loop where tileId == 4
			{
				Door* newDoor = new Door();
				newDoor->init(glm::vec2(x, y), texProgram);

				if (roomSize > 0) {
					// Create a WHOLE NEW SCENE for the room
					LevelScene* roomScene = new LevelScene(roomFiles.at(roomSize - 1), player); // Custom init that takes a filename
					roomScene->init();
					roomScene->doors[0]->setRoom(this);
					newDoor->setRoom(roomScene);
					roomScene->enemies.clear();
					roomScene->setDoorNum(roomFiles.size() - roomSize + 1);

					// IMPORTANT: The door inside the roomScene needs to point BACK to 'this'
					// You'll need a logic to link them back to the current LevelScene

					--roomSize;
				}
				doors.push_back(newDoor);
				break;
			}
			case 10:
			{
				KeyDoor* newKeyDoor = new KeyDoor();
				newKeyDoor->init(glm::vec2(x, y), texProgram);

				doors.insert(doors.begin(), newKeyDoor);
				break;
			}
			case 7:
			{
				BubbleGun* newBubbleGun = new BubbleGun();
				newBubbleGun->init(glm::vec2(x, y), texProgram);
				items.push_back(newBubbleGun);
				break;
			}
			case 8:
			{
				Bomb* newBomb = new Bomb();
				newBomb->init(glm::vec2(x, y), texProgram);
				items.push_back(newBomb);
				break;
			}
			case 2:
			{
				map->setMapTile(glm::vec2(i, j));
				Weight* newWeight = new Weight();
				newWeight->init(glm::vec2(x, y + SCREEN_Y), texProgram);
				newWeight->setTileMap(map);
				weights.push_back(newWeight);
				break;
			}
			case 9:
			{
				Clock* newClock = new Clock();
				newClock->init(glm::vec2(x, y), texProgram);
				items.push_back(newClock);
				break;
			}
			case 11:
			{
				Patroller* patroller = new Patroller();
				patroller->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
				// Patrick is 18x38, adjust spawn to be above the tile
				patroller->setPosition(glm::vec2(i * map->getTileSize(), (j * map->getTileSize()) - (38 - map->getTileSize())));
				patroller->setTileMap(map);
				enemies.push_back(patroller);
				break;
			}
			case 12:
			{
				Shooter* shooter = new Shooter();
				shooter->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
				// Squidward is 18x40, adjust spawn to be above the tile
				shooter->setPosition(glm::vec2(i * map->getTileSize(), (j * map->getTileSize()) - (40 - map->getTileSize())));
				shooter->setTileMap(map);
				enemies.push_back(shooter);
				break;
			}
			case 13:
			{
				Follower* follower = new Follower();
				follower->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
				// Plankton is 16x20, adjust spawn to be above the tile
				follower->setPosition(glm::vec2(i * map->getTileSize(), (j * map->getTileSize()) - (20 - map->getTileSize())));
				follower->setTileMap(map);
				enemies.push_back(follower);
			}
			}
		}
	}

	// Get the pair of positions from the map
	std::map<char, std::vector<glm::vec2>> const& stairPos = map->getPositionsOfStairs();

	for (auto iter : stairPos) {
		// 1. Convert tile coordinates to pixel coordinates
		float x1 = SCREEN_X + (iter.second[0].x * map->getTileSize());
		float y1 = SCREEN_Y + (iter.second[0].y * map->getTileSize()) - 15;

		float x2 = SCREEN_X + (iter.second[1].x * map->getTileSize());
		float y2 = SCREEN_Y + (iter.second[1].y * map->getTileSize()) - 15;

		// 2. Create the Entrance Stair
		Stairs* stairA = new Stairs();
		stairA->init(glm::vec2(x1, y1), texProgram);
		stairA->setDestination(glm::vec2(x2, y2));
		stairs.push_back(stairA);

		// 3. Create the Exit Stair (so you can go back)
		Stairs* stairB = new Stairs();
		stairB->init(glm::vec2(x2, y2), texProgram);
		stairB->setDestination(glm::vec2(x1, y1));
		stairs.push_back(stairB);
	}

	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;

	// Play level music and load hurt sound
	SFX::instance().playMusic("assets/audio/playing.mp3", true, 40.f);
	SFX::instance().loadSound("hurt", "assets/audio/hurt.mp3");
	SFX::instance().loadSound("you_died", "assets/audio/you_died.mp3");

	// Fade overlay: necesitas assets/images/white.png (1×1 píxel blanco)
	fadeTexture.loadFromFile("assets/images/white.png", TEXTURE_PIXEL_FORMAT_RGBA);
	fadeSprite = Sprite::createSprite(glm::ivec2(640, 480), glm::vec2(1.f, 1.f),
		&fadeTexture, &texProgram);
	fadeSprite->setNumberAnimations(1);
	fadeSprite->setAnimationSpeed(0, 1);
	fadeSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	fadeSprite->changeAnimation(0);
	fadeSprite->setPosition(glm::vec2(0.f, 0.f));

	// YOU DIED overlay: necesitas assets/images/you_died.png
	youDiedTexture.loadFromFile("assets/images/you_died.png", TEXTURE_PIXEL_FORMAT_RGBA);
	youDiedSprite = Sprite::createSprite(glm::ivec2(650, 86), glm::vec2(1.f, 1.f),
		&youDiedTexture, &texProgram);
	youDiedSprite->setNumberAnimations(1);
	youDiedSprite->setAnimationSpeed(0, 1);
	youDiedSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	youDiedSprite->changeAnimation(0);
	youDiedSprite->setPosition(glm::vec2(0.f, 197.f));

	backgroundTexture.loadFromFile(map->getBackgroundPath(), TEXTURE_PIXEL_FORMAT_RGBA);

	// Create a sprite the size of the screen
	backgroundSprite = Sprite::createSprite(glm::ivec2(640, 480), glm::vec2(1.f, 1.f), &backgroundTexture, &texProgram);
	backgroundSprite->setNumberAnimations(1);
	backgroundSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	backgroundSprite->changeAnimation(0);
}

void LevelScene::update(int deltaTime)
{
	if (firstCall)
	{
		firstCall = false;
		deltaTime = 0;
	}
	currentTime += deltaTime;

	for (auto it = activeEffects.begin(); it != activeEffects.end(); ) {
		(*it)->update(deltaTime);

		// Cast to Explosion to check if finished
		Explosion* exp = dynamic_cast<Explosion*>(*it);
		if (exp && exp->isFinished()) {
			delete* it;
			it = activeEffects.erase(it);
		}
		else {
			++it;
		}
	}

	// ── YOU DIED FADE: fade out después de mostrar "you died" ──────────
	if (youDiedFading)
	{
		fadeTimer += deltaTime;
		fadeAlpha = glm::clamp(fadeTimer / YOU_DIED_FADE_DURATION, 0.f, 1.f);
		if (fadeTimer >= YOU_DIED_FADE_DURATION)
			Game::instance().changeState(MAIN_MENU);
		return;
	}

	// ── YOU DIED: muestra filtro gris + imagen, luego fade ─────────────
	if (youDied)
	{
		youDiedTimer += deltaTime;
		if (youDiedTimer >= YOU_DIED_DURATION)
		{
			youDied = false;
			youDiedFading = true;
			fadeTimer = 0.f;
			fadeAlpha = 0.f;
			Game::instance().resetLives();
		}
		return;
	}

	// ── Fade out tras perder vida (no última) ──────────────────────────
	if (fadingOut)
	{
		fadeTimer += deltaTime;
		fadeAlpha = glm::clamp(fadeTimer / FADE_DURATION, 0.f, 1.f);

		if (fadeTimer >= FADE_DURATION)
		{
			// Reiniciar escena con una vida menos guardada
			init();
		}
		return;
	}

	// ── Hurt pause: el juego se congela en el último frame antes del fade ──
	if (hurtPaused)
	{
		hurtPauseTimer += deltaTime;
		if (hurtPauseTimer >= HURT_PAUSE_DURATION)
		{
			hurtPaused = false;
			if (player->getLives() <= 0)
			{
				// Sin vidas → mostrar "you died" con filtro gris
				youDied = true;
				youDiedTimer = 0.f;
			}
			else
			{
				// Aún quedan vidas → fade out y reiniciar
				fadingOut = true;
				fadeTimer = 0.f;
				fadeAlpha = 0.f;
			}
		}
		return;
	}

	// ── Hurt freeze: solo knockback del personaje ──────────────────────
	if (playerHurting)
	{
		player->updateHurtLogic(deltaTime);
		if (!player->isHurting())
		{
			// La animación de hurt terminó, ahora pausamos en el último frame
			playerHurting = false;
			hurtPaused = true;
			hurtPauseTimer = 0.f;
		}
		return;
	}

	// ── Door-entry freeze ──────────────────────────────────────────────
	if (enteringDoor)
	{
		player->getSprite()->update(deltaTime);
		enterAnimTimer += deltaTime;

		if (enterAnimTimer >= ENTER_ANIM_DURATION && pendingDoor != nullptr)
		{
			if (pendingDoor->getKind() == DoorType::OPENDOOR)
			{
				// Animation finished → do the actual scene transition
				LevelScene* targetScene = pendingDoor->getRoom();

				pendingDoor->setOpened(true);
				targetScene->setPlayer(this->player);
				targetScene->doors[doorNum]->setOpened(true);
				this->player->setTileMap(targetScene->getMap());

				glm::vec2 doorPos = targetScene->findDoorPosition(doorNum);
				player->setPosition(doorPos - glm::vec2(SCREEN_X, SCREEN_Y));

				targetScene->setCooldown();
				Game::instance().setScene(targetScene);

				// Reset state for when we return to this scene later
				enteringDoor = false;
				enterAnimTimer = 0.f;
				pendingDoor = nullptr;
			}
			else
			{
				KeyDoor* kd = static_cast<KeyDoor*>(pendingDoor);
				Game::instance().getNextLevel(this);
			}
		}
		return; // everything else is frozen
	}

	if (stairCooldown > 0)
		stairCooldown -= deltaTime;

	float playerWorldX = player->getPosition().x + SCREEN_X;
	float playerWorldY = player->getPosition().y + SCREEN_Y;
	float pL = playerWorldX + 4;
	float pR = playerWorldX + 20;
	float pT = playerWorldY + 4;
	float pB = playerWorldY + 28;

	// Doors
	for (Door* d : doors)
	{
		float pCenterX = player->getPosition().x + SCREEN_X + 12.f;
		float pCenterY = player->getPosition().y + SCREEN_Y + 16.f;
		float dCenterX = d->getPosition().x + 16.f;
		float dCenterY = d->getPosition().y + 16.f;

		float distX = abs(pCenterX - dCenterX);
		float distY = abs(pCenterY - dCenterY);

		// Solo activa si el jugador está muy cerca en X e Y
		// Ajusta estos valores si sigue siendo demasiado amplio o estrecho
		if (distX < 12.f && distY < 14.f)
		{
			if (Game::instance().getKey(GLFW_KEY_UP) && stairCooldown <= 0)
			{
				if ((d->getKind() == DoorType::OPENDOOR && d->getRoom() != nullptr) || (d->getKind() == DoorType::KEYDOOR && totalNumKeys() == 0))
				{
					pendingDoor = d;
					enteringDoor = true;
					enterAnimTimer = 0.f;
					player->startDoorEnterAnimation();
					return;
				}
			}
		}
	}

	// Stairs
	for (Stairs* s : stairs)
	{
		float sL = s->getPosition().x, sR = sL + 32;
		float sT = s->getPosition().y, sB = sT + 32;
		if (pL < sR && pR > sL && pT < sB && pB > sT)
		{
			if (Game::instance().getKey(GLFW_KEY_UP) && stairCooldown <= 0)
			{
				glm::vec2 dest = s->getDestination();
				player->setPosition(dest - glm::vec2(SCREEN_X, SCREEN_Y + 1));
				stairCooldown = STAIR_DELAY;
				return;
			}
		}
	}

	// 4. Player Update (with input lock if cooldown is active)
	player->update(deltaTime, (stairCooldown > 0));
	player->playerEvent(this, deltaTime);

	// 5. Item updates
	for (auto it = items.begin(); it != items.end(); ) {
		float iL = (*it)->getPosition().x;
		float iR = iL + 32;
		float iT = (*it)->getPosition().y;
		float iB = iT + 32;
		if (pL < iR && pR > iL && pT < iB && pB > iT) {
			if (pL < iR && pR > iL && pT < iB && pB > iT) {
				if (dynamic_cast<BubbleGun*>(*it) != nullptr)
				{
					player->addBullet();
				}
				else
				{
					Bomb* bomb = dynamic_cast<Bomb*>(*it);
					if (bomb != nullptr)
					{
						player->addBomb();
					}
					else
					{
						if (dynamic_cast<Clock*>(*it) != nullptr)
						{
							stoppedTime = true;
							timeStopped = 5000;
						}
					}
				}
				delete* it;
				it = items.erase(it);
			}
		}
		else {
			(*it)->update(deltaTime);
			++it;
		}
	}

	// Keys in the scene

	for (auto it = keys.begin(); it != keys.end(); ) {
		// Key hitbox matches visual size (20x13)
		float iL = (*it)->getPosition().x;
		float iR = iL + 20;
		float iT = (*it)->getPosition().y;
		float iB = iT + 13;

		if (pL < iR && pR > iL && pT < iB && pB > iT) {
			delete* it;
			it = keys.erase(it);
			player->addKey(1);
		}
		else
		{
			(*it)->update(deltaTime);
			++it;
		}
	}

	// Items placed/shot.

	for (auto bullet : bulletsFired)
	{
		bullet->update(deltaTime);
	}
	for (auto bomb : bombsPlaced)
	{
		bomb->update(deltaTime);
	}

	// Weights moved
	for (auto w = weights.begin(); w != weights.end();) {
		float pL = player->getPosition().x + SCREEN_X;
		float pR = pL + 24; // Player width
		float wL = (*w)->getPosition().x;
		float wR = wL + 16; // Weight width

		// Vertical overlap check
		float pT = player->getPosition().y + SCREEN_Y;
		float pB = pT + 32;
		float wT = (*w)->getPosition().y;
		float wB = wT + 16;

		if (pB > wT && pT < wB) { // If at the same height
			// If Player hits left side of weight while moving right
			if ((pR - 2) > wL && pL < wL && Game::instance().getKey(GLFW_KEY_RIGHT)) {
				(*w)->push(2.0f); // Match player speed
			}
			// If Player hits right side of weight while moving left
			else if ((pL + 8) < wR && pR > wR && Game::instance().getKey(GLFW_KEY_LEFT)) {
				(*w)->push(-2.0f);
			}
		}
		(*w)->update(deltaTime);
		if ((*w)->getFell())
		{
			Explosion* exp = new Explosion();
			exp->init(glm::vec2((*w)->getPosition().x, (*w)->getPosition().y - 32), &explosionTexture, texProgram);
			activeEffects.push_back(exp);
			delete* w;
			w = weights.erase(w);
			SFX::instance().playSound("explosion", 70.f);
		}
		else
		{
			w++;
		}
	}

	// --- Collision: Projectiles & Weights vs Enemies ---
	for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
		bool enemyKilled = false;

		// 1. Calculate Enemy Hitbox (World Space)
		// Using the same +SCREEN_X/Y logic your weights/player use
		float eL = (*eIt)->getPosition().x + SCREEN_X + 4; // 4px padding for tighter hits
		float eR = eL + (*eIt)->getWidth();
		float eT = (*eIt)->getPosition().y + SCREEN_Y + 4;
		float eB = eT + (*eIt)->getHeight();

		// A. Check Bullets
		for (auto bIt = bulletsFired.begin(); bIt != bulletsFired.end(); ) {
			glm::vec2 bPos = (*bIt)->getPosition();
			// Assuming Bullet is 8x8 pixels
			if (bPos.x < eR && bPos.x + 8 > eL && bPos.y < eB && bPos.y + 8 > eT) {
				enemyKilled = true;
				Explosion* exp = new Explosion();
				exp->init(glm::vec2((*eIt)->getPosition().x, (*eIt)->getPosition().y), &explosionTexture, texProgram);
				activeEffects.push_back(exp);
				delete* bIt;
				bIt = bulletsFired.erase(bIt);
				SFX::instance().playSound("explosion", 70.f);
				break; // Stop checking other bullets for this enemy
			}
			else ++bIt;
		}

		// B. Check Bombs (if enemy is still alive)
		if (!enemyKilled) {
			for (auto bmIt = bombsPlaced.begin(); bmIt != bombsPlaced.end(); ) {
				glm::vec2 bmPos = (*bmIt)->getPosition();
				// Assuming Bomb is 16x16 pixels
				if (bmPos.x < eR && bmPos.x + 16 > eL && bmPos.y < eB && bmPos.y + 16 > eT) {
					enemyKilled = true;
					Explosion* exp = new Explosion();
					exp->init(glm::vec2((*eIt)->getPosition().x, (*eIt)->getPosition().y), &explosionTexture, texProgram);
					activeEffects.push_back(exp);
					delete* bmIt;
					bmIt = bombsPlaced.erase(bmIt);
					SFX::instance().playSound("explosion", 70.f);
					break;
				}
				else ++bmIt;
			}
		}

		// C. Check Weights (Always kills on contact)
		if (!enemyKilled) {
			for (auto w = weights.begin(); w != weights.end();) {
				float wL = (*w)->getPosition().x; // Weight is already in World Space
				float wR = wL + 16;
				float wT = (*w)->getPosition().y;
				float wB = wT + 16;

				if (wL < eR && wR > eL && wT < eB && wB > eT) {
					enemyKilled = true;
					Explosion* exp = new Explosion();
					exp->init(glm::vec2((*eIt)->getPosition().x, (*eIt)->getPosition().y), &explosionTexture, texProgram);
					activeEffects.push_back(exp);
					delete (*w);
					w = weights.erase(w);
					SFX::instance().playSound("explosion", 70.f);
					break;
				}
				else ++w;
			}
		}

		// Finalize: If any item hit, delete the enemy
		if (enemyKilled) {
			delete* eIt;
			eIt = enemies.erase(eIt);
		}
		else {
			++eIt;
		}
	}

	timeStopped -= deltaTime;
	// Enemy updates
	for (Enemy* e : enemies) {
		if (stoppedTime && timeStopped < 0)
		{
			stoppedTime = false;
		}
		if (!stoppedTime)
		{
			Follower* follower = dynamic_cast<Follower*>(e);

			if (follower != nullptr)
			{
				follower->update(deltaTime, player->getPosition(), weights);
			}
			else
			{
				// 1. Try to cast the generic Enemy to a Shooter
				Shooter* shooter = dynamic_cast<Shooter*>(e);

				// 2. If the cast succeeded, shooter will not be NULL
				if (shooter != nullptr) {
					// Now you can access Shooter-specific functions
					shooter->Shoot(deltaTime, texProgram);
					shooter->update(deltaTime, weights);
					
					// Remove bullets that hit solid blocks
					std::vector<Bullet*>& bullets = shooter->getBullets();
					auto it = bullets.begin();
					while (it != bullets.end()) {
						Bullet* b = *it;
						glm::vec2 bPos = b->getPosition();
						glm::ivec2 bSize = b->getSize();
						
						// Check if bullet hits a solid tile (tile ID 1)
						int tileAtBullet = map->getTileIdAt(glm::ivec2((int)bPos.x + bSize.x/2, (int)bPos.y + bSize.y/2));
						if (tileAtBullet == 1) {
							delete b;
							it = bullets.erase(it);
						} else {
							++it;
						}
					}
				}
				else
				{
					Patroller* patroller = static_cast<Patroller*>(e);
					patroller->update(deltaTime, weights);
				}
			}
		}

		// ── Colisión enemigo → jugador ─────────────────────────────────
		if (!player->isInvincible() && !player->isGodMode())
		{
			glm::vec2 ePos = e->getPosition();
			float eWorldX = ePos.x + SCREEN_X;
			float eWorldY = ePos.y + SCREEN_Y;
			
			// Different hitboxes based on enemy type
			float eL, eR, eT, eB;
			Shooter* shooter = dynamic_cast<Shooter*>(e);
			Patroller* patroller = dynamic_cast<Patroller*>(e);
			Follower* follower = dynamic_cast<Follower*>(e);
			
			if (shooter) {
				// Squidward: 18x40
				eL = eWorldX; eR = eWorldX + 18;
				eT = eWorldY; eB = eWorldY + 40;
			} else if (patroller) {
				// Patrick: 18x38
				eL = eWorldX; eR = eWorldX + 18;
				eT = eWorldY; eB = eWorldY + 38;
			} else if (follower) {
				// Plankton: 16x20
				eL = eWorldX; eR = eWorldX + 16;
				eT = eWorldY; eB = eWorldY + 20;
			} else {
				// Default fallback
				eL = eWorldX; eR = eWorldX + 32;
				eT = eWorldY; eB = eWorldY + 32;
			}

			if (pL < eR && pR > eL && pT < eB && pB > eT)
			{
				bool enemyToRight = (eWorldX + 16.f) > (playerWorldX + 12.f);
				player->receiveDamage(1);
				player->startHurtAnimation(enemyToRight);
				SFX::instance().playSound("hurt", 80.f);
				playerHurting = true;
				return;
			}
		}
	}

	// ── Colisión balas → jugador ───────────────────────────────────────
	if (!player->isInvincible() && !player->isGodMode())
	{
		for (Enemy* e : enemies)
		{
			Shooter* shooter = dynamic_cast<Shooter*>(e);
			if (!shooter) continue;

			for (Bullet* b : shooter->getBullets())
			{
				glm::vec2 bPos = b->getPosition();
				glm::ivec2 bSize = b->getSize();
				float bL = bPos.x, bR = bPos.x + bSize.x;
				float bT = bPos.y, bB = bPos.y + bSize.y;

				if (pL < bR && pR > bL && pT < bB && pB > bT)
				{
					bool bulletFromLeft = b->isMovingRight();
					player->receiveDamage(1);
					player->startHurtAnimation(!bulletFromLeft);
					SFX::instance().playSound("hurt", 80.f);
					playerHurting = true;
					return;
				}
			}
		}
	}
}

void LevelScene::setPlayer(Player* newPlayer)
{
	player = newPlayer;
}

glm::vec2 LevelScene::findDoorPosition(int numDoor)
{
	// Check if the vector is empty or the pointer is null
	if (!doors.empty() && numDoor < doors.size())
	{
		return doors[numDoor]->getPosition();
	}

	// Fallback if that specific door isn't in this room
	return glm::vec2(100, 100);
}

void LevelScene::setCooldown()
{
	stairCooldown = STAIR_DELAY;
}

void LevelScene::render()
{
	glm::mat4 modelview;
	float zoomWidth = 320.0f;
	float zoomHeight = 240.0f;
	projection = glm::ortho(0.f, zoomWidth, zoomHeight, 0.f);

	// 1. Calculate Camera Logic
	float playerWorldX = player->getPosition().x + SCREEN_X + 12.0f;
	float playerWorldY = player->getPosition().y + SCREEN_Y + 16.0f;
	float camX = playerWorldX - (zoomWidth / 2.0f);
	float camY = playerWorldY - (zoomHeight / 2.0f);
	float mapWidth = (map->getMapSize().x * map->getTileSize()) + (SCREEN_X * 2);
	float mapHeight = (map->getMapSize().y * map->getTileSize()) + (SCREEN_Y * 2);

	if (mapWidth < zoomWidth) camX = (mapWidth - zoomWidth) / 2.0f;
	else camX = glm::clamp(camX, 0.0f, mapWidth - zoomWidth);
	if (mapHeight < zoomHeight) camY = (mapHeight - zoomHeight) / 2.0f;
	else camY = glm::clamp(camY, 0.0f, mapHeight - zoomHeight);

	modelview = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, 0.0f));

	texProgram.use();

	// --- 1. BACKGROUND ---
	if (backgroundSprite != nullptr) {
		texProgram.setUniformMatrix4f("projection", projection);
		float parallaxFactor = 0.3f;
		glm::mat4 bgModelview = glm::translate(glm::mat4(1.0f), glm::vec3(-camX * parallaxFactor, -camY * parallaxFactor, 0.0f));
		texProgram.setUniformMatrix4f("modelview", bgModelview);
		backgroundSprite->render(bgModelview);
	}

	// --- 2. WORLD OBJECTS (Tiles, Enemies, Player) ---
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	map->render();

	for (unsigned int i = 0; i < keys.size(); i++) keys[i]->render(modelview);
	for (unsigned int i = 0; i < items.size(); i++) items[i]->render(modelview);
	for (unsigned int i = 0; i < stairs.size(); i++) stairs[i]->render(modelview);
	for (unsigned int i = 0; i < doors.size(); i++) doors[i]->render(modelview);
	for (unsigned int i = 0; i < enemies.size(); i++) enemies[i]->render(modelview);
	for (auto bullet : bulletsFired) bullet->render(modelview);
	for (auto bomb : bombsPlaced) bomb->render(modelview);
	for (auto weight : weights) weight->render(modelview);

	player->render(modelview);

	for (auto effect : activeEffects) {
		effect->render(modelview);
	}

	// --- 3. HUD (SCREEN SPACE - DRAWN LAST TO BE ON TOP) ---
	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 hudProj = glm::ortho(0.f, 640.f, 480.f, 0.f);

	texProgram.setUniformMatrix4f("projection", hudProj);
	texProgram.setUniformMatrix4f("modelview", identity);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f); // FIX: Ensure no offset from previous renders
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto drawHUDItem = [&](Sprite* icon, int count, float x, float y) {
		if (icon == nullptr || numberSprite == nullptr) return;
		icon->setPosition(glm::vec2(x, y));
		icon->render(identity);

		int val = glm::clamp(count, 0, 9);
		numberSprite->changeAnimation(val);
		numberSprite->setPosition(glm::vec2(x + 18, y + 2));
		numberSprite->render(identity);
		};

	drawHUDItem(keyIcon, player->getKeyCount(), 580.f, 30.f);
	drawHUDItem(bubbleIcon, player->getBulletCount(), 580.f, 50.f);
	drawHUDItem(bombIcon, player->getBombCount(), 580.f, 70.f);

	// --- 4. OVERLAYS (Death, Fades) ---
	if (fadingOut || youDied || youDiedFading)
	{
		// ... (Your existing overlay code is fine here as it also uses identity/hudProj)
		// Ensure screenProj matches hudProj (640x480)
		glm::mat4 screenProj = glm::ortho(0.f, 640.f, 480.f, 0.f);
		texProgram.setUniformMatrix4f("projection", screenProj);
		texProgram.setUniformMatrix4f("modelview", identity);

		if (fadingOut) {
			texProgram.setUniform4f("color", 0.f, 0.f, 0.f, fadeAlpha);
			fadeSprite->render(identity);
		}
		if (youDied || youDiedFading) {
			texProgram.setUniform4f("color", 0.3f, 0.3f, 0.3f, 0.4f);
			fadeSprite->render(identity);
			texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
			youDiedSprite->render(identity);
			if (youDiedFading) {
				texProgram.setUniform4f("color", 0.f, 0.f, 0.f, fadeAlpha);
				fadeSprite->render(identity);
			}
		}
	}
	glDisable(GL_BLEND);
}

void LevelScene::setDoorNum(int numDoor)
{
	doorNum = numDoor;
}

int LevelScene::totalNumKeys()
{
	int numKeys = keys.size();
	for (auto d : doors)
	{
		if (d->getKind() == DoorType::OPENDOOR)
		{
			numKeys += d->getRoom()->numKeys();
		}
	}
	return numKeys;
}

void LevelScene::collectKeys()
{
	for (auto d : doors)
	{
		if (d->getKind() == DoorType::OPENDOOR)
		{
			if (d->getRoom()->numKeys() != 0)
			{
				player->addKey(1);
			}
			d->getRoom()->clearKeys();
		}
	}
	player->addKey(keys.size());
	keys.clear();
}

void LevelScene::clearLevel() {
	// 1. DELETE VECTORS OF POINTERS
	// We check if (!= nullptr) just in case, then delete and nullify
	for (Enemy* e : enemies) { if (e) delete e; }
	enemies.clear();

	for (Weight* w : weights) { if (w) delete w; }
	weights.clear();

	for (Key* k : keys) { if (k) delete k; }
	keys.clear();

	for (Entity* i : items) { if (i) delete i; }
	items.clear();

	for (Bullet* b : bulletsFired) { if (b) delete b; }
	bulletsFired.clear();

	for (Bomb* b : bombsPlaced) { if (b) delete b; }
	bombsPlaced.clear();

	for (Stairs* s : stairs) { if (s) delete s; }
	stairs.clear();

	for (Door* d : doors) { if (d) delete d; }
	doors.clear();

	for (Entity* a : activeEffects) { if (a) delete a; }
	activeEffects.clear();

	// 2. HANDLE SINGLETON POINTERS (Except Player)
	// We don't delete 'player' here because it's managed by Game/init logic
	if (map != nullptr) {
		delete map;
		map = nullptr;
	}

	// 3. RESET TIMERS AND FLOATS
	stairCooldown = 0.0f;
	enterAnimTimer = 0.0f;
	hurtPauseTimer = 0.0f;
	fadeAlpha = 0.0f;
	fadeTimer = 0.0f;
	youDiedTimer = 0.0f;
	timeStopped = 0;

	// 4. RESET BOOLEAN STATES
	enteringDoor = false;
	playerHurting = false;
	hurtPaused = false;
	fadingOut = false;
	youDied = false;
	youDiedFading = false;
	stoppedTime = false;

	// 5. NULLIFY PENDING POINTERS
	pendingDoor = nullptr;

	if (backgroundSprite != nullptr) {
		delete backgroundSprite;
		backgroundSprite = nullptr;
	}

	if (keyIcon != nullptr) {
		delete keyIcon;
		keyIcon = nullptr; // Crucial: prevents double-deletion
	}
	if (bubbleIcon != nullptr) {
		delete bubbleIcon;
		bubbleIcon = nullptr;
	}
	if (bombIcon != nullptr) {
		delete bombIcon;
		bombIcon = nullptr;
	}
	if (numberSprite != nullptr) {
		delete numberSprite;
		numberSprite = nullptr;
	}

	// Set to nullptr to be safe
	keyIcon = bubbleIcon = bombIcon = numberSprite = nullptr;
}