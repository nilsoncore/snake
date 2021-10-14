// fopen()
#define _CRT_SECURE_NO_WARNINGS 1

#include "renderer.h"

extern Screen screen;
extern Cursor cursor;
extern Frametime frametime;
extern Renderer_Info renderer_info;
extern GLFWwindow *window;

// Globals from snake.cpp
Player player;
Resource resource;
bool imgui_states[];
u32 game_state;

static unsigned int lighting_shader;
static unsigned int glyphs_shader;
static unsigned int rect_shader;

static unsigned int square_vbo;
static unsigned int square_vao;
static unsigned int rect_vbo; // rect = Rectangle
static unsigned int rect_vao;

static Font roboto;
static glm::mat4 projection;
static glm::mat4 text_projection;

static unsigned int square_projection_location;
static unsigned int square_model_location;
static unsigned int square_color_location;
static unsigned int rect_projection_location;
static unsigned int rect_color_location;
static unsigned int text_projection_location;
static unsigned int text_color_location;

static ImGuiContext *imgui_context;

static Vec4f clear_color = new_vec4f(0.1f, 0.1f, 0.1f, 1.0f);

static Rectangle buttons[4];

static unsigned int *font_textures;

static int windowed_x;
static int windowed_y;
static int windowed_width;
static int windowed_height;

unsigned int load_shader(const char *vertex_shader_filepath, const char *fragment_shader_filepath) {
    ZoneScoped;
    
    FILE *file;
    const char *filepath;
    char *vertex_src = NULL;
    char *fragment_src = NULL;

    if (vertex_shader_filepath) {
        filepath = vertex_shader_filepath;
        int open_result = fopen_s(&file, filepath, "r"); // Open file in 'read' mode.
        if (open_result) {
            printf("Couldn't open '%s' shader file!\n", filepath);
            fclose(file);
        }

        // Get vertex shader file size.
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        // Allocate memory to contain the whole file.
        vertex_src = (char *) malloc(sizeof(char) * file_size);
        if (!vertex_src) {
            printf("Error occured while allocating %llu bytes of memory for '%s' shader file!\n", sizeof(char) * file_size, filepath);
        }

        size_t bytes_read = fread(vertex_src, sizeof(char), file_size, file);
        if (bytes_read != file_size) {
            // printf("WARNING: Read buffer is not fully occupied while reading '%s' shader file! (%llu of %llu bytes read)\nWARNING: ... setting the rest of buffer to null.\n", filepath, bytes_read, file_size);
            memset(&vertex_src[bytes_read], '\0', file_size - bytes_read); 
        }

        fclose(file);
    }

    if (fragment_shader_filepath) {
        filepath = fragment_shader_filepath;
        int open_result = fopen_s(&file, filepath, "r"); // Open file in 'read' mode.
        if (open_result) {
            printf("Couldn't open '%s' shader file!\n", filepath);
            fclose(file);
        }

        // Get vertex shader file size.
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        // Allocate memory to contain the whole file.
        fragment_src = (char *) malloc(sizeof(char) * file_size);
        if (!fragment_src) {
            printf("Error occured while allocating %llu bytes of memory for '%s' shader file!\n", sizeof(char) * file_size, filepath);
        }

        size_t bytes_read = fread(fragment_src, 1, file_size, file);
        if (bytes_read != file_size) {
            // printf("WARNING: Read buffer is not fully occupied while reading '%s' shader file! (%llu of %llu bytes read)\nWARNING: ... setting the rest of buffer to null.\n", filepath, bytes_read, file_size);
            memset(&fragment_src[bytes_read], 0, file_size - bytes_read); 
        }

        fclose(file);
    }

    unsigned int program = glCreateProgram();
    unsigned int compiled_vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_src);
    unsigned int compiled_fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    glAttachShader(program, compiled_vertex_shader);
    glAttachShader(program, compiled_fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(compiled_vertex_shader);
    glDeleteShader(compiled_fragment_shader);

    if (program) {
        printf("Shader '%s' loaded.\n", vertex_shader_filepath);
    } else {
        printf("Failed to load '%s' shader!\n", vertex_shader_filepath);
        assert(false);
    }

    free(vertex_src);
    free(fragment_src);
    return program;
}

void init_renderer() {
    ZoneScoped;
    
    window = init_glfw();

    // Init OpenGL related information.
    renderer_info.gpu_vendor   = (const char *)glGetString(GL_VENDOR);
    renderer_info.gpu_renderer = (const char *)glGetString(GL_RENDERER);
    renderer_info.gl_version   = (const char *)glGetString(GL_VERSION);
    renderer_info.glsl_version = "#version 330";
    printf("GPU Vendor: %s, GPU Renderer: %s\n", renderer_info.gpu_vendor, renderer_info.gpu_renderer);
    printf("OpenGL/Driver version: %s\n", renderer_info.gl_version);
    printf("GLSL: %s\n", renderer_info.glsl_version);

    // Load shaders.
    lighting_shader = load_shader("resources/shaders/lighting_vertex.glsl", "resources/shaders/lighting_fragment.glsl");
    glyphs_shader = load_shader("resources/shaders/glyphs_vertex.glsl", "resources/shaders/glyphs_fragment.glsl");
    rect_shader = load_shader("resources/shaders/rect_vertex.glsl", "resources/shaders/rect_fragment.glsl");

    // Create 'Vertex Buffer' and 'Vertex Array' objects for square tiles.
    glGenBuffers(1, &square_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SQUARE_VERTICES), SQUARE_VERTICES, GL_STATIC_DRAW);

    glGenVertexArrays(1, &square_vao);
    glBindVertexArray(square_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, NULL);
    glEnableVertexAttribArray(0);

    // And for button rectangles.
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, NULL);
    glEnableVertexAttribArray(0);

    font_textures = (unsigned int *) malloc(128 * sizeof(unsigned int));
    glGenTextures(128, &font_textures[0]);

    // Load font.
    roboto = load_font_from_file("resources/fonts/Roboto-Regular.ttf", 0, screen.height/24);
    printf("sizeof(roboto): %llu bytes.\n", sizeof(roboto));

    // Init uniforms location.
    square_projection_location = glGetUniformLocation(lighting_shader, "projection");
    square_model_location = glGetUniformLocation(lighting_shader, "model");
    square_color_location = glGetUniformLocation(lighting_shader, "color");

    rect_projection_location = glGetUniformLocation(rect_shader, "projection");
    rect_color_location = glGetUniformLocation(rect_shader, "color");

    text_projection_location = glGetUniformLocation(glyphs_shader, "text_projection");
    text_color_location = glGetUniformLocation(glyphs_shader, "text_color");

    // Init ImGui.
    IMGUI_CHECKVERSION();
    imgui_context = ImGui::CreateContext();
    if (imgui_context) {
        printf("ImGui context created.\n");
    } else {
        printf("Failed to create ImGui context!\n");
        assert(false);
    }
    ImGuiIO &imgui_io = ImGui::GetIO();

    // Disable windows states saving for now.
    imgui_io.IniFilename = NULL;
    imgui_io.WantSaveIniSettings = false;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(renderer_info.glsl_version);

    ImGui::SetCurrentContext(imgui_context);

    // Enable blend for drawing text.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(screen.clear_color.r, screen.clear_color.g, screen.clear_color.b, screen.clear_color.a);

    float w = (float)screen.width;
    float h = (float)screen.height;
    text_projection = glm::ortho(0.0f, w, 0.0f, h);
    projection = glm::ortho(-w, w, -h, h);
}

