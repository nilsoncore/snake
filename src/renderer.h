#ifndef SNAKE_RENDERER_H
#define SNAKE_RENDERER_H

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "snake.h"

// Font rendering.
#include <ft2build.h>
#include FT_FREETYPE_H

//
// --- Structs ---
//
struct Shader;
struct Shader_Source;
struct Vertex_Buffer;
struct Index_Buffer;
struct Glyph;
struct Font;
struct Renderer_Info;
struct Rectangle;
struct Button;
struct Dropdown;
struct Checkbox;
struct Option;
struct Screen_Text;
enum Option_Input_Kind;
enum Align_Flag;

struct Shader {
    unsigned int id;
    const char *vertex_shader;
    const char *fragment_shader;
};

struct Shader_Source {
    const char *vertex_shader;
    const char *fragment_shader;
};

struct Vertex_Buffer {
    unsigned int id;
    unsigned int size;
    const void *data;
};

struct Index_Buffer {
    unsigned int id;
    unsigned int amount;
    const unsigned int *data;
};

struct Glyph {
    unsigned int texture_id;
    Vec2i size;
    Vec2i bearing;
    unsigned int advance;
};

struct Font {
    int width = 0;
    int height = 0;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    Glyph glyphs[128]; // For now it's just ASCII.
    int max_glyph_length_px = 0;
    int max_glyph_height_px = 0;
};

struct Renderer_Info {
    const char *gpu_vendor;
    const char *gpu_renderer;
    const char *gl_version;
    const char *glsl_version;
};

struct Triangle {
    float x0, y0;
    float x1, y1;
    float x2, y2;
};

struct Rectangle {
    float x;
    float y;
    int width;
    int height;
};

struct Rectanglei {
    int x;
    int y;
    int width;
    int height;
};

struct Button {
    float x;
    float y;
    int width;
    int height;
    const char *text;
    Font *font;
    Vec3f text_color;
    Vec3f button_color;
    u16 align_flags;
};

struct Dropdown {
    float x;
    float y;
    int width;
    int height;
    int items_count;
    int active_item;
    const char *items;
    Vec3f bg_color;
    Vec3f text_color;
    bool opened; // 'true' when it's pressed, 'false' otherwise.
};

struct Checkbox {
    float x;
    float y;
    union {
	int width;
	int height;
    };
    Vec3f color;
    bool *value_ptr;
    bool checked;
};

struct Screen_Text {
    float x;
    float y;
    const char *text;
    int length_px;
    int max_glyph_height_px;
    Vec3f color;
    u16 flags;
};

// @Incomplete
// @Unused
struct Screen_Text_Column {
    float x;
    float y;
    int size;
    const char **text;
};

struct Option {
    const char *name;
    int input_kind;
};

enum Option_Input_Kind {
    OPTION_INPUT_BUTTON = 1,
    OPTION_INPUT_DROPDOWN = 2,
    OPTION_INPUT_CHECKBOX = 3
};

enum Align_Flag {
    TEXT_ALIGN_ORIGIN = 0,

    TEXT_ALIGN_CENTER_WIDTH = (1 << 0),
    TEXT_ALIGN_CENTER_HEIGHT = (1 << 1),
    TEXT_ALIGN_CENTER = TEXT_ALIGN_CENTER_WIDTH | TEXT_ALIGN_CENTER_HEIGHT,

    TEXT_ALIGN_RIGHT_WIDTH = (1 << 2),
    TEXT_ALIGN_RIGHT_HEIGHT = (1 << 3),
    TEXT_ALIGN_RIGHT = TEXT_ALIGN_RIGHT_WIDTH | TEXT_ALIGN_RIGHT_HEIGHT,

    BUTTON_SIZE_TEXT_RELATIVE = 0,

    BUTTON_WIDTH_CONSTANT = (1 << 4),
    BUTTON_HEIGHT_CONSTANT = (1 << 5),
    BUTTON_SIZE_CONSTANT = BUTTON_WIDTH_CONSTANT | BUTTON_HEIGHT_CONSTANT,

