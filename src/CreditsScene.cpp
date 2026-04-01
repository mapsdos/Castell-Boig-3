#include "CreditsScene.h"
#include "Game.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <vector>
#include "SFX.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

CreditsScene::CreditsScene()
{
	backgroundSprite = nullptr;
	currentCreditIndex = 0;
	timePerCredit = 3300; // 3 segundos por imagen
	currentTime = 0.0f;
	isActive = false;
}

CreditsScene::~CreditsScene()
{
	if (backgroundSprite != nullptr) delete backgroundSprite;

	// Liberar todas las texturas y sprites de créditos
	for (auto* tex : creditTextures) {
		if (tex != nullptr) delete tex;
	}
	for (auto* sprite : creditSprites) {
		if (sprite != nullptr) delete sprite;
	}
	creditTextures.clear();
	creditSprites.clear();
}

void CreditsScene::init()
{
	initShaders();
	projection = glm::ortho(0.f, 80.f, 60.f, 0.f);

	// Cargar background (fondo estático)
	if (backgroundTexture.loadFromFile("assets/images/credits/credits.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		std::cout << "Background cargado correctamente" << std::endl;
		backgroundTexture.setMinFilter(GL_NEAREST);
		backgroundTexture.setMagFilter(GL_NEAREST);

		backgroundSprite = Sprite::createSprite(
			glm::vec2(80.f, 60.f),
			glm::vec2(1.0f, 1.0f),
			&backgroundTexture,
			&texProgram
		);
		backgroundSprite->setPosition(glm::vec2(0.f, 0.f));
	}

	// Cargar todas las imágenes de créditos
	loadCreditsImages();

	SFX::instance().playMusic("assets/audio/credits.mp3", true, 50.f);
	isActive = true;
}

void CreditsScene::loadCreditsImages()
{
	// Aquí defines las imágenes de crédito en orden
	// Puedes tener tantas como quieras (crédito1.png, crédito2.png, etc.)
	std::vector<std::string> creditFiles = {
		"assets/images/credits/projecte creat per.png",
		"assets/images/credits/noms.png",
		"assets/images/credits/agraiment.png",
	};

	std::cout << "\nCargando imágenes de créditos:" << std::endl;

	for (const auto& file : creditFiles) {
		// Crear nueva textura
		Texture* texture = new Texture();
		if (texture->loadFromFile(file.c_str(), TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "Cargada: " << file << " ("
				<< texture->width() << "x" << texture->height() << ")" << std::endl;

			texture->setMinFilter(GL_NEAREST);
			texture->setMagFilter(GL_NEAREST);

			// Calcular tamaño manteniendo aspect ratio
			float aspectRatio = (float)texture->width() / (float)texture->height();
			float desiredWidth = 70.0f;
			if (file.c_str() == std::string("assets/images/credits/noms.png")) {
				desiredWidth = 60.0f; // Ajuste específico para esta imagen
			}
			float desiredHeight = desiredWidth / aspectRatio;

			if (desiredHeight > 50.0f) {
				desiredHeight = 50.0f;
				desiredWidth = desiredHeight * aspectRatio;
			}

			// Crear sprite
			Sprite* sprite = Sprite::createSprite(
				glm::vec2(desiredWidth, desiredHeight),
				glm::vec2(1.0f, 1.0f),
				texture,
				&texProgram
			);

			// Centrar en la pantalla
			float xPos = (80.0f - desiredWidth) / 2.0f;
			float yPos = (60.0f - desiredHeight) / 2.0f;
			sprite->setPosition(glm::vec2(xPos, yPos));

			// Guardar textura y sprite
			creditTextures.push_back(texture);
			creditSprites.push_back(sprite);
		}
		else {
			std::cout << "No se pudo cargar: " << file << std::endl;
			delete texture;
		}
	}

	std::cout << "Total imágenes de créditos cargadas: " << creditSprites.size() << std::endl;

	// Si no hay imágenes, mostrar mensaje de depuración
	if (creditSprites.empty()) {
		std::cout << "No se cargó ninguna imagen de créditos" << std::endl;
	}
}

void CreditsScene::update(int deltaTime)
{
	if (!isActive) return;

	// Si no hay imágenes de créditos, no hacer nada
	if (creditSprites.empty()) return;

	currentTime += deltaTime;

	// Cambiar de imagen cuando pasa el tiempo
	if (currentTime >= timePerCredit) {
		currentTime = 0.0f;
		currentCreditIndex = (currentCreditIndex + 1) % creditSprites.size();
		std::cout << "Cambiando a crédito " << currentCreditIndex + 1
			<< " de " << creditSprites.size() << std::endl;
	}

	// Volver al menú con M (ESC is handled globally in Game.cpp)
	if (Game::instance().getKey(GLFW_KEY_M)) {
		Game::instance().changeState(MAIN_MENU);
	}

	// Opcional: Permitir cambiar manualmente con flechas
	if (Game::instance().getKey(GLFW_KEY_RIGHT) || Game::instance().getKey(GLFW_KEY_SPACE)) {
		currentCreditIndex = (currentCreditIndex + 1) % creditSprites.size();
		currentTime = 0.0f;
		std::cout << "Crédito " << currentCreditIndex + 1 << std::endl;
	}
	if (Game::instance().getKey(GLFW_KEY_LEFT)) {
		currentCreditIndex = (currentCreditIndex - 1 + creditSprites.size()) % creditSprites.size();
		currentTime = 0.0f;
		std::cout << "Crédito " << currentCreditIndex + 1 << std::endl;
	}
}

void CreditsScene::render()
{
	glm::mat4 modelview;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// Render background (si existe)
	if (backgroundSprite != nullptr) {
		backgroundSprite->render(modelview);
	}

	// Render imagen de créditos actual (si existe)
	if (!creditSprites.empty() && currentCreditIndex < (int)creditSprites.size()) {
		if (creditSprites[currentCreditIndex] != nullptr) {
			creditSprites[currentCreditIndex]->render(modelview);
		}
	}
}

void CreditsScene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "assets/shaders/texture.vert");
	if (!vShader.isCompiled()) {
		std::cout << "Vertex Shader Error" << std::endl;
	}

	fShader.initFromFile(FRAGMENT_SHADER, "assets/shaders/texture.frag");
	if (!fShader.isCompiled()) {
		std::cout << "Fragment Shader Error" << std::endl;
	}

	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();

	if (!texProgram.isLinked()) {
		std::cout << "Shader Linking Error" << std::endl;
	}

	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}