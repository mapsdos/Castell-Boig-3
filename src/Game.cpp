#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "MenuScene.h"

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

	level1Scene = new Scene();
	level1Scene->init();
}

bool Game::update(int deltaTime)
{
	switch (state) {
	case MAIN_MENU:
		menuScene->update(deltaTime);
		break;
	case PLAYING:
		level1Scene->update(deltaTime);
		break;
	case INSTRUCTIONS:
		instructionsScene->update(deltaTime);
		break;
	}

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
}

void Game::mousePress(int button)
{
	if (state == MAIN_MENU && button == GLFW_MOUSE_BUTTON_LEFT) {
		// Pseudo-code: Check if mouseX and mouseY are within your button quad's bounds
		//if (checkButtonCollision(mouseX, mouseY, playButtonBounds)) {
			//state = PLAYING;
		//}
	}
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}



