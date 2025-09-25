#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (length(coord) > 0.5) {
        discard;
    }
    FragColor = vec4(color, 1.0);
}