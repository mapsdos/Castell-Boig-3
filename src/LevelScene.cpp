#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelScene.h"
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
	player->render(modelview);
}