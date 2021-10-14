// #include <imgui/imgui.h>
// #include <imgui/imgui_impl_glfw.h>
// #include <imgui/imgui_impl_opengl3.h>

// time()
#include <time.h>

// glm::perspective, glm::lookAt
#include <GLM/gtc/matrix_transform.hpp>

// 'snake.h' is already included in 'renderer.h'
#include "renderer.h"

//
// --- Global variables ---
//

// Camera camera;

extern Player player;
extern Resource resource;
extern u32 game_state;
extern bool imgui_states[] = { true, false, false, false, false, false };

// Globals from renderer.cpp
Screen screen;
Cursor cursor;
Frametime frametime;
GLFWwindow *window;
Renderer_Info renderer_info;

static Stats stats;
static Vec2i player_move;
static Vec2i resource_move;

static Tail tails[PLAYER_TAIL_LENGTH_MAX];

//
// --- ImGui input ---
//
float *imgui_playerdrag[4] = { &player.x, &player.y, &player.prev_x, &player.prev_y };
float *imgui_resourcedrag[2] = { &resource.x, &resource.y };
int imgui_tail_num = 0;
float *imgui_taildrag[4] = { &tails[imgui_tail_num].x, &tails[imgui_tail_num].y, &tails[imgui_tail_num].prev_x, &tails[imgui_tail_num].prev_y };
int imgui_swap_interval = 1;

int main(int arguments_count, char **arguments) {
    ZoneScoped;
    
    srand(time(NULL)); // Init random number generator seed

    init_renderer();

    // Init defaults.
    // player.model = glm::scale(player.model, glm::vec3(TILE_SCALING_FACTOR));
    // resource.model = glm::scale(resource.model, glm::vec3(TILE_SCALING_FACTOR));
    player.tails = tails;
    player.model = glm::scale(player.model, glm::vec3(100.0f));
    resource.model = player.model;

    For (PLAYER_TAIL_LENGTH_MAX) {
        tails[it].model = player.model;
    }

    game_state = TITLE_SCREEN;
    // draw_all_tails_on_screen();

    while (!glfwWindowShouldClose(window)) {
        // !!!
        // We already do this in 'game_tick()'
        // !!!
        //
        //
        //
        make_tails_color_linear_gradient(tails, PLAYER_TAIL_LENGTH_MAX, player.color, player.last_tail_color, 0.0f);

        // process_input(window);

        // Move player or resource when "Move" button pressed.
        if (imgui_states[MOVE_PLAYER_BUTTON_PRESSED]) {
            move_player(&player, player_move.x, player_move.y);
        } else if (imgui_states[MOVE_RESOURCE_BUTTON_PRESSED]) {
            move_resource_from_origin(&resource, resource_move.x, resource_move.y);
        }

        renderer_draw(game_state);
    }

    game_exit();

    exit(EXIT_SUCCESS);
}

void game_tick() {
    ZoneScoped;
    
    bool player_on_resource_tile = floats_equal(player.x, resource.x) && floats_equal(player.y, resource.y);

    bool player_on_tail_tile = false;
    For (player.tail_length) {
        if (player_on_tail_tile) break;
        player_on_tail_tile = floats_equal(player.x, player.tails[it].x) && floats_equal(player.y, player.tails[it].y);
    }

    if (player_on_tail_tile) {
        game_over();
        game_reset();
    }

    if (player_on_resource_tile) {
        printf("[%.2f] - Player stepped on resource tile at [%.1f, %.1f]\n", frametime.current, player.x, player.y); 
        move_resource_to_rand_pos(&resource, &player);
        stats.score++;
        player.tail_length++;
    }
    move_player_tails(&player);
    make_tails_color_linear_gradient(tails, PLAYER_TAIL_LENGTH_MAX, player.color, player.last_tail_color, 0.0f);
}

