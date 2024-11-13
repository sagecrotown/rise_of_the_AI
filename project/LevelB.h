#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 5;
    GLuint B_font_texture_id;
    glm::vec3 message_pos;
    glm::vec3 fuel_pos;
    
    ~LevelB();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
