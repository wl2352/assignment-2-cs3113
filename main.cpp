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
const char KITTY_FILEPATH[] = "assets/blue-metallic.jpg";
const char WALL_FILEPATH[] = "assets/glossy.jpg";
const char BALL_FILEPATH[] = "assets/Red_Circle_full.png";

const float MINIMUM_COLLISION_DISTANCE = 1.0f;
const float collision_factor = 0.9f;

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

float previous_ticks = 0.0f;

bool is_cpu = false;
int ball_collision_bool = 0;
int wall_collision_bool = 0;

glm::vec3 g_player_position = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 bot_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 other_player_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 other_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 other_player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 other_player_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_scale = glm::vec3(0.2f, 2.0f, 0.0f);
glm::vec3 horizontal_scale = glm::vec3(10.0f, 0.1f, 0.0f);
glm::vec3 vertical_scale = glm::vec3(0.1f, 8.0f, 0.0f);
glm::vec2 angles = glm::vec2(1.0f, 1.0f);

glm::vec3 top_wall_position = glm::vec3(0.0f, 3.5f, 0.0f);
glm::vec3 bottom_wall_position = glm::vec3(0.0f, -3.5f, 0.0f);
glm::vec3 left_wall_position = glm::vec3(-5.0f, 0.0f, 0.0f);
glm::vec3 right_wall_position = glm::vec3(5.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_scale = glm::vec3(0.2f, 0.2f, 0.0f);

float g_player_speed = 5.0f;  // move 1 unit per second 
float other_player_speed = 5.0f;
float ball_speed = 4.5f;

GLuint player_texture_id;
GLuint other_texture_id;
GLuint ball_texture_id;
GLuint wall_texture_id;

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

    left_wall_matrix = glm::mat4(1.0f);
    left_wall_matrix = glm::translate(left_wall_matrix, left_wall_position);
    left_wall_matrix = glm::scale(left_wall_matrix, vertical_scale);

    right_wall_matrix = glm::mat4(1.0f);
    right_wall_matrix = glm::translate(right_wall_matrix, right_wall_position);
    right_wall_matrix = glm::scale(right_wall_matrix, vertical_scale);

    top_wall_matrix = glm::mat4(1.0f);
    top_wall_matrix = glm::translate(top_wall_matrix, top_wall_position);
    top_wall_matrix = glm::scale(top_wall_matrix, horizontal_scale);

    bottom_wall_matrix = glm::mat4(1.0f);
    bottom_wall_matrix = glm::translate(bottom_wall_matrix, bottom_wall_position);
    bottom_wall_matrix = glm::scale(bottom_wall_matrix, horizontal_scale);

    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(g_model_matrix, g_player_position);
    g_model_matrix = glm::scale(g_model_matrix, player_scale);//
    // –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //

    other_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::translate(other_model_matrix, other_player_position);
    other_model_matrix = glm::scale(other_model_matrix, player_scale);
    // –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //

    ball_matrix = glm::translate(ball_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
    ball_matrix = glm::scale(ball_matrix, ball_scale);

    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    //glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    glClearColor(0.55f, 0.234, 0.443f, 0.0f);

    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(KITTY_FILEPATH);
    ball_texture_id = load_texture(BALL_FILEPATH);
    wall_texture_id = load_texture(WALL_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ————————————————————————— NEW STUFF ———————————————————————————— //
bool check_collision(glm::vec3& position_a, glm::vec3& position_a_scale, glm::vec3& position_b, glm::vec3& position_b_scale)  //
{                                                                   //
    float x_distance = fabs(position_a.x - position_b.x) - ((position_b_scale.x * collision_factor + position_a_scale.x * collision_factor) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((position_b_scale.y * collision_factor + position_a_scale.y * collision_factor) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}                                                                   //
// ———————————————————————————————————————————————————————————————— //

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
                break;
            case SDLK_t:
                is_cpu = true;
                g_player_speed = 4.0f;
                break;
            }
            //
        default:                                                         //
            break;                                                       //
        }                                                                    //
    }                                                                        //
                                                                             //
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
                                                                             //
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (is_cpu)
    {
        if (wall_collision_bool % 2 == 0) {
            g_player_movement.y = 1.0f;
        }
        else {
            g_player_movement.y = -1.0f;
        }
    }

    else
    {
        if (key_state[SDL_SCANCODE_W])                                           //
        {                                                                        //
            g_player_movement.y = 1.0f;                                          //
        }                                                                        //
        else if (key_state[SDL_SCANCODE_S])                                      //
        {                                                                        //
            g_player_movement.y = -1.0f;                                         //
        }
    }

    if (key_state[SDL_SCANCODE_UP])                                          //
    {                                                                        //
        other_player_movement.y = 1.0f;                                      //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_DOWN])                                   //
    {                                                                        //
        other_player_movement.y = -1.0f;                                     //
    }

    // Collision checking should be done here to avoid player input
    if (check_collision(g_player_position, player_scale, top_wall_position, horizontal_scale) ||
        check_collision(g_player_position, player_scale, bottom_wall_position, horizontal_scale))
    {
        if (is_cpu) {
            if (check_collision(g_player_position, player_scale, top_wall_position, horizontal_scale)) {
                wall_collision_bool = 1;
            }
            else {
                wall_collision_bool = 0;
            }

        }
        else {
            g_player_movement = glm::vec3(0.0f);
            if (check_collision(g_player_position, player_scale, top_wall_position, horizontal_scale)) {
                g_player_movement.y = -1.0f;
            }
            else {
                g_player_movement.y = 1.0f;
            }

        }
    }
    if (check_collision(other_player_position, player_scale, top_wall_position, horizontal_scale) ||
        check_collision(other_player_position, player_scale, bottom_wall_position, horizontal_scale))
    {
        other_player_movement = glm::vec3(0.0f);
        if (check_collision(other_player_position, player_scale, top_wall_position, horizontal_scale)) {
            other_player_movement.y = -1.0f;
        }
        else {
            other_player_movement.y = 1.0f;
        }
    }

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

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    // Add direction * units per second * elapsed time                      //
    g_player_position += g_player_movement * g_player_speed * delta_time;   //
    other_player_position += other_player_movement * other_player_speed * delta_time;
    ball_position += ball_movement * ball_speed * delta_time;

    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(g_model_matrix, g_player_position);
    g_model_matrix = glm::scale(g_model_matrix, player_scale);//

    other_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::translate(other_model_matrix, other_player_position);
    other_model_matrix = glm::scale(other_model_matrix, player_scale);

    ball_matrix = glm::mat4(1.0f);
    ball_matrix = glm::translate(ball_matrix, ball_position);
    ball_matrix = glm::scale(ball_matrix, ball_scale);
    // –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //

    /*
    left paddle --> x flips,  y stays the same
    right paddle --> x flips, y stays the same for every hit

    angle.x *= -1

    topwall/bottomwall --> angle.y *= -1
    */

    // Check ball-object collisions
    if (check_collision(g_player_position, player_scale, ball_position, ball_scale) ||
        check_collision(other_player_position, player_scale, ball_position, ball_scale))
    {
        angles.x *= -1.0f;
    }

    if (check_collision(ball_position, ball_scale, top_wall_position, horizontal_scale) ||
        check_collision(ball_position, ball_scale, bottom_wall_position, horizontal_scale))
    {
        angles.y *= -1.0f;
    }

    // End game condition
    if (check_collision(ball_position, ball_scale, left_wall_position, vertical_scale) ||
        check_collision(ball_position, ball_scale, right_wall_position, vertical_scale))
    {
        g_game_is_running = false;
    }

    ball_movement.x = 0.65f * angles.x;
    ball_movement.y = 0.55f * angles.y;
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
    draw_object(left_wall_matrix, wall_texture_id);
    draw_object(right_wall_matrix, wall_texture_id);
    draw_object(top_wall_matrix, wall_texture_id);
    draw_object(bottom_wall_matrix, wall_texture_id);

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