void game_reset() {
    ZoneScoped;
    
    // Reset statistics.
    stats.start_time = frametime.current;
    stats.current_time = 0.0f;
    stats.moves = 0;
    stats.score = 0;

    float dx, dy;
    // Reset tails.
    For (player.tail_length) {
        dx = -tails[it].x;
        dy = -tails[it].y;
        tails[it].x = 0.0f;
        tails[it].y = 0.0f;
        tails[it].prev_x = 0.0f;
        tails[it].prev_y = 0.0f;
        tails[it].model = glm::translate(tails[it].model, glm::vec3(dx, dy, 0.0f));
    }

    // Reset player.
    player.tail_length = 0;
    dx = -player.x;
    dy = -player.y;
    player.x = 0.0f;
    player.y = 0.0f;
    player.prev_x = 0.0f;
    player.prev_y = 0.0f;
    player.model = glm::translate(player.model, glm::vec3(dx, dy, 0.0f));
    printf("[%.2f] - Game has been reseted.\n", frametime.current);

    // Reset resource. (move to new random tile)
    move_resource_to_rand_pos(&resource, &player);
}

void game_over() {
    ZoneScoped;
    
    stats.current_time = frametime.current;
    printf("[%.2f] - Game over! End result - Score: %d, Time: %.3f, Moves: %d\n", frametime.current, stats.score, stats.current_time - stats.start_time, stats.moves);
}

void game_save() {
    ZoneScoped;
    
    printf("[UNIMPLEMENTED] [%.2f] - Saving game session...\n", frametime.current);
}

void game_exit() {
    ZoneScoped;
    
    printf("[%.2f] - Exiting from game...\n", frametime.current);
    if (game_state & PLAY) {
        game_save();
    }
    renderer_free_resources();
    exit(EXIT_SUCCESS);
}

void store_stats(Stats *stats) {
    ZoneScoped;
    
    printf("[UNIMPLEMENTED] [%.2f] - Saving statistics...\n", frametime.current);
}

// squares_right - how many square spaces move right (if positive)
// or left (if negative)
// squrers_up    - how many square spaces move up (if positive)
// or down (if negative)
void move_player(Player *player, int squares_right, int squares_up) {
    ZoneScoped;
    
    // Actually it's '1 * squares_right', because square length is 1.
    // But it's unnecessary because '1 * squares_right' == 'squres_right'.
    float dx = 1.2f * squares_right;
    float dy = 1.2f * squares_up;
    player->prev_x = player->x;
    player->prev_y = player->y;
    player->x += dx;
    player->y += dy;
    player->model = glm::translate(player->model, glm::vec3(dx, dy, 0));

    stats.moves++;
    game_tick();
}

void move_resource_from_origin(Resource *resource, int squares_right, int squares_up) {
    ZoneScoped;
    
    // Move it to origin first, which is [0.0, 0.0].
    float dx = -resource->x;
    float dy = -resource->y;
    resource->x = 0.0f;
    resource->y = 0.0f;
    resource->model = glm::translate(resource->model, glm::vec3(dx, dy, 0));

    // And then translate to a new position.
    dx = 1.2f * squares_right;
    dy = 1.2f * squares_up;
    resource->x += dx;
    resource->y += dy;
    resource->model = glm::translate(resource->model, glm::vec3(dx, dy, 0));

    printf("[%.2f] - Resource moved to [%.1f, %.1f]\n", frametime.current, resource->x, resource->y);
}


