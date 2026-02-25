#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelScene.h"
#include "Key.h"
#include "Game.h"

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25


LevelScene::LevelScene()
{
	map = NULL;
	player = NULL;
}

LevelScene::~LevelScene()
{
	texProgram.free();
	if (map != NULL)
		delete map;
	if (player != NULL)
		delete player;
}


void LevelScene::init()
{
	initShaders();
	map = TileMap::createTileMap("assets/levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	glm::ivec2 size = map->getMapSize();
	for (int j = 0; j < size.y; j++) {
		for (int i = 0; i < size.x; i++) {
			// Check the map data for the key ID (2)
			if (map->getTileIdAt(glm::ivec2(i * map->getTileSize(), j * map->getTileSize())) == 5) {
				Key* newKey = new Key();

				// Calculate pixel position: (Column * TileSize, Row * TileSize)
				// Add SCREEN_X/Y if your map has an offset
				// In LevelScene.cpp init()
				float x = SCREEN_X + (i * map->getTileSize());
				// Subtract 16 (or the difference between sprite height and tile height) 
				// to pull the key "up" out of the floor
				float y = SCREEN_Y + (j * map->getTileSize()) - (32 - map->getTileSize());

				newKey->init(glm::vec2(x, y), texProgram);

				newKey->init(glm::vec2(x, y), texProgram);
				items.push_back(newKey);
			}
		}
	}

	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
}

void LevelScene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);

	// Ensure player bounds account for the SCREEN_X/Y offset 
	// if getPosition() only returns the relative world tile position.
	float playerWorldX = player->getPosition().x + SCREEN_X;
	float playerWorldY = player->getPosition().y + SCREEN_Y;

	float pL = playerWorldX + 4;
	float pR = playerWorldX + 28;
	float pT = playerWorldY + 4;
	float pB = playerWorldY + 28;

	for (auto it = items.begin(); it != items.end(); ) {
		// The Key iL is already (SCREEN_X + i * tileSize)
		float iL = (*it)->getPosition().x;
		float iR = iL + 32;
		float iT = (*it)->getPosition().y;
		float iB = iT + 32;

		if (pL < iR && pR > iL && pT < iB && pB > iT) {
			delete* it;
			it = items.erase(it);
		}
		else {
			(*it)->update(deltaTime);
			++it;
		}
	}
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
	player->render(modelview);
}