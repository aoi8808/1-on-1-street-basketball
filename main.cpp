#include "common.h"

// 変数の実体
int gameState = 0;
bool playerWon = false;
int playerScore = 0;
int npcScore = 0;

int ballOwner = 1;
int lastShooter = 1;
bool mustClear = false;

float playerX = 0.0f, playerZ = 5.0f;
bool playerIsBlocking = false;
float playerY = 0.0f, playerVY = 0.0f;
float npcX = 0.0f, npcZ = 2.0f;
float ballX = 0.0f, ballY = 1.0f, ballZ = 5.0f;
float ballVX = 0.0f, ballVY = 0.0f, ballVZ = 0.0f;

bool playerInShootingPose = false;
bool isLayupPose = false;
int armDownTimer = 0;
float walkAnim = 0.0f;
float dribbleAngle = 0.0f;
float currentDribbleX = 0.55f;
float targetDribbleX = 0.55f;

float npcWalkAnim = 0.0f;
bool npcIsBlocking = false;
bool npcInShootingPose = false;
int npcArmDownTimer = 0;
float npcY = 0.0f, npcVY = 0.0f;
float npcDribbleAngle = 0.0f;
int npcStunTimer = 0;

bool isMovingUp = false, isMovingDown = false, isMovingLeft = false, isMovingRight = false;
bool isDashing = false;
int currentShotPoints = 0;

int stealMessageTimer = 0;
bool lastStealByPlayer = false;

int difficulty = 1; // デフォルトはNormal(1)

// メイン描画関数
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // カメラ位置
    gluLookAt(0.0, 6.0, 14.0, 0.0, 0.0, -6.0, 0.0, 1.0, 0.0);

    drawFloor();
    drawHoop();

    // 自分
    glPushMatrix();
    glTranslatef(0.0f, playerY, 0.0f);
    drawCharacter(playerX, playerZ, playerInShootingPose || playerIsBlocking, isLayupPose, walkAnim, 0.1f, 0.3f, 0.8f);
    glPopMatrix();

    // 敵
    glPushMatrix();
    glTranslatef(0.0f, npcY, 0.0f);
    drawCharacter(npcX, npcZ, npcIsBlocking || npcInShootingPose, false, npcWalkAnim, 0.8f, 0.2f, 0.2f);
    glPopMatrix();

    // ボール
    glColor3f(1.0f, 0.5f, 0.0f);
    glPushMatrix();
    glTranslatef(ballX, ballY, ballZ);
    glutSolidSphere(0.28, 16, 16);
    glPopMatrix();

    drawHUD();
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("1on1 3x3 Street Basketball - Modular");

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialUpKeys);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}