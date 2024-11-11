#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 84.0/255.0,
                BG_BLUE    = 118.0/255.0,
                BG_GREEN   = 176.0/255.0,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

float left_edge;
float bottom_edge;
float view_left;
float view_bottom;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
LevelA *g_levelA;

//Effects *g_effects;
Scene   *g_levels[1];

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float delta_time;
float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(&g_shader_program);
}

void soft_restart()
{
    g_view_matrix = glm::mat4(1.0f);
//    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    left_edge = 10.0f;
    bottom_edge = -20.0f;
    
    g_projection_matrix = glm::ortho(-10.0f, 10.0f, -7.5f, 7.5f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_levelA = new LevelA();
    
    g_levels[0] = g_levelA;
    
    // Start at level A
    switch_to_scene(g_levels[0]);
    
//    g_effects = new Effects(g_projection_matrix, g_view_matrix);
//    g_effects->start(SHRINK, 2.0f);
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Munar Mander",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    soft_restart();
}

void process_input() {
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    g_current_scene->get_state().player->face_forward();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                    
                    case SDLK_r: // restart the game
                        for (int i = 0; i < 1; i++) {
                            g_levels[i]->set_scene_id(-1);  // reset transitions
                        }
                        initialise(); // reinit
                        break;
                        
                    case SDLK_f:
                        break;
                        
                    case SDLK_RETURN:
                        break;
                        
                    default:
                        break;
                    
                    case SDLK_SPACE:
                        if (g_current_scene->get_state().player->get_collided_bottom()) {
                            g_current_scene->get_state().player->jump();
                        }
                }
                
            default:
                g_current_scene->get_state().player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        g_current_scene->get_state().player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])  {
        g_current_scene->get_state().player->move_right();
    }
         
    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
    
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
//        g_effects->update(FIXED_TIMESTEP);
        
//        if (g_is_colliding_bottom == false && g_current_scene->get_state().player->get_collided_bottom()) g_effects->start(SHAKE, 1.0f);
        
        g_is_colliding_bottom = g_current_scene->get_state().player->get_collided_bottom();
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    
    view_left = std::max(g_current_scene->get_state().player->get_position().x, left_edge);
    view_bottom = std::max(g_current_scene->get_state().player->get_position().y, bottom_edge);
    
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-view_left, -view_bottom, 0));
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glClear(GL_COLOR_BUFFER_BIT);
       
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
       
//    g_effects->render();
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_levelA;
//    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0) {
            int curr_fuel = g_current_scene->fuel_count;
            switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
            g_current_scene->fuel_count = curr_fuel;
            g_projection_matrix = glm::ortho(-30.0f, 30.0f, -22.5f, 22.5f, -1.0f, 1.0f);
            g_shader_program.set_projection_matrix(g_projection_matrix);
        }
        
        render();
    }
    
    shutdown();
    return 0;
}
