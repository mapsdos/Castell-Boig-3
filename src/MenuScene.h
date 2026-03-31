#pragma once
#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

class MenuScene : public Scene
{
public:
	MenuScene();
	~MenuScene();
	virtual void init();
	virtual void update(int deltaTime);
	virtual void render();

private:
	void initShaders();
	void moveSelection(int direction);
	void activateCurrentButton();
	void loadButtonTextures();
	void loadAnimationFrames();

	ShaderProgram texProgram;
	glm::mat4 projection;

	// Título
	Texture titleTexture;
	Sprite* titleSprite;

	// Fondo
	Texture backgroundTexture;
	Sprite* backgroundSprite;

	// Botones con sprites
	struct Button {
		Sprite* sprite;
		Texture* texture;
		glm::vec2 position;
		int animState; // 0 = normal, 1 = pressed, 2 = selected
		float animTime;
	};

	int pressedButton;
	float pressTime;
	const float PRESS_DURATION = 200.0f;

	std::vector<Button> buttons;
	int selectedButton;

	Texture handTexture;
	Sprite* handSprite;

	// Control de teclado
	int keyCooldown;
	const int KEY_DELAY = 150;
	float currentTime;

	// ===== PRE-GAME ANIMATION =====
	bool isAnimating;
	int  animFrame;
	float animTimer;
	float animElapsed;
	const float FRAME_DURATION = 100.0f;  
	const int   ANIM_FRAME_COUNT = 39;
	const float AUTO_SKIP_DELAY = FRAME_DURATION*ANIM_FRAME_COUNT; // ms before auto-transition

	// Prevents the ENTER press that *started* the animation from
	// immediately skipping it on the very next update tick.
	bool waitForEnterRelease;

	// ===== FADE OUT TO GAME =====
	bool fadingToGame;
	float fadeToGameTimer;
	float fadeToGameAlpha;
	const float FADE_TO_GAME_DURATION = 500.0f;
	Sprite* fadeSprite;
	Texture fadeTexture;

	std::vector<Texture*> animTextures;
	std::vector<Sprite*>  animSprites;
};