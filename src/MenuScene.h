#ifndef _MENU_SCENE_INCLUDE
#define _MENU_SCENE_INCLUDE

#include "Scene.h"

struct Button {
    glm::vec2 pos;
    glm::vec2 size;
    int textureId;
};

// The "public Scene" part means MenuScene inherits from Scene
class MenuScene : public Scene
{
public:
    MenuScene();
    ~MenuScene();

    // "override" tells the compiler these replace the base Scene versions
    void init() override;
    void update(int deltaTime) override;
    void render() override;

private:
    void setupButtons();

private:
    Texture buttonSheet;
    Sprite* buttonSprites[3];
};

#endif