void resize_screen(int new_width, int new_height) {
    screen.width = new_width;
    screen.height = new_height;
    screen.aspect_ratio = (float)new_width / (float)new_height;
    glViewport(0, 0, new_width, new_height);
    if (!screen.resized) screen.resized = !screen.resized;
}

/*inline*/
bool in_window_frame(int x, int y, Rectanglei window) {
    return (x >= window.x && x <= window.x + window.width)
	&& (y >= window.y && y <= window.y + window.height);
}

GLFWmonitor *get_last_windowed_monitor() {
    int monitors_count;
    auto monitors = glfwGetMonitors(&monitors_count);

    static GLFWmonitor *last_windowed_monitor = NULL;
    Rectanglei window_rect;
    glfwGetWindowPos(window, &window_rect.x, &window_rect.y);
    glfwGetWindowSize(window, &window_rect.width, &window_rect.height);
    Rectanglei monitor_rect;
    int window_center_x = window_rect.x + window_rect.width/2;
    int window_center_y = window_rect.y + window_rect.height/2;
    For (monitors_count) {
	glfwGetMonitorWorkarea(monitors[it], &monitor_rect.x, &monitor_rect.y, &monitor_rect.width, &monitor_rect.height);
	    
	if (in_window_frame(window_center_x, window_center_y, monitor_rect)) {
	    last_windowed_monitor = monitors[it];
	    printf("Last windowed monitor index: %d\n", it);
	    break;
	}
    }

    return last_windowed_monitor;
}

void switch_fullscreen() {
    auto fullscreen_monitor = glfwGetPrimaryMonitor();
    auto videomode = glfwGetVideoMode(fullscreen_monitor);
    screen.fullscreen = !screen.fullscreen;

    if (screen.fullscreen) {
	// If we enable fullscreen mode, then we save window position,
	// set screen resolution to max monitor resolution, and
	// open window in fullscreen mode on primary monitor.
	// Max FPS clamps to refresh rate of the current videomode of the monitor.
	auto last_windowed_monitor = get_last_windowed_monitor();
	assert(last_windowed_monitor);

	glfwGetWindowPos(window, &windowed_x, &windowed_y);
	glfwGetWindowSize(window, &windowed_width, &windowed_height);
	screen.width = videomode->width;
	screen.height = videomode->height;
	glfwSetWindowMonitor(window, last_windowed_monitor, 0, 0, videomode->width, videomode->height, videomode->refreshRate);
	assert(screen.width = videomode->width);
	assert(screen.height = videomode->height);
    } else {
	// If we disable fullscreen mode, then we reset screen resolution
	// to saved settings from windowed mode and open window in windowed mode.
	// Max FPS clamps to refresh rate of the current videomode of the monitor.
	screen.width = windowed_width;
	screen.height = windowed_height;
	glfwSetWindowMonitor(window, NULL, windowed_x, windowed_y, screen.width, screen.height, 0);
    }
}

void set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void destroy_window(GLFWwindow *window) {
    glfwDestroyWindow(window);
    printf("GLFW Window destroyed.\n");
}

void set_current_context(GLFWwindow *window) {
    glfwMakeContextCurrent(window);

    ImGuiIO &imgui_io = ImGui::GetIO();
    imgui_io.IniFilename = NULL;
    imgui_io.WantSaveIniSettings = false;
    ImGui::StyleColorsDark();

    ImGui::SetCurrentContext(imgui_context);
    
    printf("Set new window context.\n");
}

GLFWwindow *create_window(int width, int height, const char *title, bool fullscreen) {
    GLFWmonitor *monitor = (fullscreen) ? glfwGetPrimaryMonitor() : NULL;
    auto window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (window) {
	printf("GLFW Window created. (%dx%d, %s)\n", width, height, (monitor) ? "Fullscreen" : "Windowed");
    } else {
	printf("GLFW ERROR: Failed to create GLFW window!\n");
	assert(false);
	exit(EXIT_FAILURE);
    }
    set_viewport(0, 0, screen.width, screen.height);
    return window;
}

/*                                OpenGL                             */

//
// --- Shaders ---
//
unsigned int compile_shader(unsigned int type, char *source) {
    ZoneScoped;

    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (!result) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) malloc(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        printf("OpenGL ERROR: Failed to compile '%s' shader!\n... %s\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment", message);
        glDeleteShader(id);
        free(message);
        return 0;
    }

    return id;
}

unsigned int create_shader(char *vertex_shader, char *fragment_shader) {
    ZoneScoped;
    
    unsigned int program = glCreateProgram();
    unsigned int compiled_vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader);
    unsigned int compiled_fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, compiled_vertex_shader);
    glAttachShader(program, compiled_fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(compiled_vertex_shader);
    glDeleteShader(compiled_fragment_shader);

    return program;
}

//
// --- OpenGL Error handling ---
//
void gl_clear_error() {
    while (glGetError() != GL_NO_ERROR);
}

bool gl_log_call(const char *function, const char *file, int line) {
    while (GLenum error = glGetError()) {
        printf("OpenGL ERROR: (%d): %s %s: %d\n", error, function, file, line);
        return false;
    }
    return true;
}

/*                                GLFW                             */

