#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 30
#define LEVEL_HEIGHT 20

constexpr char SPRITESHEET_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/skel.png",
           ENEMY_FILEPATH[]       = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/aiiiii.png",
            MAP_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/ai_tile.png",
          FONT_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/blue_font.png",
            CSV_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/ai_map.csv";

unsigned int LEVELA_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

LevelA::~LevelA() {
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise(ShaderProgram *program) {
    m_game_state.next_scene_id = -1;
    
    Utility::readCSV(CSV_FILEPATH, LEVELA_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    A_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
    std::vector<std::vector<int>> player_animation = {
        { 0 , 1 , 2 , 3  },   // facing forward
        { 4 , 5 , 6 , 7  },   // walking left
        { 8 , 9 , 10, 11 }   // walking right
    };
    
    glm::vec3 acceleration = glm::vec3(0.0f, -9.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        8.0f,                      // jumping power
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
        
    m_game_state.player->set_position(glm::vec3(10.0f, 0.0f, 0.0f));
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i] =  Entity(enemy_texture_id,
                                          1.0f,             // speed
                                          1.0f,             // width
                                          1.0f,             // height
                                          ENEMY,            // entity type
                                          BLUE,             // AI type
                                          IDLE);            // AI state
        m_game_state.enemies[i].set_position(glm::vec3(10.0f + i, 0.0f - i, 0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }
    
    m_game_state.enemies[1].set_ai_type(PURPLE);
    m_game_state.enemies[2].set_ai_type(YELLOW);
    m_game_state.enemies[3].set_ai_type(GREEN);
    m_game_state.enemies[4].set_ai_type(PINK);

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map);
    }
//    std::cout << m_game_state.player->get_position().x << std::endl;
//    if (m_game_state.player->get_position().y < -30.0f) m_game_state.next_scene_id = 1;
}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    Utility::draw_text(program, A_font_texture_id, "FIGHT FIGHT FIGHT", 0.5f, 0.01f, glm::vec3(1.0f, -1.0f , 0.0f));
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(program);
    }
    m_game_state.player->render(program);
    
}
