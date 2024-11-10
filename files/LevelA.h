#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 5;
    GLuint A_font_texture_id;
    glm::vec3 message_pos;
    glm::vec3 fuel_pos;
    
    ~LevelA();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
