#include "LevelD.h"
#include "Utility.h"

#define LEVEL_WIDTH 500
#define LEVEL_HEIGHT 140

constexpr char SPRITESHEET_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/skel.png",
           ENEMY_FILEPATH[]       = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/soph.png",
            MAP_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/mars_map.png",
            FONT_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/blue_font.png",
            CSV_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/mars_map_bigger.csv";

unsigned int LEVELD_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

LevelD::~LevelD()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelD::initialise(ShaderProgram *program) {
    
    m_game_state.next_scene_id = -1;
    
    Utility::readCSV(CSV_FILEPATH, LEVELD_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELD_DATA, map_texture_id, 1.0f, 7, 3);
    
    D_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
    std::vector<std::vector<int>> player_animation = {
        { 0 , 1 , 2 , 3  },   // facing forward
        { 4 , 5 , 6 , 7  },   // walking left
        { 8 , 9 , 10, 11 }   // walking right
    };
    
    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_animation,          // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        3,                         // animation row amount
        0.75f,                     // width
        1.0f,                      // height
        PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(2.0f, 10.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(5.0f);
    m_game_state.player->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, BLUE, IDLE);
    }

    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    
    // ShaderProgram stuff
    glm::mat4 new_projection_matrix = glm::ortho(-10.0f, 10.0f, -7.5f, 7.5f, -1.0f, 1.0f);
    
    program->set_projection_matrix(new_projection_matrix);
}

void LevelD::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    float view_left = std::max(m_game_state.player->get_position().x, 30.0f);
    float view_bottom = std::max(m_game_state.player->get_position().y - 18, -115.0f);
    
    message_pos = glm::vec3(view_left + 20, view_bottom + 20, 0.0f);
    fuel_pos = message_pos;
    fuel_pos.x += 5;
    
//    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
    if (m_game_state.player->get_collided_left() || m_game_state.player->get_collided_right()) {
        m_game_state.next_scene_id = 4;     // LOSER
    }
    else if (m_game_state.player->get_collided_bottom() && m_game_state.player->get_on_triangle()) {
        m_game_state.next_scene_id = 4;     // LOSER
    }
    else if (m_game_state.player->get_collided_bottom()) {
        m_game_state.next_scene_id = 5;     // WINNER
    }
}

void LevelD::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    Utility::draw_text(program, D_font_texture_id, "FUEL: ", 1.0f, 0.01f, message_pos);
    Utility::draw_text(program, D_font_texture_id, std::to_string(fuel_count),  1.0f, 0.01f, fuel_pos);
    m_game_state.player->render(program);
    
}