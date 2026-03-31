#include "GraphicsConfig.h"
#include "Game.h"
#include "SFX.h"
#include "CreditsScene.h"
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

Game::Game()
	: state(MAIN_MENU),
	currentScene(nullptr),
	menuScene(nullptr),
	instructionsScene(nullptr),
	creditsScene(nullptr),
	level1Scene(nullptr),
	level2Scene(nullptr),
	level3Scene(nullptr),
	level4Scene(nullptr),
	level5Scene(nullptr),
	bPlay(true)
{
	for (int i = 0; i <= GLFW_KEY_LAST; ++i)
		keys[i] = false;
}


void Game::init()
{
	SFX::instance().init();
	SFX::instance().playMusic("assets/audio/main_menu.mp3", true, 50.f);
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	state = MAIN_MENU;

	menuScene = new MenuScene();
	menuScene->init();

	//creditsScene = new CreditsScene();
	//creditsScene->init();

	currentScene = menuScene;

	playerLives = 3;
	player = new Player();

	level1Scene = new LevelScene("assets/levels/level01.txt",player);
	level2Scene = new LevelScene("assets/levels/level02.txt",player);
	level3Scene = new LevelScene("assets/levels/level03.txt",player);
	level4Scene = new LevelScene();
	level5Scene = new LevelScene();
}

bool Game::update(int deltaTime)
{
	currentScene->update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentScene->render();
}

void Game::keyPressed(int key)
{
	if(key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;
	if (state == MAIN_MENU) {
		if (key == GLFW_KEY_I) changeState(INSTRUCTIONS);
		if (key == GLFW_KEY_C) changeState(CREDITS);
	}
	else if (key == GLFW_KEY_M) { // Press M to return to menu
		changeState(MAIN_MENU);
	}
	if (key == GLFW_KEY_1) { setStateToPlaying(); currentScene = level1Scene; currentScene->init(); }
	if (key == GLFW_KEY_2) { setStateToPlaying(); getNextLevel(level1Scene); }
	if (key == GLFW_KEY_3) {
		setStateToPlaying(); getNextLevel(level2Scene);
	}
	if (key == GLFW_KEY_K)
	{
		LevelScene* currScene = dynamic_cast<LevelScene*>(currentScene);
		if (currScene != nullptr)
		{
			currScene->collectKeys();
		}
	}

	if (key == GLFW_KEY_ESCAPE) bPlay = false;
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
	mousePos.x = x;
	mousePos.y = y;
}

void Game::mousePress(int button)
{
	mouseButtons[button] = true;
}

void Game::mouseRelease(int button)
{
	mouseButtons[button] = false;
}

// Add these getters so scenes can check the mouse status
glm::ivec2 Game::getMousePos() const { return mousePos; }
bool Game::isMouseButtonPressed(int button) const { return mouseButtons[button]; }

bool Game::getKey(int key) const
{
	return keys[key];
}

void Game::setScene(Scene* newScene) {
	if (newScene != NULL) {
		currentScene = newScene;
	}
}


void Game::changeState(GameState newState)
{
	state = newState;
	switch (state)
	{
	case MAIN_MENU:
		currentScene = menuScene;
		break;
	case PLAYING:
		currentScene = level1Scene;
		break;
	case CREDITS:
		currentScene = creditsScene;
		break;
	}
	currentScene->init();
}

void Game::getNextLevel(LevelScene* lvScn)
{
	// Compare the active currentScene pointer to your level instances
	if (lvScn == level1Scene) currentScene = level2Scene;
	else if (lvScn == level2Scene) currentScene = level3Scene;
	else if (lvScn == level3Scene) currentScene = level4Scene;
	else if (lvScn == level4Scene) currentScene = level5Scene;
	else if (lvScn == level5Scene) { changeState(CREDITS); return;  }

	currentScene->init();
}


