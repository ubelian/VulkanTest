#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec2 positions;
layout(location = 0) out vec3 fragColor;

layout(binding = 1, std430) buffer colores{
  float[] p_colores;
};

vec3 colors[] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
		vec3(1.0, 1.0, 1.0)
);

void main() {
    gl_Position = vec4(positions, 0.0, 1.0);

    fragColor = vec3(p_colores[0], p_colores[1], p_colores[2]);
}
