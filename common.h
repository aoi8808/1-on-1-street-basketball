#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <glut.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

// 定数
const float HOOP_X = 0.0f;
const float HOOP_Y = 3.0f;
const float HOOP_Z = -8.0f;
const float THREE_POINT_LINE = 7.0f;
const int ARM_DOWN_DELAY = 15;

// グローバル変数の宣言
extern int gameState;
extern bool playerWon;
extern int playerScore;
extern int npcScore;

extern int ballOwner;
extern int lastShooter;
extern bool mustClear;

extern float playerX, playerZ;
extern bool playerIsBlocking;
extern float playerY, playerVY;
extern float npcX, npcZ;
extern float ballX, ballY, ballZ;
extern float ballVX, ballVY, ballVZ;

extern bool playerInShootingPose;
extern bool isLayupPose;
extern int armDownTimer;
extern float walkAnim;
extern float dribbleAngle;
extern float currentDribbleX;
extern float targetDribbleX;

extern float npcWalkAnim;
extern bool npcIsBlocking;
extern bool npcInShootingPose;
extern int npcArmDownTimer;
extern float npcY, npcVY;
extern float npcDribbleAngle;
extern int npcStunTimer;

extern bool isMovingUp, isMovingDown, isMovingLeft, isMovingRight;
extern bool isDashing;
extern int currentShotPoints;

extern int stealMessageTimer;
extern bool lastStealByPlayer;

extern int difficulty; // 0: Easy, 1: Normal, 2: Hard

// 関数のプロトタイプ宣言
void drawHUD();
void drawCharacter(float x, float z, bool isShooting, bool isLayup, float animTime, float r, float g, float b);
void drawFloor();
void drawHoop();

void update(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void specialUpKeys(int key, int x, int y);