void move_resource_to_rand_pos(Resource *resource, Player *player) {
    ZoneScoped;
    
    Vec2f new_pos;
    Vec2f check_pos;
    {
    rand:
        new_pos = new_random_pos(PLAYABLE_AREA_HEIGHT, PLAYABLE_AREA_LENGTH);
        // Make sure we don't move to the same position.
        check_pos.x = resource->x;
        check_pos.y = resource->y;
        bool on_not_valid_tile = vec2f_equal(new_pos, check_pos);
        if (on_not_valid_tile) {
            printf("[%.2f] - New resource position is on old resource's position! New iteration...\n", frametime.current);
            goto rand;
        }

        // We don't need to check the player's tile
        // because its position always should be the same as the resource's one.

        // Since we first calculate new resource position and only then
        // move the player's tail, we need to check previous player position
        // because the tail didn't go forward yet. 
        check_pos.x = player->prev_x;
        check_pos.y = player->prev_y;
        on_not_valid_tile = vec2f_equal(new_pos, check_pos);
        if (on_not_valid_tile) {
            printf("[%.2f] - New resource position is on player's previous tile! New iteration...\n", frametime.current);
            goto rand;
        }

        For (player->tail_length) {
            check_pos.x = player->tails[it].x;
            check_pos.y = player->tails[it].y;
            on_not_valid_tile = vec2f_equal(new_pos, check_pos);
            if (on_not_valid_tile) {
                printf("[%.2f] - New resource position is on player's %d-th tail tile! New iteration...\n", frametime.current, it+1);
                goto rand;
            }
        }
    }

    // Divide it by 1.2f so it is represented in squares.
    move_resource_from_origin(resource, new_pos.x / 1.2f, new_pos.y / 1.2f);
}

//@Copy of 'move_player'
void move_tail(Tail *tail, int squares_right, int squares_up) {
    ZoneScoped;
    
    float dx = 1.2f * squares_right;
    float dy = 1.2f * squares_up;
    tail->prev_x = tail->x;
    tail->prev_y = tail->y;
    tail->x += dx;
    tail->y += dy;
    tail->model = glm::translate(tail->model, glm::vec3(dx, dy, 0));
}

void move_player_tails(Player *player) {
    ZoneScoped;
    
    Tail *tails = player->tails;

    // Tail #0
    float tx = tails[0].x;
    float ty = tails[0].y;
    float dx = player->prev_x - tx;
    float dy = player->prev_y - ty;
    tails[0].prev_x = tx;
    tails[0].prev_y = ty;
    tails[0].x += dx;
    tails[0].y += dy;
    tails[0].model = glm::translate(tails[0].model, glm::vec3(dx, dy, 0.0f));

    // Tail #1..#tail_length-1
    for (int it = 1; it < player->tail_length; it++) {
        float tx = tails[it].x;
        float ty = tails[it].y;
        float dx = tails[it-1].prev_x - tx;
        float dy = tails[it-1].prev_y - ty;
        tails[it].prev_x = tx;
        tails[it].prev_y = ty;
        tails[it].x += dx;
        tails[it].y += dy;
        tails[it].model = glm::translate(tails[it].model, glm::vec3(dx, dy, 0.0f));
    }
}

Vec2f new_random_pos(int squares_up_max, int squares_right_max) {
    ZoneScoped;
    
    Vec2f result;
    // Values on Y axis: [-squares_up_max,    squares_up_max]
    // Values on X axis: [-squares_right_max, squares_right_max]

    // First, we generate number in interval [x, 2*x]
    // and then substract 'x', so interval would be [-x, x]
    int temprand = rand() % ((2*squares_right_max)+1);
    result.x = (float) (temprand - squares_right_max) * 1.2f;
    temprand = rand() % ((2*squares_up_max)+1);
    result.y = (float) (temprand - squares_up_max) * 1.2f;
    return result;
};

Vec2f new_random_resource_pos(Resource *resource, int max_tile_x, int max_tile_y, Vec2f *not_valid_tiles, int tiles_size) {
    ZoneScoped;
    
    Vec2f new_pos;
    Vec2f check_pos;
    rand:
    new_pos = new_random_pos(max_tile_x, max_tile_y);
    check_pos.x = resource->x;
    check_pos.y = resource->y;
    bool on_not_valid_tile = vec2f_equal(new_pos, check_pos);
    if (on_not_valid_tile) {
        printf("[%.2f] - New resource position is on old resource's position! New iteration...\n", frametime.current);
        goto rand;
    }

    For (tiles_size) {
        on_not_valid_tile = vec2f_equal(new_pos, not_valid_tiles[it]);
        if (on_not_valid_tile) {
            printf("[%.2f] - New resource position is invalid! New iteration...\n", frametime.current);
        }
    }
    return new_pos;
}

