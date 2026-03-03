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

	int pressedButton;  // Botón que se está presionando (-1 si ninguno)
	float pressTime;    // Tiempo para controlar la duración de la presión
	const float PRESS_DURATION = 200.0f; // Duración en ms

	std::vector<Button> buttons;
	int selectedButton;

	Texture handTexture;
	Sprite* handSprite;

	// Control de teclado
	int keyCooldown;
	const int KEY_DELAY = 150;
	float currentTime;
};