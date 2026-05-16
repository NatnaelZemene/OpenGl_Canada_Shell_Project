#include <GL/freeglut.h>

void drawFlagBackground() {
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f,  0.5f);
        glVertex2f(-0.5f,  0.5f);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(-1.0f, -0.5f);

        glVertex2f( 0.5f,  0.5f);
        glVertex2f( 1.0f,  0.5f);
        glVertex2f( 1.0f, -0.5f);
        glVertex2f( 0.5f, -0.5f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.5f,  0.5f);
        glVertex2f( 0.5f,  0.5f);
        glVertex2f( 0.5f, -0.5f);
        glVertex2f(-0.5f, -0.5f);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawFlagBackground();
    glutSwapBuffers();
}

void reshape(int width, int height) {
    if (height <= 0) {
        height = 1;
    }

    const float targetAspect = 2.0f;
    const float currentAspect = static_cast<float>(width) / static_cast<float>(height);

    int vpX = 0;
    int vpY = 0;
    int vpW = width;
    int vpH = height;

    if (currentAspect > targetAspect) {
        vpW = static_cast<int>(height * targetAspect);
        vpX = (width - vpW) / 2;
    } else if (currentAspect < targetAspect) {
        vpH = static_cast<int>(width / targetAspect);
        vpY = (height - vpH) / 2;
    }

    glViewport(vpX, vpY, vpW, vpH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -0.5, 0.5, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920, 960);
    glutCreateWindow("National Flag of Canada - FreeGLUT");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}