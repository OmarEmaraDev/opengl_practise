#define GLEW_STATIC
#define WIDTH 512
#define HEIGHT 512
#define MAXCOUNT 10
#define RADIUS 0.1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLchar* readShaderSource(char* path);

typedef struct {
    unsigned int count;
    GLfloat positions[MAXCOUNT * 2];
    GLfloat values[MAXCOUNT];
} Charges;
Charges charges = {.count = 0};

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    GLuint vbo;
    GLuint ibo;
} Quad;

typedef struct {
    char selected;
    int index;
    Point offset;
} Selection;
Selection selection;

typedef struct {
    Point location;
    int nearestCharge;
    int isAroundCharge;
    GLFWcursor* cross;
    GLFWcursor* regular;
} Cursor;
Cursor cursor;

static void cursorPosToDeviceCooridinates() {
    cursor.location.x = (cursor.location.x / WIDTH - 0.5) * 2;
    cursor.location.y = - (cursor.location.y / HEIGHT - 0.5) * 2;
}

int nearestCharge(double x, double y) {
    int chargeIndex = 0;
    double leastDistanceSquared = 4;
    double x2, y2, distanceSquared;
    for (size_t i = 0; i < charges.count; i++) {
        x2 = charges.positions[i * 2];
        y2 = charges.positions[i * 2 + 1];
        distanceSquared = (x - x2) * (x - x2) + (y - y2) * (y - y2);
        if ( distanceSquared < leastDistanceSquared){
            leastDistanceSquared = distanceSquared;
            chargeIndex = i;
        }
    }
    return chargeIndex;
}

char isAroundCharge(int index) {
    if (charges.count == 0) return 0;
    double dx = cursor.location.x - charges.positions[index * 2];
    double dy = cursor.location.y - charges.positions[index * 2 + 1];
    return dx * dx + dy * dy < RADIUS * RADIUS;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (!cursor.isAroundCharge) {
            charges.positions[charges.count * 2] = cursor.location.x;
            charges.positions[charges.count * 2 + 1] = cursor.location.y;
            charges.values[charges.count] = mods != GLFW_MOD_CONTROL ? 1 : -1;
            charges.count++;
        } else {
            selection.selected = 1;
            selection.index = cursor.nearestCharge;
            selection.offset.x = cursor.location.x - charges.positions[cursor.nearestCharge * 2];
            selection.offset.y = cursor.location.y - charges.positions[cursor.nearestCharge * 2 + 1];
        }
    } else {
        selection.selected = 0;
    }
}

static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos)
{
    cursor.location.x = xPos;
    cursor.location.y = yPos;
    cursorPosToDeviceCooridinates();
    cursor.nearestCharge = nearestCharge(cursor.location.x, cursor.location.y);
    cursor.isAroundCharge = isAroundCharge(cursor.nearestCharge);
    glfwSetCursor(window, cursor.isAroundCharge ? cursor.cross : cursor.regular);
    if (selection.selected) {
        charges.positions[selection.index * 2] = cursor.location.x - selection.offset.x;
        charges.positions[selection.index * 2 + 1] = cursor.location.y - selection.offset.y;
    }
}

GLuint getProgram(char* vertexShaderName, char* fragmentShaderName) {
    const GLchar* vertexSource = readShaderSource(vertexShaderName);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    const GLchar* fragmentSource = readShaderSource(fragmentShaderName);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free((void*)vertexSource);
    free((void*)fragmentSource);
    return shaderProgram;
}

GLuint getQuadVAO(GLuint shaderProgram, Quad* quad) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint attr = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(attr);
    glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->ibo);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return vao;
}

Quad loadQuad() {
    Quad quad;
    glGenBuffers(1, &quad.vbo);
    GLfloat vertices[] = {
        -1.,  1.,
         1.,  1.,
         1., -1.,
        -1., -1.
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &quad.ibo);
    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    return quad;
}

void drawPotentialField(Charges* charges, GLuint program, GLuint vao) {
    glUseProgram(program);
    glBindVertexArray(vao);
    glUniform4f(glGetUniformLocation(program, "transformation"), 1, 1, 0, 0);
    glUniform2fv(glGetUniformLocation(program, "positions"), charges->count, charges->positions);
    glUniform1fv(glGetUniformLocation(program, "values"), charges->count, charges->values);
    glUniform1i(glGetUniformLocation(program, "chargeCount"), charges->count);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void drawCharges(Charges* charges, GLuint program, GLuint vao) {
    glUseProgram(program);
    glBindVertexArray(vao);
    GLuint transformationLocation = glGetUniformLocation(program, "transformation");
    GLuint isNegativeLocation = glGetUniformLocation(program, "isNegative");
    for (size_t i = 0; i < charges->count; i++) {
        glUniform1i(isNegativeLocation, charges->values[i] < 0);
        glUniform4f(transformationLocation, RADIUS, RADIUS, charges->positions[i * 2], charges->positions[i * 2 + 1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

int main(void)
{
    glfwInit();
    GLFWwindow* window;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Electric Field", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    cursor.cross = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursor.regular = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Quad quad = loadQuad();
    GLuint fieldProgram = getProgram("vertex_shader.vert", "potential_field.frag");
    GLuint chargeProgram = getProgram("vertex_shader.vert", "charge.frag");
    GLuint fieldVAO = getQuadVAO(fieldProgram, &quad);
    GLuint chargeVAO = getQuadVAO(chargeProgram, &quad);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        drawPotentialField(&charges, fieldProgram, fieldVAO);
        drawCharges(&charges, chargeProgram, chargeVAO);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

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
