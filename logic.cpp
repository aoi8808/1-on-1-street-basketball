#include "common.h"

void update(int value) {
    // ゲーム中(gameState == 1)以外は画面更新のみ行い、物理演算などの処理を抜ける
    if (gameState != 1) {
        glutPostRedisplay(); glutTimerFunc(16, update, 0); return;
    }

    // 3x3ルールのクリアアウト判定である。ボール保持者が3Pラインの外に出たかを確認
    if (mustClear && ballOwner != 0) {
        float cx = (ballOwner == 1) ? playerX : npcX;
        float cz = (ballOwner == 1) ? playerZ : npcZ;
        float distFromHoop = sqrt((cx - HOOP_X) * (cx - HOOP_X) + (cz - HOOP_Z) * (cz - HOOP_Z));
        if (distFromHoop >= THREE_POINT_LINE) mustClear = false;
    }

    // Fキー押下時のダッシュ判定による移動速度の切り替え
    float speed = isDashing ? 0.11f : 0.06f;
    float oldPlayerX = playerX, oldPlayerZ = playerZ;
    bool isMoving = false;

    // 十字キーによるプレイヤーの移動処理
    if (isMovingUp) { playerZ -= speed; isMoving = true; }
    if (isMovingDown) { playerZ += speed; isMoving = true; }
    if (isMovingLeft) { playerX -= speed; isMoving = true; }
    if (isMovingRight) { playerX += speed; isMoving = true; }

    // プレイヤーとNPCのボディコンタクト（当たり判定）である。重ならないよう直前の座標に戻す
    if (sqrt((playerX - npcX) * (playerX - npcX) + (playerZ - npcZ) * (playerZ - npcZ)) < 0.8f) {
        playerX = oldPlayerX;
        playerZ = oldPlayerZ;
    }

    // 移動状態に応じた歩行アニメーションの進行処理
    if (isMoving) walkAnim += (isDashing ? 0.25f : 0.15f);
    else walkAnim = 0.0f;

    // プレイヤーのジャンプ（ブロック）の物理演算（重力による落下）
    if (playerIsBlocking) {
        playerY += playerVY;
        playerVY -= 0.01f;
        if (playerY <= 0.0f) { playerY = 0.0f; playerIsBlocking = false; }
    }

    // キャラクターの腕の動作（シュートやレイアップ後のフォロースルー）のタイマー処理
    if (armDownTimer > 0) armDownTimer--;
    else { playerInShootingPose = false; isLayupPose = false; }
    if (npcArmDownTimer > 0) npcArmDownTimer--;
    else npcInShootingPose = false;

    // フロントチェンジ時の、ボールが左右の手に移動するアニメーション処理
    if (currentDribbleX < targetDribbleX) {
        currentDribbleX += 0.08f;
        if (currentDribbleX > targetDribbleX) currentDribbleX = targetDribbleX;
    }
    else if (currentDribbleX > targetDribbleX) {
        currentDribbleX -= 0.08f;
        if (currentDribbleX < targetDribbleX) currentDribbleX = targetDribbleX;
    }

    // アンクルブレイク（フェイント）によるNPCの硬直時間を減算
    if (npcStunTimer > 0) npcStunTimer--;

	// スティールメッセージ表示のタイマー処理
    if (stealMessageTimer > 0) stealMessageTimer--;

    // NPCの移動およびAIロジック
    float oldNpcX = npcX, oldNpcZ = npcZ;
    float dxNPC = playerX - npcX;
    float dzNPC = playerZ - npcZ;
    float distNPC = sqrt(dxNPC * dxNPC + dzNPC * dzNPC);

    // 難易度Hardの時はスピードとアニメーション速度が1.8倍（ダッシュ）になる
    float npcSpd = (difficulty == 2) ? 1.8f : 1.0f;

    if (ballOwner == 0) {
        float dxToBallP = ballX - playerX;
        float dzToBallP = ballZ - playerZ;
        if (sqrt(dxToBallP * dxToBallP + dzToBallP * dzToBallP) < 1.5f && ballY < 1.0f) {
            if (lastShooter == 2) mustClear = true; // 敵のシュートだった場合はクリアアウト必須
            ballOwner = 1;
        }
    }

    if (ballOwner == 1) {
        // プレイヤーがボールを持っている場合（ディフェンス時）のNPCの設計
        if (npcStunTimer > 0) {
            npcWalkAnim = 0.0f; // 硬直中はアニメーションを止める
        }
        else {
            // NPCによる自動スティール判定
            // 距離が近く、かつランダム（約1.5%の確率/フレーム）で奪いに来る
            if (difficulty >= 1 && distNPC < 1.5f && rand() % 100 < 2) {
                ballOwner = 2;
                mustClear = true; // 攻守交替なのでクリアアウト必須
                stealMessageTimer = 60;
                lastStealByPlayer = false;
            }
            // プレイヤーがシュートモーションに入ったらブロックジャンプを行う
            if (playerInShootingPose && distNPC < 2.0f && !npcIsBlocking) {
                npcIsBlocking = true; npcVY = 0.12f;
            }
            if (npcIsBlocking) {
                npcY += npcVY; npcVY -= 0.01f;
                if (npcY <= 0.0f) { npcY = 0.0f; npcIsBlocking = false; }
            }
            else {
                // ブロック中でなければ、一定距離までプレイヤーに近づく処理
                if (distNPC > 1.2f) {
                    npcX += (dxNPC / distNPC) * 0.035f * npcSpd;
                    npcZ += (dzNPC / distNPC) * 0.035f * npcSpd;
                    npcWalkAnim += 0.15f;
                }
                else npcWalkAnim = 0.0f;
            }
        }
    }
    else if (ballOwner == 0) {
        // ボールがフリー（空宙または床にある）の場合、NPCがボールを拾いに行く処理
        float dxToBall = ballX - npcX;
        float dzToBall = ballZ - npcZ;
        float distToBall = sqrt(dxToBall * dxToBall + dzToBall * dzToBall);
        if (distToBall < 1.5f && ballY < 1.0f) {
            if (lastShooter == 1) mustClear = true; // プレイヤーのシュートに対するリバウンドならクリアアウトを必須
            ballOwner = 2;
        }
        else {
            npcX += (dxToBall / distToBall) * 0.045f * npcSpd;
            npcZ += (dzToBall / distToBall) * 0.045f * npcSpd;
            npcWalkAnim += 0.15f;
        }
    }
    else if (ballOwner == 2) {
        // NPCがボールを持っている場合（オフェンス時）の処理
        if (mustClear) {
            // クリアアウトするため、3Pライン外（トップ位置）へ移動
            float targetX = 0.0f, targetZ = 1.0f;
            float distToClear = sqrt((targetX - npcX) * (targetX - npcX) + (targetZ - npcZ) * (targetZ - npcZ));
            if (distToClear > 0.5f) {
                npcX += ((targetX - npcX) / distToClear) * 0.035f;
                npcZ += ((targetZ - npcZ) / distToClear) * 0.035f;
                npcWalkAnim += 0.15f;
            }
        }
        else {
            // ゴールへ向かい、ランダムなタイミングでシュートを放つロジック
            float dxGoal = HOOP_X - npcX;
            float dzGoal = HOOP_Z - npcZ;
            float distGoal = sqrt(dxGoal * dxGoal + dzGoal * dzGoal);

            if (distGoal > 5.5f) {
                npcX += (dxGoal / distGoal) * 0.035f * npcSpd;
                npcZ += (dzGoal / distGoal) * 0.035f * npcSpd;
                npcWalkAnim += 0.15f;
            }
            else {
                npcWalkAnim = 0.0f;
                if (rand() % 40 == 0 && !npcInShootingPose) {
                    ballOwner = 0;
                    lastShooter = 2;
                    npcInShootingPose = true;
                    npcArmDownTimer = ARM_DOWN_DELAY;

                    float startX = npcX, startY = 2.4f, startZ = npcZ - 0.1f;
                    ballX = startX; ballY = startY; ballZ = startZ;

                    float dx = HOOP_X - startX, dz = HOOP_Z - startZ;
                    float distance = sqrt(dx * dx + dz * dz);
                    if (distance < 0.1f) distance = 0.1f;

                    currentShotPoints = (distance >= THREE_POINT_LINE) ? 2 : 1;
                    float frames = distance * 6.0f;
                    if (frames < 30.0f) frames = 30.0f;

                    // プレイヤーからのプレッシャー（ブロックの有無等）によるシュート精度の変動計算
                    float errorFactor = distance * 0.0007f;
                    float currentDist = sqrt((playerX - npcX) * (playerX - npcX) + (playerZ - npcZ) * (playerZ - npcZ));
                    if (currentDist < 2.0f) {
                        if (playerIsBlocking) errorFactor *= 6.0f;
                        else errorFactor *= 3.0f;
                    }

                    ballVX = (dx / frames) + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * errorFactor;
                    ballVZ = (dz / frames) + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * errorFactor;
                    ballVY = (HOOP_Y - startY + 0.005f * (frames * (frames + 1)) / 2.0f) / frames;
                }
            }
        }
    }

    // NPC側のボディコンタクト判定（プレイヤーとの重なり防止処理）
    if (sqrt((npcX - playerX) * (npcX - playerX) + (npcZ - playerZ) * (npcZ - playerZ)) < 0.8f) {
        npcX = oldNpcX; npcZ = oldNpcZ;
    }

    // ボールがフリー（空宙または床にある）時の物理演算処理
    if (ballOwner == 0) {
        ballVY -= 0.005f; // 重力の加算
        ballX += ballVX; ballY += ballVY; ballZ += ballVZ;

        // バックボードとの当たり判定とバウンド処理
        float boardFrontZ = HOOP_Z - 0.35f, boardLeftX = HOOP_X - 1.0f, boardRightX = HOOP_X + 1.0f;
        float boardBottomY = HOOP_Y - 0.25f, boardTopY = HOOP_Y + 1.25f;
        if ((ballZ - 0.28f) <= boardFrontZ && ballVZ < 0.0f) {
            if (ballX > boardLeftX - 0.28f && ballX < boardRightX + 0.28f &&
                ballY > boardBottomY - 0.28f && ballY < boardTopY + 0.28f) {
                ballVZ = -ballVZ * 0.4f; ballVY = ballVY * 0.3f; ballZ = boardFrontZ + 0.28f;
            }
        }

        // リングのフチとの当たり判定
        float dx = ballX - HOOP_X, dz = ballZ - HOOP_Z;
        float distToCenter = sqrt(dx * dx + dz * dz), dy = ballY - HOOP_Y;

        if (fabs(dy) < 0.28f) {
            float distToRim = fabs(distToCenter - 0.5f);
            if (distToRim < 0.12f && distToCenter > 0.01f) {
                ballVY = fabs(ballVY) * 0.3f;
                ballVX += (dx / distToCenter) * 0.005f; ballVZ += (dz / distToCenter) * 0.005f;
            }
        }

        // ゴールに入ったかどうかの判定処理
        if (ballVY < 0 && ballY < HOOP_Y && ballY > HOOP_Y - 0.4f && distToCenter < 0.65f) {
            if (lastShooter == 1) playerScore += currentShotPoints;
            else npcScore += currentShotPoints;
            mustClear = true; // ゴール後は必ずクリアアウトを必須

            // どちらかのスコアが7点に達した場合、ゲーム終了（決着ステート）へ移行
            if (playerScore >= 7 || npcScore >= 7) {
                gameState = 2; playerWon = (playerScore >= 7);
            }
            ballY = HOOP_Y - 0.4f; ballVX = 0.0f; ballVZ = 0.0f;
        }

        // 床面とのバウンドおよび摩擦（減速）処理
        if (ballY <= 0.28f) {
            ballY = 0.28f;
            if (ballVY < -0.02f) ballVY = -ballVY * 0.6f; else ballVY = 0.0f;
            ballVX *= 0.9f; ballVZ *= 0.9f;
        }
    }
    // プレイヤーがボールを保持している時のドリブル位置（三角関数を用いたバウンド）の計算
    else if (ballOwner == 1) {
        dribbleAngle += 0.15f; if (dribbleAngle > 3.14159f) dribbleAngle -= 3.14159f;
        ballX = playerX + currentDribbleX; ballY = 0.28f + 0.5f * fabs(cos(dribbleAngle)); ballZ = playerZ - 0.4f;
    }
    // NPCがボールを保持している時のドリブル位置の計算
    else if (ballOwner == 2) {
        npcDribbleAngle += 0.15f; if (npcDribbleAngle > 3.14159f) npcDribbleAngle -= 3.14159f;
        ballX = npcX + 0.55f; ballY = 0.28f + 0.5f * fabs(cos(npcDribbleAngle)); ballZ = npcZ - 0.4f;
    }

    // キャラクターやボールがコート外に飛び出さないように制限する「見えない壁」の処理
    const float WALL_MAX = 9.6f, WALL_MIN = -9.6f;
    if (playerX > WALL_MAX) playerX = WALL_MAX; if (playerX < WALL_MIN) playerX = WALL_MIN;
    if (playerZ > WALL_MAX) playerZ = WALL_MAX; if (playerZ < WALL_MIN) playerZ = WALL_MIN;
    if (npcX > WALL_MAX) npcX = WALL_MAX; if (npcX < WALL_MIN) npcX = WALL_MIN;
    if (npcZ > WALL_MAX) npcZ = WALL_MAX; if (npcZ < WALL_MIN) npcZ = WALL_MIN;

    if (ballOwner == 0) {
        if (ballX > WALL_MAX) { ballX = WALL_MAX; ballVX = -ballVX * 0.7f; }
        if (ballX < WALL_MIN) { ballX = WALL_MIN; ballVX = -ballVX * 0.7f; }
        if (ballZ > WALL_MAX) { ballZ = WALL_MAX; ballVZ = -ballVZ * 0.7f; }
        if (ballZ < WALL_MIN) { ballZ = WALL_MIN; ballVZ = -ballVZ * 0.7f; }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// 矢印キー押下時の移動フラグ切り替え処理
void specialKeys(int key, int x, int y) {
    if (gameState != 1) return;
    if (key == GLUT_KEY_UP)   isMovingUp = true;
    if (key == GLUT_KEY_DOWN) isMovingDown = true;
    if (key == GLUT_KEY_LEFT) isMovingLeft = true;
    if (key == GLUT_KEY_RIGHT) isMovingRight = true;
}

// 矢印キー解放時の移動フラグ切り替え処理
void specialUpKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP)   isMovingUp = false;
    if (key == GLUT_KEY_DOWN) isMovingDown = false;
    if (key == GLUT_KEY_LEFT) isMovingLeft = false;
    if (key == GLUT_KEY_RIGHT) isMovingRight = false;
}

