#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

enum GameState {
	MAIN_MENU, PLAYING, INSTRUCTIONS, CREDITS
};

class MenuScene;

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

	bool getKey(int key) const;

private:
	GameState state;
	Scene* currentScene;
	MenuScene* menuScene;
	Scene* instructionsScene;
	Scene* creditsScene;
	Scene* level1Scene;
	Scene* level2Scene;
	Scene* level3Scene;
	Scene* level4Scene;
	Scene* level5Scene;
	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
							    // we can have access at any time

};


#endif // _GAME_INCLUDE


