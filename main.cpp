/**
* Author: William Liburd
* Assignment: Ping Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

#include <fstream>

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

// Our object's fill colour
const float TRIANGLE_RED = 1.0,
TRIANGLE_BLUE = 0.4,
TRIANGLE_GREEN = 0.4,
TRIANGLE_OPACITY = 1.0;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const char PLAYER_SPRITE_FILEPATH[] = "assets/aware.jpeg";
const char KITTY_FILEPATH[] = "assets/angrykitty.jpeg";
const char BALL_FILEPATH[] = "assets/glossy.jpg";

const float MINIMUM_COLLISION_DISTANCE = 1.0f;
const float collision_factor = 0.09f;

SDL_Window* display_window;
bool g_game_is_running = true;
bool is_growing = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, 
g_model_matrix, 
g_projection_matrix, 
other_model_matrix, 
left_wall_matrix, 
right_wall_matrix, 
top_wall_matrix, 
bottom_wall_matrix, 
ball_matrix;

float g_triangle_x = 0.0f;
float g_triangle_y = 0.0f;
float g_triangle_rotate = 0.0f;

const float RADIUS = 2.5f;
const float ROT_SPEED = 0.01f;
float o_angle = 0.0f;
float o_x_coords = RADIUS;
float o_y_coords = 0.0f;

float previous_ticks = 0.0f;

bool is_going_left = true;
bool is_going_right = false;
int ball_collision_count = 0;

glm::vec3 g_player_position = glm::vec3(-8.0f, 0.0f, 0.0f); 
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);  

glm::vec3 g_player_orientation = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 g_player_rotation = glm::vec3(0.0f, 0.0f, 0.0f); 

glm::vec3 other_player_position = glm::vec3(8.0f, 0.0f, 0.0f);
glm::vec3 other_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 other_player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 other_player_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_scale = glm::vec3(0.5f, 2.0f, 0.0f);

glm::vec3 top_wall_position = glm::vec3(0.0f, 6.0f, 0.0f);
glm::vec3 bottom_wall_position = glm::vec3(0.0f, -6.0f, 0.0f);
glm::vec3 left_wall_position = glm::vec3(-10.0f, 0.0f, 0.0f);
glm::vec3 right_wall_position = glm::vec3(10.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_scale = glm::vec3(0.001f, 0.001f, 0.0f);

float g_player_speed = 5.0f;  // move 1 unit per second 
float other_player_speed = 5.0f;
float ball_speed = 14.0f;

GLuint player_texture_id;
GLuint other_texture_id;
GLuint ball_texture_id;

#define LOG(argument) std::cout << argument << '\n'

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Assignment 2 Liburd",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::mat4(1.0f);
    ball_matrix = glm::mat4(1.0f);
    left_wall_matrix = glm::mat4(1.0f);
    right_wall_matrix = glm::mat4(1.0f);
    top_wall_matrix = glm::mat4(1.0f);
    bottom_wall_matrix = glm::mat4(1.0f);

    ball_matrix = glm::scale(ball_matrix, ball_scale);
    ball_matrix = glm::translate(ball_matrix, glm::vec3(0.0f, 0.0f, 0.0f));

    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(KITTY_FILEPATH);
    ball_texture_id = load_texture(BALL_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // –––––––––––––––––––––––––––––––– NEW STUFF –––––––––––––––––––––––––– //
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere   //
    g_player_movement = glm::vec3(0.0f);
    other_player_movement = glm::vec3(0.0f);
    //
    // –––––––––––––––––––––––––––––––– KEYSTROKES ––––––––––––––––––––––––– //
                                                                         //
    SDL_Event event;                                                         //
    while (SDL_PollEvent(&event))                                            //
    {                                                                        //
        switch (event.type)                                                  //
        {                                                                    //
            // End game                                                      //
        case SDL_QUIT:                                                   //
        case SDL_WINDOWEVENT_CLOSE:                                      //
            g_game_is_running = false;                                   //
            break;                                                       //
            //
        case SDL_KEYDOWN:                                                //
            switch (event.key.keysym.sym)                                //
            {                                                            //
            case SDLK_q:                                             //
                // Quit the game with a keystroke                    //
                g_game_is_running = false;                           //
                break;                                               //
                //q
                break;                                               //
            }                                                            //
                                                                         //
        default:                                                         //
            break;                                                       //
        }                                                                    //
    }                                                                        //
                                                                             //
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
                                                                             //
    const Uint8* key_state = SDL_GetKeyboardState(NULL);                     //
    if (key_state[SDL_SCANCODE_W])                                           //
    {                                                                        //
        g_player_movement.y = 1.0f;                                          //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_S])                                      //
    {                                                                        //
        g_player_movement.y = -1.0f;                                         //
    }
    
    if (key_state[SDL_SCANCODE_UP])                                          //
    {                                                                        //
        other_player_movement.y = 1.0f;                                      //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_DOWN])                                   //
    {                                                                        //
        other_player_movement.y = -1.0f;                                     //
    }
                                                                             //
    // This makes sure that the player can't "cheat" their way into moving   //
    // faster                                                                //
    if (glm::length(g_player_movement) > 1.0f)                               //
    {                                                                        //
        g_player_movement = glm::normalize(g_player_movement);               //
    }                                                                        
    if (glm::length(other_player_movement) > 1.0f)                           //
    {                                                                        //
        other_player_movement = glm::normalize(other_player_movement);
    }
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
}

// ————————————————————————— NEW STUFF ———————————————————————————— //
bool check_player_ball_collision(glm::vec3& position_a)  //
{                                                                   //
    float x_distance = fabs(position_a.x - ball_position.x) - ((ball_scale.x * collision_factor + player_scale.x * collision_factor) / 2.0f);
    float y_distance = fabs(position_a.y - ball_position.y) - ((ball_scale.y * collision_factor + player_scale.y * collision_factor) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}                                                                   //
// ———————————————————————————————————————————————————————————————— //

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    left_wall_matrix = glm::mat4(1.0f);
    left_wall_matrix = glm::scale(left_wall_matrix, glm::vec3(1.0f, 7.5f, 0.0f));
    left_wall_matrix = glm::translate(left_wall_matrix, glm::vec3(-5.0f, 0.0f, 0.0f));

    right_wall_matrix = glm::mat4(1.0f);
    right_wall_matrix = glm::scale(right_wall_matrix, glm::vec3(1.0f, 7.5f, 0.0f));
    right_wall_matrix = glm::translate(right_wall_matrix, glm::vec3(5.0f, 0.0f, 0.0f));

    top_wall_matrix = glm::mat4(1.0f);
    top_wall_matrix = glm::scale(top_wall_matrix, glm::vec3(9.5f, 1.0f, 0.0f));
    top_wall_matrix = glm::translate(top_wall_matrix, glm::vec3(0.0f, 4.0f, 0.0f));

    bottom_wall_matrix = glm::mat4(1.0f);
    bottom_wall_matrix = glm::scale(bottom_wall_matrix, glm::vec3(9.5f, 1.0f, 0.0f));
    bottom_wall_matrix = glm::translate(bottom_wall_matrix, glm::vec3(0.0f, -4.0f, 0.0f));


    // –––––––––––––––––––––––––––––––– NEW STUFF ––––––––––––––––––––––––– //
    // Add direction * units per second * elapsed time                      //
    g_player_position += g_player_movement * g_player_speed * delta_time;   //
    other_player_position += other_player_movement * other_player_speed * delta_time;
    ball_position += ball_movement * ball_speed * delta_time;
    //
    
    
    // Check player-wall collision
    //if (check_collision(g_player_position, top_wall_position) || check_collision(other_player_position, top_wall_position) ||
    //    check_collision(g_player_position, bottom_wall_position) || check_collision(other_player_position, bottom_wall_position)) {
    //    // DO SOMETHING TO STOP COLLISION
    //}

    //// Check ball-wall collision
    //if (check_collision(ball_position, left_wall_position)) {
    //    // other_player wins
    //    // display winner message
    //    // end game
    //}
    //if (check_collision(ball_position, right_wall_position)) {
    //    // g_player wins
    //    // display winner message
    //    // end game
    //}

    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::scale(g_model_matrix, glm::vec3(0.5f, 2.0f, 0.0f));//
    g_model_matrix = glm::translate(g_model_matrix, g_player_position);     //
    // –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //

    other_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::scale(other_model_matrix, glm::vec3(0.5f, 2.0f, 0.0f));//
    other_model_matrix = glm::translate(other_model_matrix, other_player_position);     //
    // –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //

    ball_matrix = glm::mat4(1.0f);
    ball_matrix = glm::scale(ball_matrix, glm::vec3(0.2f, 0.2f, 0.0f));
    ball_matrix = glm::translate(ball_matrix, ball_position);

    // Check ball collision
    if (check_player_ball_collision(g_player_position) || check_player_ball_collision(other_player_position)) {
        ball_collision_count += 1;
        std::cout << std::time(nullptr) << ": Collision.\n";
    }

    if (ball_collision_count % 2 == 1) {
        ball_movement.x = 1.0f;
        ball_movement.y = 0.5f;
    }
    else {
        ball_movement.x = -1.0f;
        ball_movement.y = -0.5f;
    }
    if (glm::length(g_player_movement) > 1.0f) {
        ball_movement = glm::normalize(ball_movement);
    }

}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());

    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_model_matrix, player_texture_id);
    draw_object(other_model_matrix, other_texture_id);
    draw_object(ball_matrix, ball_texture_id);
    draw_object(left_wall_matrix, ball_texture_id);
    draw_object(right_wall_matrix, ball_texture_id);
    draw_object(top_wall_matrix, ball_texture_id);
    draw_object(bottom_wall_matrix, ball_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_program.get_position_attribute());
    glDisableVertexAttribArray(g_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