/*inline*/
Vec2f get_tile_coords(int x, int y) {
    Vec2f result;
    result.x = (float)x * (1.0f / TILE_SCALING_FACTOR);
    result.y = (float)y * (1.0f / TILE_SCALING_FACTOR);
    return result;
}

/*inline*/
Vec2f get_tile_coords(Vec2i tile) {
    Vec2f result;
    result.x = (float)tile.x * (1.0f / TILE_SCALING_FACTOR);
    result.y = (float)tile.y * (1.0f / TILE_SCALING_FACTOR);
    return result;
}

/*inline*/
Vec2i get_coords_tile(float x, float y) {
    Vec2i result;
    result.x = (int) (x / (1.0f / TILE_SCALING_FACTOR));
    result.y = (int) (y / (1.0f / TILE_SCALING_FACTOR));
    return result;
}

/*inline*/
Vec2i get_coords_tile(Vec2f coords) {
    Vec2i result;
    result.x = (int) (coords.x / (1.0f / TILE_SCALING_FACTOR));
    result.y = (int) (coords.y / (1.0f / TILE_SCALING_FACTOR));
    return result;
}

void make_tails_color_step_gradient(Tail *tails, int length, glm::vec3 start_color, float step_r, float step_g, float step_b) {
    ZoneScoped;
    
    glm::vec3 new_color = start_color;
    For (length) {
        new_color.r += step_r;
        new_color.g += step_g;
        new_color.b += step_b;
        tails[it].color = new_color;
    }
}

void make_tails_color_linear_gradient(Tail *tails, int length, glm::vec3 start_color, glm::vec3 end_color, float offset_from_middle) {
    ZoneScoped;
    
    glm::vec3 new_color = start_color;
    glm::vec3 steps;
    steps.r = (end_color.r - start_color.r) / length;
    steps.g = (end_color.g - start_color.g) / length;
    steps.b = (end_color.b - start_color.b) / length;
    new_color.r += steps.r + offset_from_middle;
    new_color.g += steps.g + offset_from_middle;
    new_color.b += steps.b + offset_from_middle;
    tails[0].color = new_color;

    ForFrom (length, 1) {
        steps.r = (end_color.r - start_color.r) / length;
        steps.g = (end_color.g - start_color.g) / length;
        steps.b = (end_color.b - start_color.b) / length;
        new_color.r += steps.r;
        new_color.g += steps.g;
        new_color.b += steps.b;
        tails[it].color = new_color;
    }
}

