#shader vertex
#version 330 core
layout (location = 0) in vec2 position;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    gl_Position = projection * model * vec4(position.xy, 0.0, 1.0);
}

#shader fragment
#version 330 core

uniform vec3 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0);
}