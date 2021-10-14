#ifndef SNAKE_SNAKE_H
#define SNAKE_SNAKE_H

#include <assert.h>
#include <tracy/Tracy.hpp>

#define YPL_TYPES_BY_TYPEDEF
#define YPL_TYPES_USING_EXACT
#include "ypl_types.h"
#include "math.h"

#define For(x) for(int it = 0; it < x; it++)
#define ForFrom(x, n) for(int it = n; it < x; it++)

//
// --- Constants ---
//
const float SQUARE_VERTICES[] = {
    // Top-left triangle
    -0.5f,  0.5f,
     0.5f,  0.5f,
    -0.5f, -0.5f,
    // Bottom-right triangle
     0.5f,  0.5f,
     0.5f, -0.5f,
    -0.5f, -0.5f
};

const int PLAYABLE_AREA_HEIGHT = 4; // [x, -x] (relative to origin cell)
const int PLAYABLE_AREA_LENGTH = 7; // [x, -x] (same)

// PLAYER_TAIL_LENGTH_MAX is multiplied by 4 because it counts only 'plane' in XY coords which is [+, +] and gives us 1/4 of a needed result.
// Because we know bounds are the same in all planes |x, x| we can just multiply by 4 to get the rest of 3/4 of it.
//
// Visual example:
// (signs: [x, y])
//
// Y-axis ^
// [-, +] |  [+, +]
//  -------------> X-axis
// [-, -] |  [+, -]
//        |
//
// But then we add some magically formuled number to compensate for
// calculations we did relative to origin cell. Because of that, center
// cell would have been sligtly to bottom-right, and the center would have
// been between 4 squares.
const int PLAYER_TAIL_LENGTH_MAX = PLAYABLE_AREA_HEIGHT * PLAYABLE_AREA_LENGTH * 4 + (PLAYABLE_AREA_HEIGHT*2 + PLAYABLE_AREA_LENGTH*2) + 1;

const float TILE_SCALING_FACTOR = 0.8f;

const int HOT_MEMORY_ARENA_CAPACITY = 64 * 1024 * sizeof(u8); // 64KB
const int COLD_MEMORY_ARENA_CAPACITY = 256 * 1024 * sizeof(u8); // 256KB

//
// --- Structs ---
//
struct Camera;
struct Cursor;
struct Frametime;
struct Screen;
struct Tail;
struct Player;
struct Resource;
struct Stats;
struct Memory_Arena;
enum Imgui_State;
enum Game_State;

struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float pitch = 0.0f;
    float yaw = -90.0f;
    float fov = 45.0f;
};

struct Cursor {
    float x = 0;
    float y = 0;
    float sensitivity = 0.1f;
    bool cursor_mode = true;
};

struct Frametime {
    float last = 0.0f;
    float current = 0.0f;
    float delta = 0.0f;
};

struct Screen {
    int width = 1280;
    int height = 720;
    float aspect_ratio = (float)width / (float)height;
    Vec4f clear_color = new_vec4f(0.1f, 0.1f, 0.1f, 1.0f); // RGBA
    bool resized = false;
    bool fullscreen = false;
};

struct Tail {
    float x = 0.0f;
    float y = 0.0f;
    float prev_x = 0.0f;
    float prev_y = 0.0f;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(0.96f, 0.39f, 0.26f);
};

struct Player {
    float x = 0.0f;
    float y = 0.0f;
    float prev_x = 0.0f;
    float prev_y = 0.0f;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(0.82f, 0.62f, 0.32f); // RGB
    glm::vec3 last_tail_color = glm::vec3(1.0f, 0.0f, 0.0f); // RGB
    int tail_length = 0;
    Tail *tails;
};

struct Resource {
    float x = 0.0f;
    float y = 0.0f;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(0.16f, 1.0f, 0.16f); // RGB
};

enum Imgui_State {
    DRAW_EDIT_WINDOW = 0,
    DRAW_DEMO_WINDOW = 1,
    DRAW_CONSTANTS_WINDOW = 2,
    DRAW_GLOBALS_WINDOW = 3,
    MOVE_PLAYER_BUTTON_PRESSED = 4,
    MOVE_RESOURCE_BUTTON_PRESSED = 5,
};

struct Stats {
    float start_time = 0.0f;
    float current_time = 0.0f;
    int score = 0;
    int moves = 0;
};

enum Game_State {
    PLAY = (1 << 0),
    DEBUG = (1 << 1),
    TITLE_SCREEN = (1 << 2),
    PAUSE_SCREEN = (1 << 3),
    SETTINGS_SCREEN = (1 << 4),
};

struct Memory_Arena {
    u8 *data;
    u64 capacity;
    u64 size;
};

template <typename T>
struct Array {
    T *data;
    u32 size;
    u32 capacity;
};

//
// --- Functions ---
//
void game_tick();
void game_reset();
void game_over();
void game_save();
void save_session();
void game_exit();
void store_stats(Stats *stats);
void move_player(Player *player, int squares_right, int squares_up);
void move_resource_from_origin(Resource *resource, int squares_right, int squares_up);
void move_resource_to_rand_pos(Resource *resource, Player *player);
void move_tail(Tail *tail, int squares_right, int squares_up);
void move_player_tails(Player *player);
Vec2f new_random_pos(int squares_up_max, int squares_right_max);
Vec2f new_random_resource_pos(Resource *resoucre, int max_tile_x, int max_tile_y, Vec2f *not_valid_tiles, int tiles_size);
inline Vec2f get_tile_coords(int x, int y);
inline Vec2f get_tile_coords(Vec2i tile);
inline Vec2i get_coords_tile(float x, float y);
inline Vec2i get_coords_tile(Vec2f coords);
void make_tails_color_step_gradient(Tail *tails, int length, glm::vec3 start_color, float step_r, float step_g, float step_b);
void make_tails_color_linear_gradient(Tail *tails, int length, glm::vec3 start_color, glm::vec3 end_color, float offset_from_middle);
void make_imgui_layout();
void draw_all_tails_on_screen();
Memory_Arena alloc_memory_arena(u64 capacity);

template <typename T>
void array_add(Array<T> *array, T item);

#endif /*SNAKE_SNAKE_H*/