void make_imgui_layout() {
    ZoneScoped;
    
    imgui_taildrag[0] = &tails[imgui_tail_num].x;
    imgui_taildrag[1] = &tails[imgui_tail_num].y;
    imgui_taildrag[2] = &tails[imgui_tail_num].prev_x;
    imgui_taildrag[3] = &tails[imgui_tail_num].prev_y;

    if (imgui_states[DRAW_DEMO_WINDOW]) {
        ImGui::ShowDemoWindow(&imgui_states[DRAW_DEMO_WINDOW]);
    }

    if (imgui_states[DRAW_CONSTANTS_WINDOW]) {
        ImGui::Begin("Constants");
        ImGui::Text("Playable area height: %d", PLAYABLE_AREA_HEIGHT);
        ImGui::Text("Playable area length: %d", PLAYABLE_AREA_LENGTH);
        ImGui::Text("Player tail length max: %d", PLAYER_TAIL_LENGTH_MAX);
        ImGui::Text("Floats compare precision: %g", COMPARE_FLOAT_PRECISION);
        ImGui::End();
    }

    if (imgui_states[DRAW_GLOBALS_WINDOW]) {
        // ...
    }

    if (imgui_states[DRAW_EDIT_WINDOW]) {
        ImGuiIO &imgui_io = ImGui::GetIO();

        ImGui::Begin("Scene");
        ImGui::Checkbox("Demo Window", &imgui_states[DRAW_DEMO_WINDOW]);
        ImGui::Checkbox("Constants Window", &imgui_states[DRAW_CONSTANTS_WINDOW]);
        ImGui::Checkbox("Globals Window", &imgui_states[DRAW_GLOBALS_WINDOW]);
        ImGui::InputInt("Swap interval", &imgui_swap_interval);
        ImGui::ColorEdit3("Clear color", &screen.clear_color.r);
        ImGui::DragInt2("Move Player", &player_move.x);
        ImGui::SameLine(); imgui_states[MOVE_PLAYER_BUTTON_PRESSED] = ImGui::Button("MoveP");
        ImGui::DragInt2("Move Resource", &resource_move.x);
        ImGui::SameLine(); imgui_states[MOVE_RESOURCE_BUTTON_PRESSED] = ImGui::Button("MoveR");
        ImGui::DragFloat4("Player Pos[1, 2] / Prev. pos[3, 4]", imgui_playerdrag[0], 1.0f, 0.0f, 0.0f, "%.1f");
        ImGui::ColorEdit3("Player Color", &player.color[0]);
        ImGui::ColorEdit3("Player Last Tail Color", &player.last_tail_color[0]);
        ImGui::DragFloat2("Resource", imgui_resourcedrag[0], 1.0f, 0.0f, 0.0f, "%.1f");
        ImGui::ColorEdit3("Resource Color", &resource.color[0]);
        ImGui::InputInt("Tail Number X (from 0 to 64)", &imgui_tail_num);
        ImGui::DragFloat4("Tail #X", imgui_taildrag[imgui_tail_num], 1.0f, 0.0f, 0.0f, "%.1f");
        ImGui::ColorEdit3("Tail Color", &tails[imgui_tail_num].color[0]);
        ImGui::NewLine();
        ImGui::Text("GPU Vendor: %s", renderer_info.gpu_vendor);
        ImGui::Text("GPU Renderer: %s", renderer_info.gpu_renderer);
        ImGui::Text("OpenGL/Driver version: %s", renderer_info.gl_version);
        ImGui::Text("GLSL: %s", renderer_info.glsl_version);
        ImGui::Text("ImGui Frametime: %.3f ms/frame (%.1f FPS)", 1000.0f / imgui_io.Framerate, imgui_io.Framerate);
        ImGui::Text("Frametime: %.3f ms/frame (%.1f FPS)", frametime.delta, 1000.0f / frametime.delta);
        ImGui::End();
    }
}

// @Hack @Debug
void draw_all_tails_on_screen() {
    ZoneScoped;
    
    player.tail_length = PLAYER_TAIL_LENGTH_MAX;
    int i = 0;
    for (int row = PLAYABLE_AREA_HEIGHT; row > -PLAYABLE_AREA_HEIGHT-1; row--) {
        for (int column = -PLAYABLE_AREA_LENGTH; column < PLAYABLE_AREA_LENGTH+1; column++) {
            move_tail(&tails[i], column, row);
            // Uncomment next line to see debug output.
            // printf("Tail[%d] - x: %.1f, y: %.1f\n", i, tails[i].x, tails[i].y);
            i++;
        }
    }
    printf("[Debug] - Tail moves total: %d\n", i);
}

Memory_Arena alloc_memory_arena(u64 capacity) {
    Memory_Arena mem;
    mem.data = (u8 *) malloc(capacity * sizeof(u8));
    if (mem.data) {
        printf("Memory arena successfully allocated at %p.\n", &mem.data[0]);
    } else {
        printf("Couldn't allocate memory for memory arena!\n");
    }
    mem.capacity = capacity;
    mem.size = 0;
    return mem;
}