// 通常キー解放時の処理である（ここではダッシュの解除を行う）
void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'f' || key == 'F') {
        isDashing = false;
    }
}

// 通常キー押下時の各種アクション処理
void keyboard(unsigned char key, int x, int y) {
    // Escキーでゲームを終了
    if (key == 27) std::exit(0);

    // タイトルまたは終了画面でEnterキーが押された際、変数を初期化してゲームを開始する
    if (gameState != 1) {
        
            if (key == '1' || key == '2' || key == '3' || key == 13) {
                if (key == '1') difficulty = 0; // Easy
                else if (key == '2' || key == 13) difficulty = 1; // Normal (EnterキーでもNormal)
                else if (key == '3') difficulty = 2; // Hard
            gameState = 1;
            playerScore = 0; npcScore = 0;
            playerX = 0.0f; playerZ = 5.0f; npcX = 0.0f; npcZ = 2.0f;
            ballOwner = 1; mustClear = false;
            isMovingUp = false; isMovingDown = false; isMovingLeft = false; isMovingRight = false;
            isDashing = false;
        }
        return;
    }

    // 現在の攻守状態を判定する
    bool isOffense = (ballOwner == 1);
    bool isDefense = (ballOwner != 1);


    // Cキー：フロントチェンジおよびアンクルブレイク（NPC硬直）の判定処理
    if (key == 'c' || key == 'C') {
        if (isOffense) {
            targetDribbleX = -targetDribbleX;
            float distToNPC = sqrt((playerX - npcX) * (playerX - npcX) + (playerZ - npcZ) * (playerZ - npcZ));
            // NPCが一定距離内にいる場合、50%の確率でNPCの動きを止める
            if (distToNPC < 2.5f && npcStunTimer == 0) {
                if (rand() % 2 == 0) {
                    npcStunTimer = 40;
                    std::cout << "Ankle Break!! (NPC Stunned)" << std::endl;
                }
            }
        }
    }

    // Fキー：ダッシュのフラグを有効化する
    if (key == 'f' || key == 'F') {
        isDashing = true;
    }

    if ((key == 's' || key == 'S') && isDefense && ballOwner == 2) {
        float distToNPC = sqrt((playerX - npcX) * (playerX - npcX) + (playerZ - npcZ) * (playerZ - npcZ));
        if (distToNPC < 1.5f) { // NPCとの距離が近い場合
            if (rand() % 100 < 40) { // 40%の確率でスティール成功！
                ballOwner = 1; // 自分のボールになる
                mustClear = true; // 攻守交替なのでクリアアウト必須
                npcStunTimer = 40; // 奪われたNPCは少し硬直する
                stealMessageTimer = 60;
                lastStealByPlayer = true;
            }
        }
    }

    // スペースキー：ジャンプシュートまたはブロックジャンプの実行
    if (key == ' ') {
        if (isOffense) {
            // オフェンス時：クリアアウト要件を満たしていない場合はシュートを制限
            if (mustClear) {
                std::cout << "3Pラインの外に出てください！" << std::endl;
            }
            else {
                ballOwner = 0; lastShooter = 1;
                playerInShootingPose = true; armDownTimer = ARM_DOWN_DELAY;

                float startX = playerX, startY = 2.4f, startZ = playerZ - 0.1f;
                ballX = startX; ballY = startY; ballZ = startZ;

                // シュートの軌道とゴールまでの距離を計算
                float dx = HOOP_X - startX, dz = HOOP_Z - startZ;
                float distance = sqrt(dx * dx + dz * dz);
                if (distance < 0.1f) distance = 0.1f;

                // 距離に応じた得点（1点か2点か）の設定
                currentShotPoints = (distance >= THREE_POINT_LINE) ? 2 : 1;
                float frames = distance * 6.0f; if (frames < 30.0f) frames = 30.0f;

                // 敵のプレッシャーに応じたシュート精度の計算処理
                float errorFactor = distance * 0.0001f;
                float distToNPC = sqrt((playerX - npcX) * (playerX - npcX) + (playerZ - npcZ) * (playerZ - npcZ));
                if (distToNPC < 2.0f) {
                    if (npcIsBlocking) errorFactor *= 5.0f;
                    else errorFactor *= 2.5f;
                }

                // ボールの初速（ベクトル）を、乱数によるブレを含めて決定
                ballVX = (dx / frames) + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * errorFactor;
                ballVZ = (dz / frames) + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * errorFactor;
                ballVY = (HOOP_Y - startY + 0.005f * (frames * (frames + 1)) / 2.0f) / frames;
            }
        }
        else if (isDefense) {
            // ディフェンス時：敵のシュートを妨害するブロックジャンプを行う処理
            if (!playerIsBlocking) { playerIsBlocking = true; playerVY = 0.12f; }
        }
    }

    // Vキー：レイアップシュートの実行
    if ((key == 'v' || key == 'V') && isOffense) {
        // オフェンス時：クリアアウト要件を満たしていない場合はシュートを制限
        if (mustClear) {
            std::cout << "3Pラインの外に出てください！" << std::endl;
        }
        else {
            ballOwner = 0; lastShooter = 1;
            isLayupPose = true; armDownTimer = 25;

            float startX = playerX + 0.35f, startY = 2.4f, startZ = playerZ - 0.4f;
            ballX = startX; ballY = startY; ballZ = startZ;

            // ジャンプシュートよりも打点が低く、直接ゴールを狙う軌道設定
            float targetX = HOOP_X, targetY = HOOP_Y + 0.4f, targetZ = HOOP_Z - 0.35f;
            float frames = 22.0f;
            currentShotPoints = 2;

            ballVX = (targetX - startX) / frames;
            ballVZ = (targetZ - startZ) / frames;
            ballVY = (targetY - startY + 0.005f * (frames * (frames + 1)) / 2.0f) / frames;
        }
    }
}