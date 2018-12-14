#version 330 core
out vec4 color;
in vec3 vertex_color;
uniform int type;

void main()
{
    if (type == 0) { //player
        color = vec4(1, 1, 1, 1);
    }
    else if (type == 1) { //ground
        color = vec4(0.2, 0.4, 0.2, 1);
    }
    else if (type == 2) { //obstacle
        color = vec4(vertex_color, 1);
    }
    else if (type == 3) { //wind
        color = vec4(0.8, 0.8, 0.8, 0.2);
    }
    else if (type == 4) { //goal
        color = vec4(1, 0.8, 0, 1);
    }
    else {
        color = vec4(vertex_color, 1);
    }
}
