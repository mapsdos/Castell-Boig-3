#include "Scene.h"
//#include "TexturedQuad.h" // Assuming you use this for buttons

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
    void initShaders() override;

private:
    //TexturedQuad* buttonQuads[3];
    Texture buttonTexs[3];
    // Add variables for your buttons here later
};

