#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "MenuScene.h"
#include "Game.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

MenuScene::MenuScene()
{
	selectedButton = 0;
	keyCooldown = 0;
	currentTime = 0.0f;
	pressedButton = -1; 
	pressTime = 0.0f;

	titleSprite = nullptr;
	backgroundSprite = nullptr;
	handSprite = nullptr;
}

MenuScene::~MenuScene()
{
	if (titleSprite != nullptr)
		delete titleSprite;

	if (backgroundSprite != nullptr)
		delete backgroundSprite;

	if (handSprite != nullptr) 
		delete handSprite;

	for (auto& button : buttons) {
		if (button.sprite != nullptr)
			delete button.sprite;
		if (button.texture != nullptr)
			delete button.texture;
	}
}

void MenuScene::loadButtonTextures()
{
	// Array con los nombres de los archivos de botones
	const char* buttonFiles[3] = {
		"assets/images/menu/buttons/spritesheet play button.png",
		"assets/images/menu/buttons/spritesheet guide button.png",
		"assets/images/menu/buttons/spritesheet exit button.png"
	};

	// Posiciones de los botones
	float centerX = 67.f;
	float startY = 18.f;
	float spacing = 5.5f;

	// Tamaño del botón en pantalla
	float buttonWidth = 25.f / 2.25f;
	float buttonHeight = 10.f / 2.25f;

	// Calcular el tamaño UV correctamente
	float sheetWidth = 114.0f;
	float sheetHeight = 16.0f;
	float frameWidthUV = (sheetWidth / 3.0f) / sheetWidth; // (38/114) = 0.3333
	float frameHeightUV = 1.0f;

	for (int i = 0; i < 3; i++) {
		Button newButton;

		// Cargar textura del botón
		newButton.texture = new Texture();
		if (!newButton.texture->loadFromFile(buttonFiles[i], TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "ERROR: No se pudo cargar " << buttonFiles[i] << std::endl;
			continue;
		}

		newButton.texture->setMinFilter(GL_NEAREST);
		newButton.texture->setMagFilter(GL_NEAREST);

		// Crear sprite del botón
		newButton.sprite = Sprite::createSprite(
			glm::vec2(buttonWidth, buttonHeight),
			glm::vec2(frameWidthUV, frameHeightUV),
			newButton.texture,
			&texProgram
		);

		// Configurar las 3 animaciones del botón
		newButton.sprite->setNumberAnimations(3);

		// Animación 0: Estado normal (primer frame)
		newButton.sprite->setAnimationSpeed(0, 1);
		newButton.sprite->addKeyframe(0, glm::vec2(0.f, 0.f));

		// Animación 1: Botón presionado (segundo frame)
		newButton.sprite->setAnimationSpeed(1, 1);
		newButton.sprite->addKeyframe(1, glm::vec2(frameWidthUV, 0.f));

		// Animación 2: Botón seleccionado (intercala entre frame NORMAL y GRIS)
		// CORREGIDO: Ahora usa frame 0 (normal) y frame 2 (gris)
		newButton.sprite->setAnimationSpeed(2, 3); // 4 fps
		newButton.sprite->addKeyframe(2, glm::vec2(0.f, 0.f));              // Frame normal (0)
		newButton.sprite->addKeyframe(2, glm::vec2(2 * frameWidthUV, 0.f)); // Frame gris (2)

		// Empezar con animación normal
		newButton.sprite->changeAnimation(0);

		// Posicionar el botón
		newButton.position = glm::vec2(centerX - buttonWidth / 2, startY + i * spacing);
		newButton.sprite->setPosition(newButton.position);

		// Guardar estado inicial
		newButton.animState = 0;
		newButton.animTime = 0.0f;

		buttons.push_back(newButton);
	}
}

void MenuScene::init()
{
	initShaders();

	// Proyección que coincide exactamente con la pantalla
	projection = glm::ortho(0.f, 80.f, 60.f, 0.f);

	// ===== 1. CARGAR FONDO =====
	if (backgroundTexture.loadFromFile("assets/images/menu/0.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		float imgAspect = 80.f / 45.f;
		float screenAspect = 80.f / 60.f;

		float width, height;
		if (imgAspect > screenAspect) {
			width = 80.f;
			height = 80.f / imgAspect;
		}
		else {
			height = 60.f;
			width = 60.f * imgAspect;
		}

		backgroundSprite = Sprite::createSprite(
			glm::vec2(width, height),
			glm::vec2(1.0f, 1.0f),
			&backgroundTexture,
			&texProgram
		);

		float xOffset = (80.f - width) / 2.0f;
		float yOffset = (60.f - height) / 2.0f;
		backgroundSprite->setPosition(glm::vec2(xOffset, yOffset));

		backgroundTexture.setMinFilter(GL_NEAREST);
		backgroundTexture.setMagFilter(GL_NEAREST);
	}

	// ===== 2. CARGAR TÍTULO =====
	if (titleTexture.loadFromFile("assets/images/menu/title.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		titleTexture.setMinFilter(GL_NEAREST);
		titleTexture.setMagFilter(GL_NEAREST);

		float originalWidth = 559.f;
		float originalHeight = 108.f;
		float targetWidth = 70.f;
		float targetHeight = (targetWidth / originalWidth) * originalHeight;

		if (targetHeight > 20.f) {
			targetHeight = 20.f;
			targetWidth = (targetHeight / originalHeight) * originalWidth;
		}

		titleSprite = Sprite::createSprite(
			glm::vec2(targetWidth / 3, targetHeight / 3),
			glm::vec2(1.0f, 1.0f),
			&titleTexture,
			&texProgram
		);

		titleSprite->setPosition(glm::vec2(55.f, 12.f));
	}

	loadButtonTextures();

	if (handTexture.loadFromFile("assets/images/menu/buttons/pointing hand.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		handTexture.setMinFilter(GL_NEAREST);
		handTexture.setMagFilter(GL_NEAREST);

		// La imagen es 26x16, mantener tamaño similar a los botones
		float handWidth = 13.f/4.25f;  // Escalado para que sea visible pero no demasiado grande
		float handHeight = 8.f/4.25f;  // Mantener proporción (26/16 ≈ 1.625)

		handSprite = Sprite::createSprite(
			glm::vec2(handWidth, handHeight),
			glm::vec2(1.0f, 1.0f),  // Usar toda la textura (solo 1 frame)
			&handTexture,
			&texProgram
		);

		// La posición se actualizará en update()
		handSprite->setPosition(glm::vec2(0.f, 0.f));
	}
	else {
		std::cout << "ERROR: No se pudo cargar pointing hand.png" << std::endl;
	}
}

void MenuScene::update(int deltaTime)
{
	currentTime += deltaTime;

	// Actualizar sprites de los botones
	for (auto& button : buttons) {
		button.sprite->update(deltaTime);
	}

	// --- ACTUALIZAR ANIMACIONES DE LOS BOTONES ---
	for (int i = 0; i < buttons.size(); i++) {
		if (i == pressedButton) {
			// Botón presionado: animación 1 (frame presionado)
			if (buttons[i].animState != 1) {
				buttons[i].sprite->changeAnimation(1);
				buttons[i].animState = 1;
			}
		}
		else if (i == selectedButton) {
			// Botón seleccionado: animación 2 (intercala normal/gris)
			if (buttons[i].animState != 2) {
				buttons[i].sprite->changeAnimation(2);
				buttons[i].animState = 2;
			}
		}
		else {
			// Botón no seleccionado: animación normal
			if (buttons[i].animState != 0) {
				buttons[i].sprite->changeAnimation(0);
				buttons[i].animState = 0;
			}
		}
	}

	// --- CONTROLAR DURACIÓN DEL ESTADO PRESIONADO ---
	if (pressedButton != -1) {
		pressTime += deltaTime;
		if (pressTime >= PRESS_DURATION) {
			// Terminar efecto de presión
			pressedButton = -1;
			pressTime = 0.0f;
		}
	}

	// Posicionar mano (solo si no hay botón presionado o después de la presión)
	if (handSprite != nullptr && selectedButton >= 0 && selectedButton < buttons.size()) {
		float handX = buttons[selectedButton].position.x + buttons[selectedButton].sprite->getSize().x + 1.f;
		float handY = buttons[selectedButton].position.y + (buttons[selectedButton].sprite->getSize().y - handSprite->getSize().y) / 2.f;
		handSprite->setPosition(glm::vec2(handX, handY));
	}

	// Actualizar cooldown de teclas
	if (keyCooldown > 0) {
		keyCooldown -= deltaTime;
	}

	// --- NAVEGACIÓN CON TECLADO (solo si no hay botón presionado) ---
	if (pressedButton == -1 && keyCooldown <= 0) {
		if (Game::instance().getKey(GLFW_KEY_UP)) {
			moveSelection(-1);
			keyCooldown = KEY_DELAY;
		}
		else if (Game::instance().getKey(GLFW_KEY_DOWN)) {
			moveSelection(1);
			keyCooldown = KEY_DELAY;
		}
	}

	// --- ACTIVAR BOTÓN CON ENTER ---
	if (pressedButton == -1 && Game::instance().getKey(GLFW_KEY_ENTER)) {
		// Activar efecto de presión
		pressedButton = selectedButton;
		pressTime = 0.0f;

		// Ejecutar acción del botón (con un pequeño retraso para ver la animación)
		activateCurrentButton();

		keyCooldown = KEY_DELAY * 2;
	}
}

void MenuScene::moveSelection(int direction)
{
	selectedButton += direction;

	// Limitar entre 0 y número de botones -1
	if (selectedButton < 0)
		selectedButton = buttons.size() - 1;
	else if (selectedButton >= (int)buttons.size())
		selectedButton = 0;
}

void MenuScene::activateCurrentButton()
{
	switch (selectedButton) {
	case 0: // JUGAR
		std::cout << "JUGAR seleccionado" << std::endl;
		// Game::instance().changeState(PLAYING);
		break;
	case 1: // INSTRUCCIONES
		std::cout << "INSTRUCCIONES seleccionado" << std::endl;
		break;
	case 2: // CREDITOS
		std::cout << "CREDITOS seleccionado" << std::endl;
		break;
	}
}

void MenuScene::render()
{
	glm::mat4 modelview;

	// Limpiar pantalla
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Usar shaders
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// ===== 1. RENDERIZAR FONDO =====
	if (backgroundSprite != nullptr) {
		backgroundSprite->render(modelview);
	}

	// ===== 2. RENDERIZAR TÍTULO =====
	if (titleSprite != nullptr) {
		titleSprite->render(modelview);
	}

	// ===== 3. RENDERIZAR BOTONES =====
	for (auto& button : buttons) {
		button.sprite->render(modelview);
	}

	if (handSprite != nullptr) {
		handSprite->render(modelview);
	}

	// Configurar matrices para 2D
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&projection[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (int i = 0; i < buttons.size(); i++) {
		// Determinar color del texto según selección
		glm::vec3 textColor;
		if (i == selectedButton) {
			// Botón seleccionado: texto amarillo brillante
			textColor = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else {
			// Botón no seleccionado: texto blanco
			textColor = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		// Dibujar texto (usando rectángulos simples)
		float textX = buttons[i].position.x + 5.0f; // Offset para centrar aproximadamente
		float textY = buttons[i].position.y + 3.0f;

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBegin(GL_QUADS);
		glColor4f(textColor.r, textColor.g, textColor.b, 1.0f);

		glEnd();
		glPopAttrib();
	}

	// Reactivar shaders
	texProgram.use();
}

void MenuScene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "assets/shaders/texture.vert");
	if (!vShader.isCompiled()) {
		std::cout << "Vertex Shader Error" << std::endl;
		std::cout << vShader.log() << std::endl;
	}

	fShader.initFromFile(FRAGMENT_SHADER, "assets/shaders/texture.frag");
	if (!fShader.isCompiled()) {
		std::cout << "Fragment Shader Error" << std::endl;
		std::cout << fShader.log() << std::endl;
	}

	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();

	if (!texProgram.isLinked()) {
		std::cout << "Shader Linking Error" << std::endl;
		std::cout << texProgram.log() << std::endl;
	}

	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}