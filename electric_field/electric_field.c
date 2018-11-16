#define GLEW_STATIC
#define WIDTH 512
#define HEIGHT 512
#define MAXCOUNT 10
#define THRESHOLD 0.1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLchar* readShaderSource(char* path);

GLuint chargeCount = 0;
GLfloat chargePositions[MAXCOUNT * 2];

GLFWcursor* crossCursor;
GLFWcursor* regularCursor;

char isNear(double x1, double y1, double x2, double y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < THRESHOLD * THRESHOLD;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_PRESS) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        xPos = (xPos / WIDTH - 0.5) * 2;
        yPos = - (yPos / HEIGHT - 0.5) * 2;
        char isNearAny = 0;
        for (size_t i = 0; i < chargeCount; i++) {
            if (isNear(xPos, yPos, chargePositions[i * 2], chargePositions[i * 2 + 1])){
                isNearAny = 1;
            }
        }
        if (!isNearAny) {
            chargePositions[chargeCount * 2] = xPos;
            chargePositions[chargeCount * 2 + 1] = yPos;
            chargeCount++;
        }
    }
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    double xPos = (xpos / WIDTH - 0.5) * 2;
    double yPos = - (ypos / HEIGHT - 0.5) * 2;
    char isNearAny = 0;
    for (size_t i = 0; i < chargeCount; i++) {
        if (isNear(xPos, yPos, chargePositions[i * 2], chargePositions[i * 2 + 1])){
            isNearAny = 1;
        }
    }
    glfwSetCursor(window, isNearAny ? crossCursor : regularCursor);
}

int main(void)
{
    glfwInit();
    GLFWwindow* window;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Electric Field", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    crossCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    regularCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    GLuint potentialFieldvbo;
    glGenBuffers(1, &potentialFieldvbo);
    GLfloat vertices[] = {
        -1.,  1.,
         1.,  1.,
         1., -1.,
        -1., -1.
    };
    glBindBuffer(GL_ARRAY_BUFFER, potentialFieldvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint potentialFieldibo;
    glGenBuffers(1, &potentialFieldibo);
    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, potentialFieldibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLchar* vertexSource = readShaderSource("vertex_shader.vert");
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    free((void*)vertexSource);

    const GLchar* fragmentSource = readShaderSource("potential_field_fragmenet_shader.frag");
    GLuint potentialFieldFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(potentialFieldFragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(potentialFieldFragmentShader);
    free((void*)fragmentSource);

    GLuint potentialFieldShaderProgram = glCreateProgram();
    glAttachShader(potentialFieldShaderProgram, vertexShader);
    glAttachShader(potentialFieldShaderProgram, potentialFieldFragmentShader);
    glLinkProgram(potentialFieldShaderProgram);
    glUseProgram(potentialFieldShaderProgram);

    GLint posAttrib = glGetAttribLocation(potentialFieldShaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint chargesLocation = glGetUniformLocation(potentialFieldShaderProgram, "charges");
    GLint chargeCountLocation = glGetUniformLocation(potentialFieldShaderProgram, "chargeCount");

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform2fv(chargesLocation, chargeCount, chargePositions);
        glUniform1i(chargeCountLocation, chargeCount);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glDeleteProgram(potentialFieldShaderProgram);
    glDeleteShader(potentialFieldFragmentShader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &potentialFieldvbo);

    glfwTerminate();
    return 0;
}

GLchar* readShaderSource(char* path) {
    long length;
    FILE* f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    GLchar* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    fclose (f);
    buffer[length] = '\0';
    return buffer;
}
