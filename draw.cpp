#include "common.h"

void renderString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string);
        string++;
    }
}

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    if (gameState == 0) {
        // スタート画面の背景
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(800, 0); glVertex2f(800, 600); glVertex2f(0, 600);
        glEnd();
        glDisable(GL_BLEND);

        // タイトルロゴ
        glColor3f(1.0f, 0.5f, 0.0f);
        renderString(220, 500, GLUT_BITMAP_TIMES_ROMAN_24, "1 on 1 STREET BASKETBALL");
        glColor3f(1.0f, 1.0f, 1.0f);
        renderString(300, 460, GLUT_BITMAP_HELVETICA_18, "First to 7 Points Wins!");

        // 操作説明を追加 
        glColor3f(0.0f, 0.8f, 1.0f); // 水色
        renderString(335, 390, GLUT_BITMAP_HELVETICA_18, "- CONTROLS -");

        glColor3f(0.9f, 0.9f, 0.9f); // グレー
        renderString(230, 340, GLUT_BITMAP_HELVETICA_18, "[ Arrow Keys ] : Move");
        renderString(230, 310, GLUT_BITMAP_HELVETICA_18, "[ Space ] : Jump Shoot / Block (Defense)");
        renderString(230, 280, GLUT_BITMAP_HELVETICA_18, "[ V ] : Layup (Near hoop)");
        renderString(230, 250, GLUT_BITMAP_HELVETICA_18, "[ C ] : Crossover / Ankle Break");
        renderString(230, 220, GLUT_BITMAP_HELVETICA_18, "[ F ] (Hold) : Dash");
        renderString(230, 160, GLUT_BITMAP_HELVETICA_18, "[ S ] : Steal (Defense)");

        // 変更：スタートボタンの案内を難易度選択に変更
        glColor3f(1.0f, 1.0f, 0.0f); // 黄色
        renderString(260, 120, GLUT_BITMAP_TIMES_ROMAN_24, "Select Difficulty to Start :");

        glColor3f(0.0f, 1.0f, 0.0f); // 緑色
        renderString(180, 80, GLUT_BITMAP_HELVETICA_18, "[ 1 ] EASY");
        glColor3f(1.0f, 1.0f, 1.0f); // 白色
        renderString(350, 80, GLUT_BITMAP_HELVETICA_18, "[ 2 ] NORMAL");
        glColor3f(1.0f, 0.0f, 0.0f); // 赤色
        renderString(530, 80, GLUT_BITMAP_HELVETICA_18, "[ 3 ] HARD");
}
    else if (gameState == 2) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(800, 0); glVertex2f(800, 600); glVertex2f(0, 600);
        glEnd();
        glDisable(GL_BLEND);

        if (playerWon) {
            glColor3f(0.0f, 1.0f, 0.0f);
            renderString(320, 350, GLUT_BITMAP_TIMES_ROMAN_24, "YOU WIN !!!");
        }
        else {
            glColor3f(1.0f, 0.0f, 0.0f);
            renderString(320, 350, GLUT_BITMAP_TIMES_ROMAN_24, "YOU LOSE...");
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        renderString(260, 280, GLUT_BITMAP_HELVETICA_18, "Press [ ENTER ] to Return to Title");
    }
    else {
        char scoreText[64];
        sprintf_s(scoreText, "YOU: %d   -   NPC: %d", playerScore, npcScore);
        glColor3f(1.0f, 0.8f, 0.0f);
        renderString(20, 560, GLUT_BITMAP_TIMES_ROMAN_24, scoreText);


        if (mustClear && ballOwner == 1) {
            glColor3f(1.0f, 0.2f, 0.2f);
            renderString(230, 450, GLUT_BITMAP_TIMES_ROMAN_24, "CLEAR THE BALL! (Go outside 3PT Line)");
        }
        if (stealMessageTimer > 0) {
            if (lastStealByPlayer) {
                glColor3f(0.0f, 1.0f, 0.0f); // 緑色で強調
                renderString(360, 480, GLUT_BITMAP_TIMES_ROMAN_24, "STEAL !!");
            }
            else {
                glColor3f(1.0f, 0.0f, 0.0f); // 赤色で警告
                renderString(350, 480, GLUT_BITMAP_TIMES_ROMAN_24, "STOLEN...");
            }
        }
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawCharacter(float x, float z, bool isShooting, bool isLayup, float animTime, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    float legSwing = sin(animTime) * 30.0f;
    float armSwing = sin(animTime) * 20.0f;

    glColor3f(r, g, b); // 胴体
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(0.6f, 1.2f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(1.0f, 0.8f, 0.6f); // 頭
    glPushMatrix();
    glTranslatef(0.0f, 1.9f, 0.0f);
    glutSolidSphere(0.25, 16, 16);
    glPopMatrix();

    glColor3f(r, g, b); // 左足
    glPushMatrix();
    glTranslatef(-0.15f, 0.6f, 0.0f);
    glRotatef(legSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.4f, 0.0f);
    glScalef(0.25f, 0.8f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix(); // 右足
    glTranslatef(0.15f, 0.6f, 0.0f);
    if (isLayup) glRotatef(-70.0f, 1.0f, 0.0f, 0.0f);
    else glRotatef(-legSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.4f, 0.0f);
    glScalef(0.25f, 0.8f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(1.0f, 0.8f, 0.6f); // 左腕
    glPushMatrix();
    glTranslatef(-0.35f, 1.4f, 0.0f);
    if (isShooting) glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    else if (isLayup) glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
    else glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.35f, 0.0f);
    glScalef(0.2f, 0.7f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix(); // 右腕
    glTranslatef(0.35f, 1.4f, 0.0f);
    if (isShooting || isLayup) {
        if (isShooting) glRotatef(-170.0f, 1.0f, 0.0f, 0.0f);
        else glRotatef(-150.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -0.5f, 0.0f);
        glScalef(0.2f, 1.0f, 0.2f);
    }
    else {
        glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -0.35f, 0.0f);
        glScalef(0.2f, 0.7f, 0.2f);
    }
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();
}

void drawFloor() {
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 0.0f, 10.0f); glVertex3f(10.0f, 0.0f, 10.0f);
    glVertex3f(10.0f, 0.0f, -10.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 180; i++) {
        float rad = i * 3.14159f / 180.0f;
        glVertex3f(HOOP_X + cos(rad) * THREE_POINT_LINE, 0.01f, HOOP_Z + sin(rad) * THREE_POINT_LINE);
    }
    glEnd();
}

void drawHoop() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(HOOP_X, HOOP_Y / 2.0f, HOOP_Z - 0.5f);
    glScalef(0.2f, HOOP_Y, 0.2f); glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(HOOP_X, HOOP_Y + 0.5f, HOOP_Z - 0.4f);
    glScalef(2.0f, 1.5f, 0.1f); glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(HOOP_X, HOOP_Y, HOOP_Z);
    glScalef(0.8f, 0.05f, 0.8f); glutWireCube(1.0);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f); // ネット
    glPushMatrix();
    glTranslatef(HOOP_X, HOOP_Y, HOOP_Z);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_LINE);
    gluCylinder(quad, 0.4f, 0.25f, 0.5f, 16, 5);
    gluDeleteQuadric(quad);
    glPopMatrix();
}