#include <GL/glut.h>
#include "game.h"
#include "audio.h"

Game game;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    game.draw();
    glFlush();
}

void update(int) {
    game.update();
    glutPostRedisplay();
    glutTimerFunc(30, update, 0);
}

void specialKeys(int key, int, int) {
    if (game.state == PLAYING) {
        if (key == GLUT_KEY_LEFT) game.player.moveLeft();
        if (key == GLUT_KEY_RIGHT) game.player.moveRight();
    }
}

void keyboard(unsigned char key, int, int) {

    if (game.state == MENU && key == 13) {
        game.state = PLAYING;
    }
    else if (game.state == GAMEOVER && key == 'r') {
        game = Game();
    }
    else if (game.state == PLAYING && key == ' ') {
        game.shoot();
        shootSound();
    }
}

void init() {
    glClearColor(0, 0, 0, 1);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Space Shooter Pro");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, update, 0);

    glutMainLoop();
}