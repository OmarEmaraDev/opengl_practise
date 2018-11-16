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

typedef struct {
    double x;
    double y;
} mousePos;
mousePos mPos;

static void mousePosToDeviceCooridinates() {
    mPos.x = (mPos.x / WIDTH - 0.5) * 2;
    mPos.y = - (mPos.y / HEIGHT - 0.5) * 2;
}

char isNearAny(double x, double y) {
    if (chargeCount == 0) return 0;
    char isNearAny = 0;
    for (size_t i = 0; i < chargeCount; i++) {
        double x2 = chargePositions[i * 2];
        double y2 = chargePositions[i * 2 + 1];
        if ((x - x2) * (x - x2) + (y - y2) * (y - y2) < THRESHOLD * THRESHOLD){
            isNearAny = 1;
        }
    }
    return isNearAny;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_PRESS) {
        glfwGetCursorPos(window, &mPos.x, &mPos.y);
        mousePosToDeviceCooridinates();
        if (!isNearAny(mPos.x, mPos.y)) {
            chargePositions[chargeCount * 2] = mPos.x;
            chargePositions[chargeCount * 2 + 1] = mPos.y;
            chargeCount++;
        }
    }
}

static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos)
{
    mPos.x = xPos; mPos.y = yPos;
    mousePosToDeviceCooridinates();
    glfwSetCursor(window, isNearAny(mPos.x, mPos.y) ? crossCursor : regularCursor);
}

typedef struct {
    GLuint chargesLocation;
    GLuint chargeCountLocation;
    GLuint shaderProgram;
    GLuint vbo;
    GLuint ibo;
} potentialField;
potentialField field;

void preparePotentialField() {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    GLfloat vertices[] = {
        -1.,  1.,
         1.,  1.,
         1., -1.,
        -1., -1.
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    field.vbo = vbo;

    GLuint ibo;
    glGenBuffers(1, &ibo);
    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    field.ibo = ibo;

    const GLchar* vertexSource = readShaderSource("vertex_shader.vert");
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    free((void*)vertexSource);

    const GLchar* fragmentSource = readShaderSource("potential_field_fragmenet_shader.frag");
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    free((void*)fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    field.shaderProgram = shaderProgram;

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    field.chargesLocation = glGetUniformLocation(shaderProgram, "charges");
    field.chargeCountLocation = glGetUniformLocation(shaderProgram, "chargeCount");
}

void drawPotentialField() {
    glUseProgram(field.shaderProgram);
    glUniform2fv(field.chargesLocation, chargeCount, chargePositions);
    glUniform1i(field.chargeCountLocation, chargeCount);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

    preparePotentialField();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        drawPotentialField();

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glDeleteProgram(field.shaderProgram);
    glDeleteBuffers(1, &field.vbo);
    glDeleteBuffers(1, &field.ibo);

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
