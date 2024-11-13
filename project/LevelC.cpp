#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 10

constexpr char SPRITESHEET_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/skel.png",
           ENEMY_FILEPATH[]       = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/aiiiii.png",
            MAP_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/ai_tile.png",
          FONT_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/blue_font.png",
            CSV_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/ai_game_over.csv";

constexpr char* COLOR_FILEPATHS[] = {
    "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/blue_spot.png",
    "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/red_spot.png",
    "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/yellow_spot.png",
    "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/green_spot.png",
    "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/project/assets/purple_spot.png",
};

unsigned int LEVELC_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

LevelC::~LevelC() {
    for (int i = 0; i < m_game_state.enemies.size(); i++) {
        delete m_game_state.enemies[i];
    }
//    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise(ShaderProgram *program) {

    m_game_state.next_scene_id = -1;
    
    Utility::readCSV(CSV_FILEPATH, LEVELC_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);
    
    C_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
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
        
    m_game_state.player->set_position(glm::vec3(4.5f, -5.0f, 0.0f));
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        GLuint color_texture_id = Utility::load_texture(COLOR_FILEPATHS[i]);
        
        m_game_state.colors.push_back( new Entity( color_texture_id,          // texture id
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
                                                   COLOR
                                               )
                                      );
        m_game_state.colors[i]->set_position(m_game_state.player->get_position());
        if (m_game_state.active_colors[i]) m_game_state.colors[i]->activate();
        else m_game_state.colors[i]->deactivate();
    }
    
    
    //Enemies' stuff
    
    // TODO: find better solution than dynamic cast (but not tonight)
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    
    std::vector<std::vector<int>> enemy_animation_frames = {
        { 0 , 1 , 2 , 3  },   // blue
        { 4 , 5 , 6 , 7  },   // red
        { 8 , 9 , 10, 11 },   // yellow
        { 12, 13, 14, 15 },   // green right
        { 16, 17, 18, 19 },   // green left
        { 20, 21, 22, 23 },   // purple right
        { 24, 25, 26, 27 }    // purple left
    };
    

    for (int i = 0; i < ENEMY_COUNT; i++) {
        
        m_game_state.enemies.push_back( new AI(enemy_texture_id,          // tex id
                                          1.0f + i * 0.1f,                      // speed
                                          acceleration,              // acceleration
                                          8.0f,                      // jumping power
                                          enemy_animation_frames,    // animation index sets
                                          0.0f,                      // animation time
                                          4,                         // animation frame amount
                                          0,                         // current animation index
                                          4,                         // animation column amount
                                          7,                         // animation row amount
                                          0.f,                      // width
                                          1.0f,                      // height
                                          ENEMY,                     // entity type
                                          BLUE,                      // AI type
                                          IDLE)                     // AI state
                                       );
        m_game_state.enemies[i]->deactivate();
    }
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelC::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.colors, m_game_state.map);
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.colors, m_game_state.map);
    }
 
}

void LevelC::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    Utility::draw_text(program, C_font_texture_id, "WHAT, YOU'RE JUST GOING TO", 0.5f, 0.01f, glm::vec3(-2.0f, 3.0f , 0.0f));
    Utility::draw_text(program, C_font_texture_id, "LET THEM KILL YOU?", 0.5f, 0.01f, glm::vec3(0.25f, 2.0f , 0.0f));
    m_game_state.player->render(program);
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->render(program);
    }
    
}

