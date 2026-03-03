#include "GraphicsConfig.h"
#include "Game.h"

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
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	state = MAIN_MENU;
	
	menuScene = new MenuScene();
	menuScene->init();
	currentScene = menuScene;

	level1Scene = new LevelScene();
	level1Scene->init("assets/levels/level01.txt");
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
		if (key == GLFW_KEY_1) state = PLAYING;
		if (key == GLFW_KEY_2) state = INSTRUCTIONS;
		if (key == GLFW_KEY_3) state = CREDITS;
	}
	else if (key == GLFW_KEY_M) { // Press M to return to menu
		state = MAIN_MENU;
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
	}
}


