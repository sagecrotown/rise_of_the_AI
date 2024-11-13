#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 5;
    GLuint C_font_texture_id;
    glm::vec3 message_pos;
    
    ~LevelC();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
