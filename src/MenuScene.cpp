#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "MenuScene.h"
#include "Game.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

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
	fadeSprite = nullptr;

	isAnimating = false;
	animFrame = 0;
	animTimer = 0.0f;
	animElapsed = 0.0f;
	waitForEnterRelease = false;
	
	fadingToGame = false;
	fadeToGameTimer = 0.0f;
	fadeToGameAlpha = 0.0f;
}

MenuScene::~MenuScene()
{
	if (titleSprite != nullptr) delete titleSprite;
	if (backgroundSprite != nullptr) delete backgroundSprite;
	if (handSprite != nullptr) delete handSprite;
	if (fadeSprite != nullptr) delete fadeSprite;

	for (auto& button : buttons) {
		if (button.sprite != nullptr) delete button.sprite;
		if (button.texture != nullptr) delete button.texture;
	}

	for (auto* s : animSprites) { if (s) delete s; }
	for (auto* t : animTextures) { if (t) delete t; }
	animSprites.clear();
	animTextures.clear();
}

// ---------------------------------------------------------------------------
// loadButtonTextures
// ---------------------------------------------------------------------------

void MenuScene::loadButtonTextures()
{
	const char* buttonFiles[4] = {
		"assets/images/menu/buttons/spritesheet play button.png",
		"assets/images/menu/buttons/spritesheet guide button.png",
		"assets/images/menu/buttons/spritesheet credits button.png",
		"assets/images/menu/buttons/spritesheet exit button.png"
		
	};

	float centerX = 67.f;
	float startY = 18.f;
	float spacing = 5.5f;
	float buttonWidth = 25.f / 2.25f;
	float buttonHeight = 10.f / 2.25f;

	float sheetWidth = 114.0f;
	float sheetHeight = 16.0f;
	float frameWidthUV = (sheetWidth / 3.0f) / sheetWidth;
	float frameHeightUV = 1.0f;

	for (int i = 0; i < 4; i++) {
		Button newButton;

		newButton.texture = new Texture();
		if (!newButton.texture->loadFromFile(buttonFiles[i], TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "ERROR: No se pudo cargar " << buttonFiles[i] << std::endl;
			continue;
		}
		newButton.texture->setMinFilter(GL_NEAREST);
		newButton.texture->setMagFilter(GL_NEAREST);

		newButton.sprite = Sprite::createSprite(
			glm::vec2(buttonWidth, buttonHeight),
			glm::vec2(frameWidthUV, frameHeightUV),
			newButton.texture,
			&texProgram
		);

		newButton.sprite->setNumberAnimations(3);

		newButton.sprite->setAnimationSpeed(0, 1);
		newButton.sprite->addKeyframe(0, glm::vec2(0.f, 0.f));

		newButton.sprite->setAnimationSpeed(1, 1);
		newButton.sprite->addKeyframe(1, glm::vec2(frameWidthUV, 0.f));

		newButton.sprite->setAnimationSpeed(2, 3);
		newButton.sprite->addKeyframe(2, glm::vec2(0.f, 0.f));
		newButton.sprite->addKeyframe(2, glm::vec2(2 * frameWidthUV, 0.f));

		newButton.sprite->changeAnimation(0);

		newButton.position = glm::vec2(centerX - buttonWidth / 2, startY + i * spacing);
		newButton.sprite->setPosition(newButton.position);
		newButton.animState = 0;
		newButton.animTime = 0.0f;

		buttons.push_back(newButton);
	}
}

// ---------------------------------------------------------------------------
// loadAnimationFrames
// ---------------------------------------------------------------------------

void MenuScene::loadAnimationFrames()
{
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

	float xOffset = (80.f - width) / 2.0f;
	float yOffset = (60.f - height) / 2.0f;

	animTextures.reserve(ANIM_FRAME_COUNT);
	animSprites.reserve(ANIM_FRAME_COUNT);

	for (int i = 0; i < ANIM_FRAME_COUNT; i++) {
		std::string path = "assets/images/menu/" + std::to_string(i) + ".png";

		Texture* tex = new Texture();
		if (!tex->loadFromFile(path.c_str(), TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "WARNING: Could not load animation frame: " << path << std::endl;
			animTextures.push_back(nullptr);
			animSprites.push_back(nullptr);
			delete tex;
			continue;
		}

		tex->setMinFilter(GL_NEAREST);
		tex->setMagFilter(GL_NEAREST);

		Sprite* spr = Sprite::createSprite(
			glm::vec2(width, height),
			glm::vec2(1.0f, 1.0f),
			tex,
			&texProgram
		);
		spr->setPosition(glm::vec2(xOffset, yOffset));

		animTextures.push_back(tex);
		animSprites.push_back(spr);
	}

	std::cout << "Animation frames loaded: " << ANIM_FRAME_COUNT << std::endl;
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------

void MenuScene::init()
{
	// Reset animation states each time menu is entered
	isAnimating = false;
	animFrame = 0;
	animTimer = 0.0f;
	animElapsed = 0.0f;
	waitForEnterRelease = false;
	fadingToGame = false;
	fadeToGameTimer = 0.0f;
	fadeToGameAlpha = 0.0f;
	selectedButton = 0;
	pressedButton = -1;
	pressTime = 0.0f;
	keyCooldown = 0;

	// Reset button animations to selected state for first button
	for (int i = 0; i < (int)buttons.size(); i++) {
		if (i == 0) {
			buttons[i].sprite->changeAnimation(2); // selected animation
			buttons[i].animState = 2;
		} else {
			buttons[i].sprite->changeAnimation(0); // normal animation
			buttons[i].animState = 0;
		}
	}

	// Only initialize resources once (first time)
	if (backgroundSprite != nullptr) return;

	initShaders();

	projection = glm::ortho(0.f, 80.f, 60.f, 0.f);

	// Background
	if (backgroundTexture.loadFromFile("assets/images/menu/0.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		float imgAspect = 80.f / 45.f;
		float screenAspect = 80.f / 60.f;
		float width, height;
		if (imgAspect > screenAspect) { width = 80.f; height = 80.f / imgAspect; }
		else { height = 60.f; width = 60.f * imgAspect; }

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

	// Title
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

	// Pointing hand
	if (handTexture.loadFromFile("assets/images/menu/buttons/pointing hand.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		handTexture.setMinFilter(GL_NEAREST);
		handTexture.setMagFilter(GL_NEAREST);

		float handWidth = 13.f / 4.25f;
		float handHeight = 8.f / 4.25f;

		handSprite = Sprite::createSprite(
			glm::vec2(handWidth, handHeight),
			glm::vec2(1.0f, 1.0f),
			&handTexture,
			&texProgram
		);
		handSprite->setPosition(glm::vec2(0.f, 0.f));
	}
	else {
		std::cout << "ERROR: No se pudo cargar pointing hand.png" << std::endl;
	}

	loadAnimationFrames();

	// Fade sprite (1x1 white pixel scaled to screen)
	if (fadeTexture.loadFromFile("assets/images/white.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		fadeSprite = Sprite::createSprite(
			glm::vec2(80.f, 60.f),
			glm::vec2(1.0f, 1.0f),
			&fadeTexture,
			&texProgram
		);
		fadeSprite->setPosition(glm::vec2(0.f, 0.f));
	}
}

// ---------------------------------------------------------------------------
// update
// ---------------------------------------------------------------------------

void MenuScene::update(int deltaTime)
{
	currentTime += deltaTime;

	// =========================================================
	// FADE TO GAME
	// =========================================================
	if (fadingToGame) {
		fadeToGameTimer += deltaTime;
		fadeToGameAlpha = fadeToGameTimer / FADE_TO_GAME_DURATION;
		if (fadeToGameAlpha > 1.0f) fadeToGameAlpha = 1.0f;
		
		if (fadeToGameTimer >= FADE_TO_GAME_DURATION) {
			fadingToGame = false;
			Game::instance().changeState(PLAYING);
		}
		return;
	}

	// =========================================================
	// ANIMATION MODE
	// =========================================================
	if (isAnimating) {
		animTimer += deltaTime;
		animElapsed += deltaTime;

		// Advance frame
		if (animTimer >= FRAME_DURATION) {
			animTimer -= FRAME_DURATION;
			animFrame = (animFrame + 1) % ANIM_FRAME_COUNT;
		}

		bool enterCurrentlyHeld = Game::instance().getKey(GLFW_KEY_ENTER);

		// Step 1: wait until the player has RELEASED the ENTER key that
		//         triggered the animation in the first place.
		if (waitForEnterRelease) {
			if (!enterCurrentlyHeld)
				waitForEnterRelease = false;  // key released → now we can skip
			return; // don't process skip logic yet
		}

		// Step 2: now a *fresh* ENTER press or the timeout can transition to the game
		bool autoSkip = (animElapsed >= AUTO_SKIP_DELAY);
		if (enterCurrentlyHeld || autoSkip) {
			// Start fade out instead of immediate transition
			isAnimating = false;
			fadingToGame = true;
			fadeToGameTimer = 0.0f;
			fadeToGameAlpha = 0.0f;
		}

		return;
	}

	// =========================================================
	// NORMAL MENU MODE
	// =========================================================

	for (auto& button : buttons)
		button.sprite->update(deltaTime);

	for (int i = 0; i < (int)buttons.size(); i++) {
		if (i == pressedButton) {
			if (buttons[i].animState != 1) {
				buttons[i].sprite->changeAnimation(1);
				buttons[i].animState = 1;
			}
		}
		else if (i == selectedButton) {
			if (buttons[i].animState != 2) {
				buttons[i].sprite->changeAnimation(2);
				buttons[i].animState = 2;
			}
		}
		else {
			if (buttons[i].animState != 0) {
				buttons[i].sprite->changeAnimation(0);
				buttons[i].animState = 0;
			}
		}
	}

	if (pressedButton != -1) {
		pressTime += deltaTime;
		if (pressTime >= PRESS_DURATION) {
			pressedButton = -1;
			pressTime = 0.0f;
		}
	}

	if (handSprite != nullptr && selectedButton >= 0 && selectedButton < (int)buttons.size()) {
		float handX = buttons[selectedButton].position.x + buttons[selectedButton].sprite->getSize().x + 1.f;
		float handY = buttons[selectedButton].position.y
			+ (buttons[selectedButton].sprite->getSize().y - handSprite->getSize().y) / 2.f;
		handSprite->setPosition(glm::vec2(handX, handY));
	}

	if (keyCooldown > 0)
		keyCooldown -= deltaTime;

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

	if (pressedButton == -1 && Game::instance().getKey(GLFW_KEY_ENTER)) {
		pressedButton = selectedButton;
		pressTime = 0.0f;
		activateCurrentButton();
		keyCooldown = KEY_DELAY * 2;
	}
}

// ---------------------------------------------------------------------------
// moveSelection / activateCurrentButton
// ---------------------------------------------------------------------------

void MenuScene::moveSelection(int direction)
{
	selectedButton += direction;
	if (selectedButton < 0)
		selectedButton = (int)buttons.size() - 1;
	else if (selectedButton >= (int)buttons.size())
		selectedButton = 0;
}

void MenuScene::activateCurrentButton()
{
	switch (selectedButton) {
	case 0: // PLAY
		std::cout << "JUGAR seleccionado – starting intro animation" << std::endl;
		isAnimating = true;
		animFrame = 0;
		animTimer = 0.0f;
		animElapsed = 0.0f;
		waitForEnterRelease = true;  // <-- ignore the key until it's released
		break;

	case 1:
		std::cout << "INSTRUCCIONES seleccionado" << std::endl;
		Game::instance().changeState(INSTRUCTIONS);
		break;

	case 2:
		std::cout << "CREDITOS seleccionado" << std::endl;
		Game::instance().changeState(CREDITS);
		break;
	case 3:
		std::cout << "SALIR seleccionado" << std::endl;
		exit(0);
		break;
	}
}

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------

void MenuScene::render()
{
	glm::mat4 modelview;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// =========================================================
	// FADE TO GAME - render last animation frame + fade overlay
	// =========================================================
	if (fadingToGame) {
		// Show the last frame of animation
		int lastFrame = ANIM_FRAME_COUNT - 1;
		if (lastFrame < (int)animSprites.size() && animSprites[lastFrame] != nullptr)
			animSprites[lastFrame]->render(modelview);
		
		// Render fade overlay
		if (fadeSprite != nullptr) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			texProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, fadeToGameAlpha);
			fadeSprite->render(modelview);
			glDisable(GL_BLEND);
			texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		}
		return;
	}

	// =========================================================
	// ANIMATION MODE
	// =========================================================
	if (isAnimating) {
		if (animFrame < (int)animSprites.size() && animSprites[animFrame] != nullptr)
			animSprites[animFrame]->render(modelview);
		return;
	}

	// =========================================================
	// NORMAL MENU
	// =========================================================
	if (backgroundSprite != nullptr)
		backgroundSprite->render(modelview);

	if (titleSprite != nullptr)
		titleSprite->render(modelview);

	for (auto& button : buttons)
		button.sprite->render(modelview);

	if (handSprite != nullptr)
		handSprite->render(modelview);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&projection[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (int i = 0; i < (int)buttons.size(); i++) {
		glm::vec3 textColor = (i == selectedButton)
			? glm::vec3(1.0f, 1.0f, 0.0f)
			: glm::vec3(1.0f, 1.0f, 1.0f);

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBegin(GL_QUADS);
		glColor4f(textColor.r, textColor.g, textColor.b, 1.0f);
		glEnd();
		glPopAttrib();
	}

	texProgram.use();
}

// ---------------------------------------------------------------------------
// initShaders
// ---------------------------------------------------------------------------

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