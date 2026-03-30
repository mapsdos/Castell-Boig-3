#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelScene.h"
#include "Key.h"
#include "Door.h"
#include "Game.h"
#include "Patroller.h"
#include "Shooter.h"
#include "Follower.h"

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25


LevelScene::LevelScene()
{
	map = NULL;
	player = NULL;
	doorNum = 0;
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
    // Reset all gameplay and animation flags
    youDied = false;
    fadingOut = false;
    youDiedTimer = 0.f;
    fadeTimer = 0.f;
    fadeAlpha = 0.f;
    playerHurting = false;
    enteringDoor = false;
    enterAnimTimer = 0.f;
    pendingDoor = nullptr;
    stairCooldown = 0.f;
    // Reinicializa la escena normalmente
    init("assets/levels/level01.txt");
}

void LevelScene::init(string path)
{
    // Limpiar entidades previas para evitar duplicados
    for (auto* e : enemies) delete e;
    enemies.clear();
    for (auto* it : items) delete it;
    items.clear();
    for (auto* it : initialItems) delete it;
    initialItems.clear();
    for (auto* s : stairs) delete s;
    stairs.clear();
    for (auto* d : doors) delete d;
    doors.clear();
    // Ahora inicializa normalmente
	initShaders();
	map = TileMap::createTileMap(path, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	vector<string> const & roomFiles = map->getRoomFiles();
	unsigned roomSize = roomFiles.size();
	doorNum = 0;

	glm::ivec2 size = map->getMapSize();
	for (int j = 0; j < size.y; j++) {
		for (int i = 0; i < size.x; i++) {
			// Check the map data for the key ID (2)
			int tileId = map->getTileIdAt(glm::ivec2(i * map->getTileSize(), j * map->getTileSize()));
			if (tileId == 5) {
				Key* newKey = new Key();

				// Calculate pixel position: (Column * TileSize, Row * TileSize)
				// Add SCREEN_X/Y if your map has an offset
				// In LevelScene.cpp init()
				float x = SCREEN_X + (i * map->getTileSize());
				// Subtract 16 (or the difference between sprite height and tile height) 
				// to pull the key "up" out of the floor
				float y = SCREEN_Y + (j * map->getTileSize()) - (32 - map->getTileSize());

				newKey->init(glm::vec2(x, y), texProgram);
				items.push_back(newKey);
				initialItems.push_back(newKey->clone(texProgram));
			}
			// Inside the tile loop where tileId == 4
			else if (tileId == 4) {
				Door* newDoor = new Door();
				float x = SCREEN_X + (i * map->getTileSize());
				float y = SCREEN_Y + (j * map->getTileSize()) - (32 - map->getTileSize());
				newDoor->init(glm::vec2(x, y), texProgram);

				if ( roomSize > 0) {
					// Create a WHOLE NEW SCENE for the room
					LevelScene* roomScene = new LevelScene();
					roomScene->init(roomFiles.at(roomSize - 1)); // Custom init that takes a filename
					roomScene->doors[0]->setRoom(this);
					newDoor->setRoom(roomScene);
					roomScene->enemies.clear();
					roomScene->setDoorNum(roomFiles.size() - roomSize);

					// IMPORTANT: The door inside the roomScene needs to point BACK to 'this'
					// You'll need a logic to link them back to the current LevelScene

					--roomSize;
				}
				doors.push_back(newDoor);
			}
		}
	}

	// Get the pair of positions from the map
	std::map<char,std::vector<glm::vec2>> const & stairPos = map->getPositionsOfStairs();

	for(auto iter : stairPos) {
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

	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;

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

	LoadEnemies();
}

void LevelScene::LoadEnemies()
{
	Patroller* patroller = new Patroller();
	patroller->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
    patroller->setPosition(glm::vec2(10 * map->getTileSize(), 25 * map->getTileSize()));
    patroller->setTileMap(map);
	enemies.push_back(patroller);

	Shooter* shooter= new Shooter();
	shooter->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	shooter->setPosition(glm::vec2(10 * map->getTileSize(), 25 * map->getTileSize()));
	shooter->setTileMap(map);
	enemies.push_back(shooter);

	Follower* follower = new Follower();
	follower->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	follower->setPosition(glm::vec2(10 * map->getTileSize(), 25 * map->getTileSize()));
	follower->setTileMap(map);
	enemies.push_back(follower);
}

void LevelScene::update(int deltaTime)
{
    currentTime += deltaTime;

    // ── YOU DIED: espera y vuelve al menú ─────────────────────────────
    if (youDied)
    {
        youDiedTimer += deltaTime;
        if (youDiedTimer >= YOU_DIED_DURATION)
            Game::instance().changeState(MAIN_MENU);
        return;
    }

    // ── Fade out tras muerte ───────────────────────────────────────────
    if (fadingOut)
    {
        fadeTimer += deltaTime;
        fadeAlpha = glm::clamp(fadeTimer / FADE_DURATION, 0.f, 1.f);

        if (fadeTimer >= FADE_DURATION)
        {
            if (player->getLives() <= 0)
            {
                // Sin vidas → YOU DIED
                fadingOut = false;
                youDied = true;
                youDiedTimer = 0.f;
            }
            else
            {
                // Reiniciar escena con una vida menos guardada
                int livesLeft = player->getLives();
                delete player;
                player = nullptr;

                // Re-inicializar la escena
                // (enemies, items, etc. se recrean; player también)
                // Borramos lo que sea necesario primero
                for (auto* e : enemies) delete e;
                enemies.clear();
                for (auto* it : items)  delete it;
                items.clear();
                // Restaurar items al estado inicial
                for (auto* orig : initialItems) items.push_back(orig->clone(texProgram));
                // stairs y doors ya existen; los dejamos (o podrías limpiar también)

                // Recrear player
                player = new Player();
                player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
                player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(),
                    INIT_PLAYER_Y_TILES * map->getTileSize()));
                player->setTileMap(map);
                player->setLives(livesLeft);   // ← necesitas este setter público (ya lo tienes)

                LoadEnemies();

                fadingOut = false;
                playerHurting = false;
                fadeAlpha = 0.f;
                fadeTimer = 0.f;
            }
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
            LevelScene* targetScene = pendingDoor->getRoom();
            pendingDoor->setOpened(true);
            targetScene->setPlayer(this->player);
            targetScene->doors[doorNum]->setOpened(true);
            this->player->setTileMap(targetScene->getMap());

            glm::vec2 doorPos = targetScene->findDoorPosition(doorNum);
            player->setPosition(doorPos - glm::vec2(SCREEN_X, SCREEN_Y));

            targetScene->setCooldown();
            Game::instance().setScene(targetScene);

            enteringDoor = false;
            enterAnimTimer = 0.f;
            pendingDoor = nullptr;
        }
        return;
    }

    // ── Hurt freeze: solo knockback del personaje ──────────────────────
    if (playerHurting)
    {
        player->updateHurtLogic(deltaTime);
        if (!player->isHurting())
        {
            playerHurting = false;
            fadingOut = true;
            fadeTimer = 0.f;
            fadeAlpha = 0.f;
        }
        return;
    }

    // ── Lógica normal ─────────────────────────────────────────────────
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

        if (abs(pCenterX - dCenterX) < 12.f && abs(pCenterY - dCenterY) < 14.f)
        {
            if (Game::instance().getKey(GLFW_KEY_UP) && stairCooldown <= 0)
            {
                if (d->getRoom() != nullptr)
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

    player->update(deltaTime, (stairCooldown > 0));

    // Items
    for (auto it = items.begin(); it != items.end(); )
    {
        float iL = (*it)->getPosition().x, iR = iL + 32;
        float iT = (*it)->getPosition().y, iB = iT + 32;
        if (pL < iR && pR > iL && pT < iB && pB > iT) { delete* it; it = items.erase(it); }
        else { (*it)->update(deltaTime); ++it; }
    }

    // Enemies + colisión con jugador
    for (Enemy* e : enemies)
    {
        Follower* follower = dynamic_cast<Follower*>(e);
        if (follower) follower->update(deltaTime, player->getPosition());
        else
        {
            e->update(deltaTime);
            Shooter* shooter = dynamic_cast<Shooter*>(e);
            if (shooter) shooter->Shoot(deltaTime, texProgram);
        }

        // ── Colisión enemigo → jugador ─────────────────────────────────
        if (!player->isInvincible())
        {
            glm::vec2 ePos = e->getPosition();
            float eWorldX = ePos.x + SCREEN_X;
            float eWorldY = ePos.y + SCREEN_Y;
            float eL = eWorldX + 4, eR = eWorldX + 28;
            float eT = eWorldY + 4, eB = eWorldY + 28;

            if (pL < eR && pR > eL && pT < eB && pB > eT)
            {
                bool enemyToRight = (eWorldX + 16.f) > (playerWorldX + 12.f);
                player->receiveDamage(1);
                player->startHurtAnimation(enemyToRight);
                playerHurting = true;
                return;
            }
        }
    }

    // ── Colisión balas → jugador ───────────────────────────────────────
    if (!player->isInvincible())
    {
        for (Enemy* e : enemies)
        {
            Shooter* shooter = dynamic_cast<Shooter*>(e);
            if (!shooter) continue;

            for (Bullet* b : shooter->getBullets())
            {
                glm::vec2 bPos = b->getPosition();
                float bL = bPos.x, bR = bPos.x + 8.f;
                float bT = bPos.y, bB = bPos.y + 8.f;

                if (pL < bR && pR > bL && pT < bB && pB > bT)
                {
                    bool bulletFromLeft = b->isMovingRight(); // bala viene de la izquierda
                    player->receiveDamage(1);
                    player->startHurtAnimation(!bulletFromLeft); // enemigo al lado del que viene
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
	// 1. Set the Zoom (Projection)
	// Instead of the full 640x480, we define a view volume of 320x240
	float zoomWidth = 320.0f;
	float zoomHeight = 240.0f;
	projection = glm::ortho(0.f, zoomWidth, zoomHeight, 0.f);

	// 2. Calculate Camera Position
	// charX and charY are the pixel coordinates of your character
	float camX = player->getPosition().x - (zoomWidth / 2.0f);
	float camY = player->getPosition().y - (zoomHeight / 2.0f);

	// 3. Clamp Camera to Map Edges (Optional but recommended)
	// Map size in pixels = (28 blocks * blockSize) x (36 blocks * blockSize)
	float mapWidth = 640.f;// 28.0f * blockSize;
	float mapHeight = 480.f;//36.0f * blockSize;

	camX = glm::clamp(camX, 0.0f, mapWidth - zoomWidth);
	camY = glm::clamp(camY, 0.0f, mapHeight - zoomHeight);

	// 4. Apply to Modelview
	// We move the "world" in the opposite direction of the camera
	modelview = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, 0.0f));

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	for (unsigned int i = 0; i < items.size(); i++) {
		items[i]->render(modelview);
	}
	for (unsigned int i = 0; i < stairs.size(); i++)
	{
		stairs[i]->render(modelview);
	}
	for (unsigned int i = 0; i < doors.size(); i++)
	{
		doors[i]->render(modelview);
	}
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->render(modelview);
	}
	player->render(modelview);
    // ── Fade / YOU DIED overlays (en espacio de pantalla, sin cámara) ──
    if (fadingOut || youDied)
    {
        glm::mat4 identity = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        texProgram.setUniformMatrix4f("modelview", identity);
        // Proyección ortho simple para pantalla completa
        glm::mat4 screenProj = glm::ortho(0.f, 640.f, 480.f, 0.f);
        texProgram.setUniformMatrix4f("projection", screenProj);

        if (fadingOut)
        {
            texProgram.setUniform4f("color", 0.f, 0.f, 0.f, fadeAlpha);
            fadeSprite->render(identity);
        }
        if (youDied)
        {
            texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
            youDiedSprite->render(identity);
        }

        glDisable(GL_BLEND);
        // Restaurar color
        texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
    }
}

void LevelScene::setDoorNum(int numDoor)
{
	doorNum = numDoor;
}