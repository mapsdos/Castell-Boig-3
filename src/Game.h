#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include "GraphicsConfig.h"
#include "Scene.h"
#include "MenuScene.h"
#include "LevelScene.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

enum GameState {
	MAIN_MENU, PLAYING, INSTRUCTIONS, CREDITS
};

// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game();
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	void changeState(GameState newState);

	bool getKey(int key) const;
	glm::ivec2 Game::getMousePos() const;
	bool isMouseButtonPressed(int button) const;

private:
	GameState state;
	Scene* currentScene;
	MenuScene* menuScene;
	Scene* instructionsScene;
	Scene* creditsScene;
	LevelScene* level1Scene;
	LevelScene* level2Scene;
	LevelScene* level3Scene;
	LevelScene* level4Scene;
	LevelScene* level5Scene;


	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
							    // we can have access at any time

	glm::ivec2 mousePos;
	bool mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];

};


#endif // _GAME_INCLUDE