GLFWwindow *init_glfw() {
    ZoneScoped;
    
    GLFWwindow *window;

    glfwSetErrorCallback(error_callback);

    if (glfwInit()) {
	printf("GLFW initialized.\n");
    } else {
	printf("ERROR: Failed to initialize GLFW!\n");
	assert(false);
	exit(EXIT_FAILURE);
    }

    // OpenGL 3.3, Core profile.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context.
    // Put  glfwGetPrimaryMonitor()  after the window's name to make it fullscreen.
    // ... "name", glfwGetPrimaryMonitor(), NULL);
    // GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    auto monitor = glfwGetPrimaryMonitor();
    auto videomode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, videomode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, videomode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, videomode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, videomode->refreshRate);

    // screen.width = videomode->width;
    // screen.height = videomode->height;

    int windowxpos;
    int windowypos;
    glfwGetMonitorWorkarea(monitor, &windowxpos, &windowypos, &screen.width, &screen.height);

    monitor = (screen.fullscreen) ? monitor : NULL;
    window = glfwCreateWindow(screen.width, screen.height, "snake", monitor, NULL);
    if (window) {
	printf("GLFW Window created. (%dx%d, %s)\n", screen.width, screen.height, (monitor) ? "Fullscreen" : "Windowed");
    } else {
	printf("GLFW ERROR: Failed to create window!\n");
	assert(false);
	exit(EXIT_FAILURE);
    }
    // glfwSetWindowPos(window, 0, 0);

    glfwMakeContextCurrent(window);

    // In the 2-nd argument we are telling GLFW
    // the function, that will execute whenever
    // frame buffer changes (when window changes its size)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_window_callback);
    // Callback function for processing single press.
    // Keys which are being listened continuously are
    // in process_input().
    glfwSetKeyCallback(window, single_press_key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Tell GLFW to capture mouse movements and draw the cursor.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPos(window, cursor.x, cursor.y);

    // Vertical sync (VSync).
    // 1 - sync to a monitor refresh rate (60Hz = 60fps, 144hz = 144fps)
    // 2 - 1/2 of a monitor refresh rate (60Hz = 30fps)
    // 3 - 1/3...
    // glfwSwapInterval(1);

    if (glewInit() == GLEW_OK) {
	printf("GLEW initialized.\n");
    } else {
	printf("ERROR: Failed to initialize GLEW!\n");
	assert(false);
	exit(EXIT_FAILURE);
    }

    return window;
}

//
// --- Callbacks ---
//
void framebuffer_size_callback(GLFWwindow *window, int new_width, int new_height) {
    resize_screen(new_width, new_height);
}

void cursor_pos_callback(GLFWwindow *window, double new_cursor_x, double new_cursor_y) {
    cursor.x = new_cursor_x;
    // We invert the Y-coordinate value since OpenGL's screen origin is the upper-left corner
    // but we want the bottom-left one.
    cursor.y = abs(new_cursor_y - screen.height);
}

/*inline*/
bool in_rect_frame(float x, float y, Rectangle rect) {
    return (x <= rect.x + rect.width && x >= rect.x - rect.width) 
	&& (y <= rect.y + rect.height && y >= rect.y - rect.height);
}

/*inline*/
bool in_rect_frame(int x, int y, Rectanglei rect) {
    return (x <= rect.x + rect.width && x >= rect.x - rect.width)
	&& (y <= rect.y + rect.height && y >= rect.y - rect.height);
}

/*inline*/
bool is_in_range(int value, int min_value, int max_value) {
    return (value >= min_value && value <= max_value);
}

static void print_game_state(u32 game_state) {
    bool states[] = { 
        game_state & PLAY,
        game_state & DEBUG,
        game_state & TITLE_SCREEN,
        game_state & PAUSE_SCREEN,
        game_state & SETTINGS_SCREEN
    };
    printf("Game state:\n"
    " - PLAY: %d\n"
    " - DEBUG: %d\n"
    " - TITLE_SCREEN: %d\n"
    " - PAUSE_SCREEN: %d\n"
    " - SETTINGS_SCREEN: %d\n",
    states[0],
    states[1],
    states[2],
    states[3],
    states[4]);
}

void process_button_click(Rectangle *buttons) {
    ZoneScoped;
    
    if (game_state & TITLE_SCREEN) {
        For (3 /*buttons*/) {
            bool button_clicked = in_rect_frame(cursor.x, cursor.y, buttons[it]);
            if (button_clicked) {
                if (it == 0) /*New Game*/ {
                    game_state ^= TITLE_SCREEN;
                    game_state |= PLAY;
                    printf("[%.2f] - New game started.\n", frametime.current);
                    print_game_state(game_state);
                    game_reset();
                } else if (it == 1) /*Settings*/ {
                    game_state |= SETTINGS_SCREEN;
                    print_game_state(game_state);
                } else if (it == 2) /*Quit*/ {
                    print_game_state(game_state);
                    game_exit();
                }
            }
        }
    } else if (game_state & PAUSE_SCREEN) {
        For (4 /*buttons*/) {
            bool button_clicked = in_rect_frame(cursor.x, cursor.y, buttons[it]);
            if (button_clicked) {
                if (it == 0) /*Continue*/ {
                    game_state ^= PAUSE_SCREEN;
                    print_game_state(game_state);
                } else if (it == 1) /*Settings*/ {
                    game_state ^= SETTINGS_SCREEN;
                    print_game_state(game_state);
                } else if (it == 2) /*Quit Session*/ {
                    game_state ^= PAUSE_SCREEN;
                    game_state ^= TITLE_SCREEN;
		    game_state ^= PLAY;
                    print_game_state(game_state);
                } else if (it == 3) /*Quit Game*/ {
                    // save_session();
                    game_exit();
                }
            }
        }
    }
} 

void mouse_button_callback(GLFWwindow *window, int button, int action, int flags) {
    if (action == GLFW_PRESS) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
	    process_button_click(&buttons[0]);
	}
    }
}

void cursor_enter_window_callback(GLFWwindow *window, int entered) {
    if (entered) {
	// printf("[%.2f] - Cursor entered the window.\n", frametime->current);
    } else {
	// printf("[%.2f] - Cursor left the window.\n", frametime->current);
    }
}

void scroll_callback(GLFWwindow *window, double offset_x, double offset_y) {
    // ...
}

void process_input(GLFWwindow *window) {
    ZoneScoped;
    
    // NOTE: It doesn't work like GLFW key press callback function
    // where there is additional GLFW_REPEAT and GLFW_PRESS is treated
    // as a single press. Here, GLFW_PRESS updates every frame.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
	// TODO: Move up
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
	// TODO: Move down
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
	// TODO: Move left
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
	// TODO: Move right
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
	// ...
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
	// TODO: Open main menu
    }
}

