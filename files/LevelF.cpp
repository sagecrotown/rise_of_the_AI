#include "LevelF.h"
#include "Utility.h"

#define LEVEL_WIDTH 60
#define LEVEL_HEIGHT 45

constexpr char SPRITESHEET_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/sprites.png",
           ENEMY_FILEPATH[]       = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/soph.png",
            MAP_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/mars_map.png",
            FONT_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/mars_font.png",
            CSV_FILEPATH[] = "/Users/Sage/Downloads/Game Programming/rise_of_the_AI/files/assets/winner.csv";

unsigned int LEVELF_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

static std::vector<std::string> MESSAGE = {"AFTER RETURNING TO YOUR DORM, RAYTHEONSPACEXDRPEPPER ", "CONGRATULATES YOU WITH A SLIDESHOW OF YOUR 'BEST MOMENTS' ", "AND GIVES EVERYONE ICE CREAM AND HALF A DAY OFF. YOU ARE ", "TOLD YOU HAVE A WHOLE WEEK TO YOURSELF BEFORE YOUR NEXT ", "MISSION, THOUGH YOU'RE NOT QUITE SURE WHAT TO DO WITH ", "THE TIME. AFTER COMPLETING YOUR ACTUAL MISSION, YOU WERE ", "INSTRUCTED TO GATHER SEVERAL MORE MARTIAN ROCKS, WHICH ", "YOU WILL DISTRIBUTE TO THE OTHER KIDS WHO ARE RECOVERING ", "FROM THEIR IMPLANTS. YOU TELL THEM IT'S NOT SO BAD, AND ", "YOU TRULY BELIEVE IT. YOU ARE HAPPY, YOU THINK. AFTER ", "ALL, ISN'T THIS WHAT LIFE IS ALL ABOUT?"};

LevelF::~LevelF()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelF::initialise(ShaderProgram *program)
{
    m_game_state.next_scene_id = -1;
    
    F_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
    Utility::readCSV(CSV_FILEPATH, LEVELF_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELF_DATA, map_texture_id, 1.0f, 7, 3);
    
    std::vector<std::vector<int>> player_animation = {
        { 0 },   // dead
        { 1 },   // landed safely
        { 2 },   // playing
        { 3 },   // blasting
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        4,                         // animation row amount
        1.0f,                      // width
        1.0f,                       // height
        PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(30.0f, -40.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(20.0f, 20.0f, 1.0f));
    m_game_state.player->face_right();
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
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
    
    // Font stuff:
}

void LevelF::update(float delta_time) {
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
}

void LevelF::render(ShaderProgram *program) {
    m_game_state.map->render(program);
    Utility::draw_text(program, F_font_texture_id, "YOU WIN!", 2.0f, 0.01f, glm::vec3(20.0f, -3.5f, 0.0f));
    for (int i = 0; i < MESSAGE.size(); i++) {
        Utility::draw_text(program, F_font_texture_id, MESSAGE[i], 0.95f, 0.01f, glm::vec3(3.0f, -7.0f - (i * 2.0f), 0.0f));
    }
    m_game_state.player->render(program);
    
}
