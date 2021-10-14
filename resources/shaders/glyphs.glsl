#shader vertex
#version 330 core
layout (location = 0) in vec4 vertices; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 text_projection;

void main()
{
    gl_Position = text_projection * vec4(vertices.xy, 0.0, 1.0);
    TexCoords = vertices.zw;
}

#shader fragment
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 text_color;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(text_color, 1.0) * sampled;
}