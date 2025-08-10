#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // преобразуем позицию в мировые координаты
    FragPos = vec3(model * vec4(aPos, 1.0));
    // стандартное преобразование в в Clip Space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    UV = aUV;
    // Передаем нормаль (учитываем поворот модели, но не масштабирование)
    Normal = mat3(transpose(inverse(model))) * aNormal;
}