void single_press_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ZoneScoped;
    
    if (action == GLFW_PRESS) {
        if (game_state == PLAY) {
            if (key == GLFW_KEY_W) {
                move_player(&player, 0, 1);
            }

            if (key == GLFW_KEY_S) {
                move_player(&player, 0, -1);
            }

            if (key == GLFW_KEY_A) {
                move_player(&player, -1, 0);
            }

            if (key == GLFW_KEY_D) {
                move_player(&player, 1, 0);
            }
        }

	if (key == GLFW_KEY_GRAVE_ACCENT) {
	    imgui_states[DRAW_EDIT_WINDOW] = !imgui_states[DRAW_EDIT_WINDOW];
	    printf("[%.2f] - 'Edit' window mode: %s\n", frametime.current, (imgui_states[DRAW_EDIT_WINDOW]) ? "SHOW" : "HIDE");
	}

	if ((key == GLFW_KEY_ENTER) && (mods & GLFW_MOD_ALT)) {
	    switch_fullscreen();
	}

	if (key == GLFW_KEY_ESCAPE) {
            if (game_state & PLAY) {
                if (game_state & SETTINGS_SCREEN) {
                    game_state ^= SETTINGS_SCREEN;
                } else {
		    game_state ^= PAUSE_SCREEN;
		}
		print_game_state(game_state);
            } else /*TITLE_SCREEN*/ {
		if (game_state & SETTINGS_SCREEN) {
		    game_state ^= SETTINGS_SCREEN;
		}
	    }

            /*
	      cursor.cursor_mode = !cursor.cursor_mode;
	      if (cursor.cursor_mode) {
	      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	      } else {
	      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	      }

	      printf("[%.2f] - Cursor mode: %s\n", frametime.current, (cursor.cursor_mode) ? "ON" : "OFF");
            */
	}
    }
}

void error_callback(int error_code, const char *description) {
    printf("GLFW ERROR: %s (Error code: %d)\n", description, error_code);
}

// @MemoryLeak
Font load_font_from_file(const char *filepath, int pixel_width, int pixel_height) {
    ZoneScoped;

    printf("loading font...\n");
    Font font;
    font.width = pixel_width;
    font.height = pixel_height;

    // Generate vertex buffer and vertex array if they are not generated yet.
    if (!font.vbo) {
	glGenBuffers(1, &font.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    }
    if (!font.vao) {
        glGenVertexArrays(1, &font.vao);
        glBindVertexArray(font.vao);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
    }

    FT_Library freetype;
    if (FT_Init_FreeType(&freetype)) {
        printf("FreeType ERROR: Couldn't initialize FreeType library!\n");
    }

    FT_Face face;
    if (FT_New_Face(freetype, filepath, 0, &face)) {
	printf("FreeType ERROR: Couldn't load '%s'!\n", filepath);
    }

    FT_Set_Pixel_Sizes(face, font.width, font.height);

    // Disable byte-alignment restriction.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int glyph_length_px;
    int glyph_height_px;
    for (unsigned char c = 0; c < 128; c++) {
	// Load character glyph.
	if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
	    printf("[%.2f] - FreeType ERROR: Failed to load a Glyph for char '%c'!\n", frametime.current, c);
	    continue;
	}

	// Generate texture.
	// unsigned int texture;
	// glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, font_textures[c]);
	glTexImage2D(
	    /* target         */ GL_TEXTURE_2D,
	    /* level          */ 0,
	    /* internalFormat */ GL_RED,
	    /* width          */ face->glyph->bitmap.width,
	    /* height         */ face->glyph->bitmap.rows,
	    /* border         */ 0,
	    /* format         */ GL_RED,
	    /* type           */ GL_UNSIGNED_BYTE,
	    /* data           */ face->glyph->bitmap.buffer
	    );

	// Set texture options.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Now store character for later use.
	Glyph glyph = {
	    font_textures[c],
	    new_vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	    new_vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
	    face->glyph->advance.x
	};
	font.glyphs[c] = glyph;

	// Evaluate max glyph parameters.
	glyph_length_px = (glyph.advance >> 6);
	if (glyph_length_px > font.max_glyph_length_px) {
	    font.max_glyph_length_px = glyph_length_px;
	}

	glyph_height_px = glyph.size.y;
	if (glyph_height_px > font.max_glyph_height_px) {
	    font.max_glyph_height_px = glyph_height_px;
	}
    }

    FT_Done_Face(face);

    return font;
}

u64 load_file_into_memory_arena(const char *filepath, u8 *memory_chunk) {
    FILE *file;
    file = fopen(filepath, "rb"); // Read-binary mode.
    if (!file) {
        printf("Couldn't open '%s' file!\n", filepath);
        fclose(file);
    }

    fseek(file, 0, SEEK_END);
    u64 file_size = ftell(file);
    rewind(file);

    u64 bytes_read = fread(memory_chunk, sizeof(u8), file_size, file);
    if (bytes_read != file_size) {
        printf("WARNING: Read buffer is not fully occupied while reading '%s' file! (%llu of %llu bytes read)\nWARNING: ... setting the rest of buffer to null.\n", filepath, bytes_read, file_size);
        memset(&memory_chunk[bytes_read], 0, file_size - bytes_read); 
    }

    fclose(file);
    return bytes_read;
}

inline Rectangle draw_text(Font *font, Screen_Text text, float scale /*= 1.0f*/) {
    return draw_text(font, text.text, text.x, text.y, scale, text.color, text.flags);
}

inline void draw_text_column(int amount, Rectangle *return_array, Font *font, Screen_Text text, float button_gap, float scale /*= 1.0f*/) {
    // draw_text_column(amount, return_array, font, screen
}

Rectangle draw_text(Font *font, const char *text, float x, float y, float scale, Vec3f color, u16 flags /*= TEXT_ALIGN_ORIGIN*/) {
    ZoneScoped;

    glUseProgram(glyphs_shader);
    glUniformMatrix4fv(text_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(text_color_location, 1, &color.x);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->vao);

    float original_x = x;

    // px = pixels
    unsigned int text_length_px = 0;
    unsigned int text_max_glyph_height_px = 0;
    int text_size = string_length(text);
    Glyph glyph;
    
    For (text_size) {
	glyph = font->glyphs[text[it]];
	text_length_px += (glyph.advance >> 6) * scale;
	if (glyph.size.y > text_max_glyph_height_px) {
	    text_max_glyph_height_px = glyph.size.y;
	}
    }
	
    int half_width = text_length_px / 2;
    int half_height = text_max_glyph_height_px / 2;

    // Return text's X position.
    x = original_x;

    if (flags & TEXT_ALIGN_CENTER_WIDTH) {
	x -= half_width;
    } else if (flags & TEXT_ALIGN_RIGHT_WIDTH) {
	x -= half_width * 2;
    }

    Rectangle rect;
    rect.x = x + half_width;
    rect.y = y + half_height;
    rect.width = half_width;
    rect.height = half_height;
    
    // Iterate through all characters in text.
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);	
    For (text_size) {
	glyph = font->glyphs[text[it]];

	float xpos = x + glyph.bearing.x * scale;
	float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;

	float w = glyph.size.x * scale;
	float h = glyph.size.y * scale;

	// Update VBO for each character.
	float vertices[6][4] = {
	    { xpos,     ypos + h,   0.0f, 0.0f },
	    { xpos,     ypos,       0.0f, 1.0f },
	    { xpos + w, ypos,       1.0f, 1.0f },

	    { xpos,     ypos + h,   0.0f, 0.0f },
	    { xpos + w, ypos,       1.0f, 1.0f },
	    { xpos + w, ypos + h,   1.0f, 0.0f }
	};

	// Render glyph texture over quad.
	glBindTexture(GL_TEXTURE_2D, glyph.texture_id);

	// Update content of VBO memory.
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

	// Render quad.
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Now advance cursors for next glyph (note that advance is number of 1/64 pixels).
	x += (glyph.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64).
    }
    
    return rect;
}