    COLUMN_ALIGN_CENTER_HEIGHT = (1 << 6)
};

//
// --- Functions ---
//
void init_renderer();
void renderer_free_resources();
void renderer_draw(u32 game_state);
void draw_title_screen();
void draw_pause_screen();
void draw_settings_screen();
void draw_square_tiles();
Rectangle draw_text(Font *font, Screen_Text text, float scale = 1.0f);
Rectangle draw_text(Font *font, const char *text, float x, float y, float scale, Vec3f color, u16 flags = TEXT_ALIGN_ORIGIN);
Rectangle draw_text2(Font *font, const char *text, float x, float y, float scale, Vec3f color, u16 flags = TEXT_ALIGN_ORIGIN);
void draw_text_column(int amount, Rectangle *return_array, Font *font, const char **text_array, float x, float y, float text_gap, float scale, Vec3f color, u16 flags = TEXT_ALIGN_ORIGIN);
void draw_text_column(int amount, Rectangle *return_array, Font *font, Screen_Text text, float button_gap, float scale = 1.0f);
Rectangle draw_button(Font *font, const char *text, float x, float y, float scale, Vec3f text_color, Vec3f button_color, u16 flags = 0);
void draw_button_column(int amount, Rectangle *return_array, Font *font, const char **text_array, float x, float  y, float button_gap, float scale, Vec3f text_color, Vec3f button_color, u16 flags = 0);
void draw_rect(Rectangle rect, Vec3f color, int draw_mode = GL_TRIANGLES);
void draw_triangle(Triangle tri, Vec3f color, int draw_mode = GL_TRIANGLES);
void draw_rects(int amount, Rectangle *rects, Vec3f color, int draw_mode = GL_TRIANGLES);
Screen_Text new_screen_text(const char *text, Font *font, float x, float y, Vec3f color = new_vec3f(1.0f), u16 flags = 0);
Rectangle draw_dropdown(Dropdown drop, u16 flags = 0, int draw_mode = GL_TRIANGLES);
Rectangle draw_checkbox(Checkbox box, int draw_mode = GL_TRIANGLES);
void resize_screen(int new_width, int new_height);
void switch_fullscreen();
void destroy_window(GLFWwindow *window);
void set_current_context(GLFWwindow *window);
GLFWwindow *create_window(int width, int height, const char *title, bool fullscren);
void set_viewport(int x, int y, int width, int height);
inline bool in_rect_frame(float x, float y, Rectangle rect);
inline bool in_rect_frame(int x, int y, Rectanglei rect);
inline bool is_in_range(int value, int min_value, int max_value);
inline bool in_window_frame(int x, int y, Rectanglei window);

// Shaders
unsigned int load_shader(const char *vertex_shader_filepath, const char *fragment_shader_filepath);
unsigned int compile_shader(unsigned int type, char *source);
unsigned int create_shader(char *vertex_shader, char *fragment_shader);
void gl_clear_error();
bool gl_log_call(const char *function, const char *file, int line);
inline Rectangle new_rect(float x, float y, int width, int height);
inline Triangle new_triangle(float x0, float y0, float x1, float y1, float x2, float y2);

// GLFW Callbacks
void framebuffer_size_callback(GLFWwindow *window, int new_width, int new_height);
void cursor_pos_callback(GLFWwindow *window, double new_cursor_x, double new_cursor_y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int flags);
void cursor_enter_window_callback(GLFWwindow *window, int entered);
void scroll_callback(GLFWwindow *window, double offset_x, double offset_y);
void single_press_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void error_callback(int error_code, const char *description);

// Unlike 'single_press_key_callback()' function it can process
// long key press (press and hold) since it's called every frame.
void process_input(GLFWwindow *window);

// Internal functions
GLFWwindow *init_glfw();
Font load_font_from_file(const char *filepath, int pixel_width, int pixel_height);
u64 load_file_into_memory_arena(const char *filepath, u8 *memory_chunk);
int string_length(const char *text);
void process_button_click(Rectangle *buttons);

#endif /*SNAKE_RENDERER_H*/
