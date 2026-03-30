#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE

#include "Entity.h"

class LevelScene;

class Door : public Entity {
public:
    Entity* clone(ShaderProgram&) const override;
public:
    // Ensure these signatures match Entity.h exactly
    void init(const glm::vec2& pos, ShaderProgram& program) override;
    void update(int deltaTime) override;
    void render(const glm::mat4& modelview) override;

    void setRoom(LevelScene* setRoom);
    LevelScene* getRoom() const;

    void setOpened(bool open) { isOpen = open; }
    bool isOpened() const { return isOpen; }
    
private:
    bool isOpen = false;
    Sprite* opened;
    Texture spritesheetOpened;
    LevelScene* room;
};

#endif