void draw_square_tiles() {
    ZoneScoped;
    
    glUseProgram(lighting_shader);
    glBindVertexArray(square_vao);

    glUniformMatrix4fv(square_projection_location, 1, false, &projection[0][0]);

    // Draw Resource.
    glUniformMatrix4fv(square_model_location, 1, false, &resource.model[0][0]);
    glUniform3fv(square_color_location, 1, &resource.color[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw Player.
    glUniformMatrix4fv(square_model_location, 1, false, &player.model[0][0]);
    glUniform3fv(square_color_location, 1, &player.color[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw Player's tails.
    For (player.tail_length) {
	glUniformMatrix4fv(square_model_location, 1, false, &player.tails[it].model[0][0]);
	glUniform3fv(square_color_location, 1, &player.tails[it].color[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

// no scale.
Screen_Text new_screen_text(const char *text, Font *font, float x, float y, Vec3f color /*= new_vec3f(0.0f)*/, u16 flags /*= 0*/) {
    Screen_Text result;
    result.text = text;
    result.x = x;
    result.y = y;
    result.color = color;
    result.flags = flags;
    result.length_px = 0;
    result.max_glyph_height_px = 0;
    
    int text_size = string_length(text);
    Glyph glyph;
    For (text_size) {
	glyph = font->glyphs[text[it]];
	result.length_px += (glyph.advance >> 6); // Should have been (... * scale).
	if (glyph.size.y > result.max_glyph_height_px) {
	    result.max_glyph_height_px = glyph.size.y;
	}
    }

    return result;
}


// 'scale' doesn't work.
//
// Important Note: 'text_gap' is distance between previous and next text centers,
// not between their rectangle faces.
void draw_text_column(int amount, Rectangle *return_array, Font *font, const char **text_array, float x, float y, float text_gap, float scale, Vec3f color, u16 flags /*= TEXT_ALIGN_ORIGIN*/) {
    ZoneScoped;
    
    glUseProgram(glyphs_shader);
    glUniformMatrix4fv(text_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(text_color_location, 1, &color.x);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->vao);

    float original_x = x;

    // px = pixels
    unsigned int whole_text_max_length_px = 0;
    unsigned int whole_text_max_glyph_height_px = 0;
    const char *current_text;
    int text_size;
    Glyph glyph;
    
    for (int text_num = 0; text_num < amount; text_num++) {
	current_text = text_array[text_num];
	text_size = string_length(current_text);
	For (text_size) {
            glyph = font->glyphs[current_text[it]];
            whole_text_max_length_px += (glyph.advance >> 6) * scale;
            if (glyph.size.y > whole_text_max_glyph_height_px) {
		whole_text_max_glyph_height_px = glyph.size.y;
            }
	}
    }
	
    for (int text_num = 0; text_num < amount; text_num++) {
	unsigned int current_text_length_px = 0;
	unsigned int current_text_max_glyph_height_px = 0;
	current_text = text_array[text_num];
	text_size = string_length(current_text);

	For (text_size) {
	    glyph = font->glyphs[current_text[it]];
	    current_text_length_px += (glyph.advance >> 6) * scale;
	    if (glyph.size.y > current_text_max_glyph_height_px) {
		current_text_max_glyph_height_px = glyph.size.y;
	    }
	}

	int half_width;
	int half_height;

	if (flags & BUTTON_WIDTH_CONSTANT) {
	    half_width = whole_text_max_length_px / 2;
	} else /*BUTTON_SIZE_TEXT_RELATIVE*/ {
	    half_width = current_text_length_px / 2;
	}

	if (flags & BUTTON_HEIGHT_CONSTANT) {
	    half_height = whole_text_max_glyph_height_px / 2;
	} else /*BUTTON_SIZE_TEXT_RELATIVE*/ {
	    half_height = current_text_max_glyph_height_px / 2;
	}

	// Return text's X position.
	x = original_x;

	if (flags & TEXT_ALIGN_CENTER_WIDTH) {
	    x -= half_width;
	} else if (flags & TEXT_ALIGN_RIGHT_WIDTH) {
	    x -= half_width * 2;
	    // x -= current_text_length_px;
	}

	// Set rectangle's X position.
	Rectangle rect;
	rect.x = x + half_width;

	// Set rectangle's Y position.
	if (text_num == 0) {
	    rect.y = y;
	    if (flags & COLUMN_ALIGN_CENTER_HEIGHT) {
		rect.y += amount/2 * (rect.height + text_gap);
	    }
	} else {
	    rect.y = return_array[text_num-1].y - return_array[text_num-1].height - text_gap;
	}

	// Set rectangle's width and height.
	rect.width = half_width;
	rect.height = half_height;
	return_array[text_num] = rect;

	y = rect.y - half_height;

	// Iterate through all characters in text.
	glBindBuffer(GL_ARRAY_BUFFER, font->vbo);	
	For (text_size) {
	    glyph = font->glyphs[current_text[it]];

	    float xpos = x + glyph.bearing.x * scale;
	    float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;

	    float w = glyph.size.x * scale;
	    float h = glyph.size.y * scale;

	    // Update VBO for each character.
	    float vertices[6][4] = {
		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos,     ypos,       0.0f, 1.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },

		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },
		{ xpos + w, ypos + h,   1.0f, 0.0f }
	    };

	    // Render glyph texture over quad.
	    glBindTexture(GL_TEXTURE_2D, glyph.texture_id);

	    // Update content of VBO memory.
	    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

	    // Render quad.
	    glDrawArrays(GL_TRIANGLES, 0, 6);

	    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels).
	    x += (glyph.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64).
	}
    }
}

void renderer_draw(u32 game_state) {
    ZoneScoped; // For profiling in Tracy.
    
    frametime.current = glfwGetTime();
    frametime.delta = 1000.0f * (frametime.current - frametime.last);
    frametime.last = frametime.current;

    glClearColor(screen.clear_color.r, screen.clear_color.g, screen.clear_color.b, screen.clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);

    if (screen.resized) {
        // @Speed
        // Loading font from file every window resize is okay enough for now,
        // but later we really should store font in memory.
        // @MemoryLeak
        roboto = load_font_from_file("resources/fonts/Roboto-Regular.ttf", 0, screen.height/24);
        screen.resized = false;
 
       printf("[%.2f] - New window size: %dx%d\n", frametime.current, screen.width, screen.height);
        float w = (float)screen.width;
        float h = (float)screen.height;
        text_projection = glm::ortho(0.0f, w, 0.0f, h);
        projection = glm::ortho(-w, w, -h, h);
    }

    if (game_state & TITLE_SCREEN) {
        if (game_state & SETTINGS_SCREEN) {
            draw_settings_screen();
        } else {
            draw_title_screen();
        }
    } else if (game_state & PLAY) {
        if (game_state & DEBUG) {
            draw_all_tails_on_screen();
        } else {
            draw_square_tiles();
        }

        if (game_state & PAUSE_SCREEN) {
            if (game_state & SETTINGS_SCREEN) {
                draw_settings_screen();
            } else {
                draw_pause_screen();
            }
        }
    }

    //
    // --- ImGui Render ---
    //
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    make_imgui_layout();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
    glFlush();
}

void renderer_free_resources() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imgui_context);

    glDeleteProgram(lighting_shader);
    glDeleteProgram(glyphs_shader);

    glfwTerminate();
}

/*inline*/
Rectangle new_rect(float x, float y, int width, int height) {
    Rectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return rect;
}


void draw_title_screen() {
    ZoneScoped;
    
    int x = screen.width/2;
    int y = screen.height/2;
    float button_gap = screen.height/32;
    Vec3f text_color = new_vec3f(0.9f, 0.9f, 0.9f);
    Vec3f button_color = new_vec3f(1.0f, 0.0f, 0.0f);
    // Vec3f button_color = new_vec3f(clear_color.r, clear_color.g, clear_color.b);
    const char *text[] = { "snake", "New Game", "Settings", "Quit" };
    draw_text(&roboto, &text[0][0], x, y + screen.height/6, 1.0f, text_color, TEXT_ALIGN_CENTER);
    draw_button_column(3, &buttons[0], &roboto, &text[1], x, y, button_gap, 1.0f, text_color, button_color, TEXT_ALIGN_CENTER | BUTTON_SIZE_CONSTANT);
}

void draw_pause_screen() {
    ZoneScoped;
    
    int x = screen.width/2;
    int y = screen.height/2;
    float button_gap = screen.height/32;
    Vec3f text_color = new_vec3f(0.9f, 0.9f, 0.9f);
    Vec3f button_color = new_vec3f(1.0f, 0.0f, 0.0f);
    const char *text[] = { "Continue", "Settings", "Quit Session", "Quit Game" };
    draw_button_column(4, &buttons[0], &roboto, &text[0], x, y, button_gap, 1.0f, text_color, button_color, TEXT_ALIGN_CENTER | BUTTON_SIZE_CONSTANT | COLUMN_ALIGN_CENTER_HEIGHT);

    // For (4 /*rects*/) {
    // draw_rect(buttons[it], text_color, GL_LINE_STRIP);
    // }
}

void draw_triangle(Triangle tri, Vec3f color, int draw_mode /*= GL_TRIANGLES*/) {
    ZoneScoped;
    
    glUseProgram(rect_shader);
    glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(rect_color_location, 1, &color.x);

    glBindVertexArray(rect_vao);
    
    float vertices[6] = {
        tri.x0, tri.y0,
	tri.x1, tri.y1,
	tri.x2, tri.y2
    };

    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(draw_mode, 0, 3);
}

void draw_rect(Rectangle rect, Vec3f color, int draw_mode /*= GL_TRIANGLES*/) {
    ZoneScoped;
    
    glUseProgram(rect_shader);
    glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(rect_color_location, 1, &color.x);

    glBindVertexArray(rect_vao);

    float vertices[6][2] = {
	{ rect.x - rect.width, rect.y - rect.height },
	{ rect.x - rect.width, rect.y + rect.height },
	{ rect.x + rect.width, rect.y + rect.height },

	{ rect.x + rect.width, rect.y - rect.height },
	{ rect.x - rect.width, rect.y - rect.height },
	{ rect.x + rect.width, rect.y + rect.height }
    };

    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(draw_mode, 0, 6);
}

void draw_rects(int amount, Rectangle *rects, Vec3f color, int draw_mode /*= GL_TRIANGLES*/) {
    ZoneScoped;
    
    glUseProgram(rect_shader);
    glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(rect_color_location, 1, &color.x);

    glBindVertexArray(rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	
    For (amount) {
	Rectangle rect = rects[it];
		
	float vertices[6][2] {
	    { rect.x - rect.width, rect.y - rect.height },
	    { rect.x - rect.width, rect.y + rect.height },
	    { rect.x + rect.width, rect.y + rect.height },

	    { rect.x + rect.width, rect.y - rect.height },
	    { rect.x - rect.width, rect.y - rect.height },
	    { rect.x + rect.width, rect.y + rect.height }
	};

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(draw_mode, 0, 6);
    }
}

void print_rect(Rectangle rect, const char *name) {
    printf("Rectangle '%s':\n"
	   " - x: %.f\n"
	   " - y: %.f\n"
	   " - width: %d\n"
	   " - height: %d\n",
	   name, rect.x, rect.y, rect.width, rect.height);
}

void draw_settings_screen() {
    ZoneScoped;
    
    int x = screen.width/2;
    int y = screen.height/2;
    float button_gap = screen.height/32;
    float center_line_gap = button_gap/4;
    Rectangle menu_frame = new_rect(x, y, screen.width/4, screen.height/2 - button_gap);
    Vec3f button_color = new_vec3f(1.0f, 0.0f, 0.0f);
    Vec3f text_color = new_vec3f(0.9f, 0.9f, 0.9f);
    const char *text[] = { "Resolution:", "Window Mode:", "VSync:" };
    float gap_between_options = screen.height/32;
    Option options[] = {
	{ "Resolution:", OPTION_INPUT_DROPDOWN },
	{ "Window Mode:", OPTION_INPUT_DROPDOWN },
	{ "VSync:", OPTION_INPUT_CHECKBOX }
    };

    draw_rect(menu_frame, text_color, GL_LINE_STRIP);

    Rectangle text_rects[3];
    x = menu_frame.x - center_line_gap;
    y = menu_frame.y + menu_frame.height - button_gap;
    draw_text_column(3, &text_rects[0], &roboto, &text[0], x, y, button_gap, 1.0f, text_color, TEXT_ALIGN_RIGHT);

    Screen_Text scr_text = new_screen_text(text[0], &roboto, x, y, text_color, TEXT_ALIGN_RIGHT);
    // Rectangle scr_text_rect = draw_text(&roboto, scr_text);
    // draw_rect(scr_text_rect, text_color, GL_LINE_STRIP);

    For (3 /*rects*/) {
	// draw_rect(text_rects[it], text_color, GL_LINE_STRIP);
    }

    // Options part.
    const char *res_items[] = { "2560x1440", "1920x1080", "1280x720" };
    
    Dropdown res; // res = resolution
    res.x = menu_frame.x + center_line_gap;
    res.y = text_rects[0].y;
    res.width = menu_frame.width/4;
    res.height = text_rects[0].height;
    res.items = &res_items[0][0];
    res.items_count = 3;
    res.active_item = 0;
    res.bg_color = vec3f_from_vec4f(screen.clear_color);
    res.text_color = new_vec3f(1.0f, 0.0f, 0.0f);
    res.opened = true;

    Rectangle res_rect = draw_dropdown(res);
    // draw_rect(res_rect, text_color, GL_LINE_STRIP);

    const char *window_mode_items[] = { "Fullscreen", "Windowed", "Windowed (Borderless)" };
    
    Dropdown window_mode;
    window_mode.x = menu_frame.x + center_line_gap;
    window_mode.y = text_rects[1].y;
    window_mode.width = menu_frame.width/4;
    window_mode.height = text_rects[1].height;
    window_mode.items = &window_mode_items[0][0];
    window_mode.items_count = 3;
    window_mode.active_item = 0;
    window_mode.bg_color = vec3f_from_vec4f(screen.clear_color);
    window_mode.text_color = new_vec3f(1.0f, 0.0f, 0.0f);
    window_mode.opened = false;

    Rectangle window_mode_rect = draw_dropdown(window_mode);
    // draw_rect(window_mode_rect, text_color, GL_LINE_STRIP);

    Checkbox vsync;
    vsync.width = text_rects[2].height;
    // 'height' is the same as 'width' because it is a square and it is a union.
    vsync.x = menu_frame.x + center_line_gap + vsync.width;
    vsync.y = text_rects[2].y;
    vsync.color = new_vec3f(1.0f, 0.0f, 0.0f);
    vsync.checked = true;

    Rectangle vsync_rect = draw_checkbox(vsync);
    // draw_rect(vsync_rect, text_color, GL_LINE_STRIP);
}

float *get_rect_vertices(Rectangle rect) {
    ZoneScoped;
    
    float button_vertices[12] {
	rect.x - rect.width, rect.y + rect.height,
	rect.x + rect.width, rect.y + rect.height,
	rect.x - rect.width, rect.y - rect.height,

	rect.x + rect.width, rect.y + rect.height,
	rect.x + rect.width, rect.y - rect.height,
	rect.x - rect.width, rect.y - rect.height
    };

    return &button_vertices[0];
}

// 'gap' argument is referred to as gap between inside and outside rectangles of a checkbox.
Rectangle draw_checkbox(Checkbox box, int draw_mode /*= GL_TRIANGLES*/) {
    ZoneScoped;

    // @Temp, make it as an actual argument.
    int gap = box.width/6;
    int gap_size = box.width/4;

    Rectangle outside_rect = new_rect(box.x, box.y, box.width, box.height);
    draw_rect(outside_rect, box.color, draw_mode);

    Rectangle inside_bg_rect = outside_rect; // bg = background
    inside_bg_rect.width -= gap;
    inside_bg_rect.height -= gap;

    // We can't really yet convert 'Vector4f' to 'Vec3f', so we do this dummy operation.
    Vec3f clear_color;
    clear_color.r = screen.clear_color.r;
    clear_color.g = screen.clear_color.g;
    clear_color.b = screen.clear_color.b;
    draw_rect(inside_bg_rect, clear_color, GL_TRIANGLES);

    if (box.checked) {
	Rectangle inside_rect = inside_bg_rect;
	inside_rect.width -= gap_size;
	inside_rect.height -= gap_size;
	draw_rect(inside_rect, box.color, draw_mode);
    }

    return outside_rect;
}

// Make 'Font' argument?
// @Incomplete: use drop.color !!!
Rectangle draw_dropdown(Dropdown drop, u16 flags /*= 0*/, int draw_mode /*= GL_TRIANGLES*/) {
    ZoneScoped;
    
    glUseProgram(rect_shader);
    glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(rect_color_location, 1, &drop.bg_color.x);

    // Draw text.
    Rectangle text_rect = draw_text(
	&roboto,
	&drop.items[drop.active_item],
	drop.x,
	drop.y - drop.height,
	1.0f,
	drop.text_color,
	TEXT_ALIGN_ORIGIN
	);

    Rectangle drop_rect = new_rect(text_rect.x, text_rect.y, text_rect.width, text_rect.height);
    
    // Draw only for debug purposes.
    // draw_rect(drop_rect, bg_color, GL_LINE_STRIP);

    Rectangle select_rect;
    select_rect.width = drop_rect.height;
    select_rect.height = drop_rect.height;
    select_rect.x = (drop_rect.x + drop_rect.width) + select_rect.width;
    select_rect.y = drop_rect.y;

    draw_rect(select_rect, drop.text_color, draw_mode);

    float half_width = select_rect.width/2;
    float half_height = select_rect.height/2;
    
    Triangle select_tri;
    if (drop.opened) {
	// Make triangle "look down".
	select_tri.x0 = select_rect.x - half_width;
	select_tri.y0 = select_rect.y - half_height;
	select_tri.x1 = select_rect.x + half_width;
	select_tri.y1 = select_rect.y - half_height;
	select_tri.x2 = select_rect.x;
	select_tri.y2 = select_rect.y + half_height;
    } else {
	// Make triangle "look up".
	select_tri.x0 = select_rect.x - half_width;
	select_tri.y0 = select_rect.y + half_height;
	select_tri.x1 = select_rect.x + half_width;
	select_tri.y1 = select_rect.y + half_height;
	select_tri.x2 = select_rect.x;
	select_tri.y2 = select_rect.y - half_height;
    }

    draw_triangle(select_tri, drop.bg_color, draw_mode);

    Rectangle whole_dropdown_rect = drop_rect;
    whole_dropdown_rect.x += select_rect.width;
    whole_dropdown_rect.width += select_rect.width;
    
    return whole_dropdown_rect;
}

inline Triangle new_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
    Triangle result = { x0, y0, x1, y1, x2, y2 };
    return result;
};

// 'scale' doesn't work properly.
void draw_button_column(int amount, Rectangle *return_array, Font *font, const char **text_array, float x, float y, float button_gap, float scale, Vec3f text_color, Vec3f button_color, u16 flags /*= 0*/) {
    ZoneScoped;
    
    float original_x = x;

    // Prepare uniforms.
    glUseProgram(rect_shader);
    glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(rect_color_location, 1, &button_color.x);
    glUseProgram(glyphs_shader);
    glUniformMatrix4fv(text_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(text_color_location, 1, &text_color.x);

    unsigned int max_text_length_in_pixels = 0;
    unsigned int max_text_max_glyph_height_in_pixels = 0;
    const char *button_text;
    Glyph glyph;
    
    for (int button = 0; button < amount; button++) {
        button_text = text_array[button];
        int text_size = string_length(button_text);
        For (text_size) {
            glyph = font->glyphs[button_text[it]];
            max_text_length_in_pixels += (glyph.advance >> 6) * scale;
            if (glyph.size.y > max_text_max_glyph_height_in_pixels) {
                max_text_max_glyph_height_in_pixels = glyph.size.y;
            }
        }
    }

    for (int button = 0; button < amount; button++) {
        unsigned int text_length_in_pixels = 0;
        unsigned int text_max_glyph_height_in_pixels = 0;
        button_text = text_array[button];
        int text_size = string_length(button_text);
        For (text_size) {
            glyph = font->glyphs[button_text[it]];
            text_length_in_pixels += (glyph.advance >> 6) * scale;
            if (glyph.size.y > text_max_glyph_height_in_pixels) {
                text_max_glyph_height_in_pixels = glyph.size.y;
            }
        }

	Rectangle rect;

        if (flags & BUTTON_WIDTH_CONSTANT) {
            rect.width = max_text_length_in_pixels / 2;
        } else /*BUTTON_TEXT_RELATIVE*/ {
            rect.width = text_length_in_pixels;
        }

        if (flags & BUTTON_HEIGHT_CONSTANT) {
            rect.height = max_text_max_glyph_height_in_pixels;
        } else /*BUTTON_HEIGHT_TEXT_RELATIVE*/ {
            rect.height = text_max_glyph_height_in_pixels;
        }

        if (button == 0) {
            rect.y = y;
            if (flags & COLUMN_ALIGN_CENTER_HEIGHT) {
                rect.y += amount/2 * (rect.height + button_gap);
            }
        } else {
            rect.y = return_array[button-1].y - return_array[button-1].height*2 - button_gap;
        }

        x = original_x;
        rect.x = x;
        return_array[button] = rect;
        y = rect.y;

        // Draw button.
        {
            glUseProgram(rect_shader);
            glBindVertexArray(rect_vao);

            float button_vertices[6][2] {
                { rect.x - rect.width, rect.y + rect.height },
                { rect.x + rect.width, rect.y + rect.height },
                { rect.x - rect.width, rect.y - rect.height },

                { rect.x + rect.width, rect.y + rect.height },
                { rect.x + rect.width, rect.y - rect.height },
                { rect.x - rect.width, rect.y - rect.height }
            };

            glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(button_vertices), button_vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Draw text.
        glUseProgram(glyphs_shader);
        glBindVertexArray(font->vao);
        glActiveTexture(GL_TEXTURE0);

        if (flags & TEXT_ALIGN_CENTER_WIDTH) {
            x -= text_length_in_pixels / 2;
        }

        if (flags & TEXT_ALIGN_CENTER_HEIGHT) {
            // y -= text_max_glyph_height_in_pixels / 2;
            y -= rect.height / 3;
        }

        For (text_size) {
            glyph = font->glyphs[button_text[it]];

            float xpos = x + glyph.bearing.x * scale;
            float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;

            float w = glyph.size.x * scale;
            float h = glyph.size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, glyph.texture_id);

            glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (glyph.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64).
        }
    }
}

Rectangle draw_button(Font *font, const char *text, float x, float y, float scale, Vec3f text_color, Vec3f button_color, u16 flags /*= 0*/) {
    ZoneScoped;
    
    Glyph glyph;
    unsigned int text_length_in_pixels = 0;
    unsigned int text_max_glyph_height_in_pixels = 0;
    int text_size = string_length(text);
    For (text_size) {
        glyph = font->glyphs[text[it]];
        text_length_in_pixels += (glyph.advance >> 6) * scale;
        if (glyph.size.y > text_max_glyph_height_in_pixels) {
            text_max_glyph_height_in_pixels = glyph.size.y;
        }
    }

    Rectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = text_length_in_pixels;
    rect.height = text_max_glyph_height_in_pixels;

    if (flags & TEXT_ALIGN_CENTER_WIDTH) {
        x -= text_length_in_pixels / 2;
    }

    if (flags & TEXT_ALIGN_CENTER_HEIGHT) {
        y -= text_max_glyph_height_in_pixels / 2;
    }

    // Draw button.
    {
        glUseProgram(rect_shader);
        glUniformMatrix4fv(rect_projection_location, 1, false, &text_projection[0][0]);
        glUniform3fv(rect_color_location, 1, &button_color.x);
        glBindVertexArray(rect_vao);

        float button_vertices[6][2] {
            { rect.x - rect.width, rect.y + rect.height },
            { rect.x + rect.width, rect.y + rect.height },
            { rect.x - rect.width, rect.y - rect.height },

            { rect.x + rect.width, rect.y + rect.height },
            { rect.x + rect.width, rect.y - rect.height },
            { rect.x - rect.width, rect.y - rect.height }
        };

        glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(button_vertices), button_vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Draw text.
    //
    // @Copy from 'draw_text()'.
    // Activate corresponding render state.
    glUseProgram(glyphs_shader);
    glUniformMatrix4fv(text_projection_location, 1, false, &text_projection[0][0]);
    glUniform3fv(text_color_location, 1, &text_color.x);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->vao);

    // Iterate through all characters in text.
    For (text_size) {
        glyph = font->glyphs[text[it]];

	float xpos = x + glyph.bearing.x * scale;
	float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;

	float w = glyph.size.x * scale;
	float h = glyph.size.y * scale;

	// Update VBO for each character.
	float vertices[6][4] = {
	    { xpos,     ypos + h,   0.0f, 0.0f },            
	    { xpos,     ypos,       0.0f, 1.0f },
	    { xpos + w, ypos,       1.0f, 1.0f },

	    { xpos,     ypos + h,   0.0f, 0.0f },
	    { xpos + w, ypos,       1.0f, 1.0f },
	    { xpos + w, ypos + h,   1.0f, 0.0f }           
	};

	// Render glyph texture over quad.
	glBindTexture(GL_TEXTURE_2D, glyph.texture_id);

	// Update content of VBO memory.
	glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

	// Render quad.
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Now advance cursors for next glyph (note that advance is number of 1/64 pixels).
	x += (glyph.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64).
    }

    return rect;
}

/*inline*/ 
int string_length(const char *text) {
    int i = 0;
    while (text[i] != '\0') { i++; }
    return i;
}
