#include <GL/freeglut.h>
#include <vector>
#include <string>

// math utilities
float cubic(float p0, float p1, float p2, float p3, float t) {
    return 0.0f; // placeholder
}

// parsing svg
void parseSvgPath() {
    // Math to compute shell border points
}

void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Shell Logo");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
