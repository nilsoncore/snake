#ifndef SNAKE_MATH_H
#define SNAKE_MATH_H
#include <GLM/glm.hpp>

// glm::perspective, glm::lookAt
#include <GLM/gtc/matrix_transform.hpp>

//
// --- Structs ---
//
struct Vec2f {
    float x;
    float y;
};

// XYZ or RGB
struct Vec3f {
    union {
	float x;
	float r;
    };
    union {
	float y;
	float g;
    };
    union {
	float z;
	float b;
    };
};

// XYZW or RGBA
struct Vec4f {
    union {
	float x;
	float r;
    };
    union {
	float y;
	float g;
    };
    union {
	float z;
	float b;
    };
    union {
	float w;
	float a;
    };
};

struct Vec2i {
    int x;
    int y;
};

// XYZ or RGB
struct Vec3i {
    union {
	int x;
	int r;
    };
    union {
	int y;
	int g;
    };
    union {
	int z;
	int b;
    };
};

// XYZW or RGBA
struct Vec4i {
    union {
	int x;
	int r;
    };
    union {
	int y;
	int g;
    };
    union {
	int z;
	int b;
    };
    union {
	int w;
	int a;
    };
};

//
// --- Constants ---
//
const float COMPARE_FLOAT_PRECISION = 0.1f;

//
// --- Declarations ---
//
/*inline*/ Vec2f new_vec2f(float x, float y);
/*inline*/ Vec2f new_vec2f(float xy);
/*inline*/ Vec3f new_vec3f(float x, float y, float z);
/*inline*/ Vec3f new_vec3f(float xyz);
/*inline*/ Vec4f new_vec4f(float x, float y, float z, float w);
/*inline*/ Vec4f new_vec4f(float xyzw);
/*inline*/ Vec2i new_vec2i(int x, int y);
/*inline*/ Vec2i new_vec2i(int xy);
/*inline*/ Vec3i new_vec3i(int x, int y, int z);
/*inline*/ Vec3i new_vec3i(int xyz);
/*inline*/ Vec4i new_vec4i(int x, int y, int z, int w);
/*inline*/ Vec4i new_vec4i(int xyzw);
/*inline*/ bool floats_equal(float a, float b);
/*inline*/ bool vec2f_equal(Vec2f a, Vec2f b);
/*inline*/ bool vec3f_equal(Vec3f a, Vec3f b);
/*inline*/ bool vec4f_equal(Vec4f a, Vec4f b);
/*inline*/ bool vec2i_equal(Vec2i a, Vec2i b);
/*inline*/ bool vec3i_equal(Vec3i a, Vec3i b);
/*inline*/ bool vec4i_equal(Vec4i a, Vec4i b);
/*inline*/ Vec3f vec3f_from_vec4f(Vec4f vec4f);
/*inline*/ Vec4f vec4f_from_vec3f(Vec3f vec3f);

//
// --- Implementations ---
//
inline Vec2f new_vec2f(float x, float y) {
    Vec2f result = { x, y };
    return result;
}

inline Vec2f new_vec2f(float xy) {
    Vec2f result = { xy, xy };
    return result;
}

inline Vec3f new_vec3f(float x, float y, float z) {
    Vec3f result = { x, y, z };
    return result;
}

inline Vec3f new_vec3f(float xyz) {
    Vec3f result = { xyz, xyz, xyz };
    return result;
}

inline Vec4f new_vec4f(float x, float y, float z, float w) {
    Vec4f result = { x, y, z, w };
    return result;
}

inline Vec4f new_vec4f(float xyzw) {
    Vec4f result = { xyzw, xyzw, xyzw, xyzw };
    return result;
}

inline Vec2i new_vec2i(int x, int y) {
    Vec2i result = { x, y };
    return result;
}

inline Vec2i new_vec2i(int xy) {
    Vec2i result = { xy, xy };
    return result;
}

inline Vec3i new_vec3i(int x, int y, int z) {
    Vec3i result = { x, y, z };
    return result;
}

inline Vec3i new_vec3i(int xyz) {
    Vec3i result = { xyz, xyz, xyz };
    return result;
}

inline Vec4i new_vec4i(int x, int y, int z, int w) {
    Vec4i result = { x, y, z, w };
    return result;
}

inline Vec4i new_vec4i(int xyzw) {
    Vec4i result = { xyzw, xyzw, xyzw, xyzw };
    return result;
}

inline bool floats_equal(float a, float b) {
    return fabs(a-b) < COMPARE_FLOAT_PRECISION;
}

inline bool vec2f_equal(Vec2f a, Vec2f b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION);
}

inline bool vec3f_equal(Vec3f a, Vec3f b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.z - b.z) < COMPARE_FLOAT_PRECISION);
}

inline bool vec4f_equal(Vec4f a, Vec4f b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.z - b.z) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.w - b.w) < COMPARE_FLOAT_PRECISION);
}

inline bool vec2i_equal(Vec2i a, Vec2i b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION);
}

inline bool vec3i_equal(Vec3i a, Vec3i b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.z - b.z) < COMPARE_FLOAT_PRECISION);
}

inline bool vec4i_equal(Vec4i a, Vec4i b) {
    return (fabs(a.x - b.x) < COMPARE_FLOAT_PRECISION) 
	&& (fabs(a.y - b.y) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.z - b.z) < COMPARE_FLOAT_PRECISION)
	&& (fabs(a.w - b.w) < COMPARE_FLOAT_PRECISION);
}

inline Vec3f vec3f_from_vec4f(Vec4f vec4f) {
    Vec3f result;
    result.x = vec4f.x;
    result.y = vec4f.y;
    result.z = vec4f.z;
    return result;
}

inline Vec4f vec4f_from_vec3f(Vec3f vec3f) {
    Vec4f result;
    result.x = vec3f.x;
    result.y = vec3f.y;
    result.z = vec3f.z;
    result.w = 0.0f;
}

#endif /* SNAKE_MATH_H */
