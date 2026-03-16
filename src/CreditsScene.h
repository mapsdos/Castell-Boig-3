#ifndef _CREDITS_SCENE_H_
#define _CREDITS_SCENE_H_

#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include <vector>

class CreditsScene : public Scene
{
public:
	CreditsScene();
	~CreditsScene();

	virtual void init();
	virtual void update(int deltaTime);
	virtual void render();

private:
	void initShaders();
	void loadCreditsImages();

	Texture backgroundTexture;
	Sprite* backgroundSprite;

	std::vector<Texture*> creditTextures;    // Texturas de cada imagen de crédito
	std::vector<Sprite*> creditSprites;      // Sprites de cada imagen de crédito

	int currentCreditIndex;                   // Índice de la imagen actual
	float timePerCredit;                      // Tiempo que se muestra cada imagen (ms)
	float currentTime;                         // Tiempo acumulado

	ShaderProgram texProgram;
	glm::mat4 projection;

	bool isActive;
};

#endif