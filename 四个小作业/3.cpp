#include <graphics.h>
#include <conio.h>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const int PADDLE_WIDTH = 120;
const int PADDLE_HEIGHT = 20;
const int BALL_RADIUS = 15;
const int BRICK_WIDTH = 70;
const int BRICK_HEIGHT = 30;
const int BRICK_ROWS = 5;
const int BRICK_COLS = 10;

// 游戏状态枚举
enum GameState { START, PLAYING, PAUSED, GAME_OVER, WIN };

// 球类
class Ball {
public:
    float x, y;
    float dx, dy;
    COLORREF color;
    bool sticky; // 是否粘在挡板上

    Ball() : x(WIDTH/2), y(HEIGHT/2), dx(4), dy(-4), color(RGB(255, 255, 255)), sticky(true) {}

    void reset() {
        x = WIDTH/2;
        y = HEIGHT/2;
        dx = 4;
        dy = -4;
        sticky = true;
    }

    void update() {
        if (!sticky) {
            x += dx;
            y += dy;
            
            // 左右边界碰撞
            if (x - BALL_RADIUS < 0 || x + BALL_RADIUS > WIDTH) {
                dx = -dx;
                x = (x - BALL_RADIUS < 0) ? BALL_RADIUS : WIDTH - BALL_RADIUS;
            }
            
            // 上边界碰撞
            if (y - BALL_RADIUS < 0) {
                dy = -dy;
                y = BALL_RADIUS;
            }
        }
    }

    void draw() {
        setfillcolor(color);
        solidcircle(x, y, BALL_RADIUS);
        
        // 添加高光效果
        setfillcolor(RGB(255, 255, 255));
        solidcircle(x - BALL_RADIUS/3, y - BALL_RADIUS/3, BALL_RADIUS/3);
    }
};

// 挡板类
class Paddle {
public:
    float x, y;
    float width, height;
    COLORREF color;
    float speed;

    Paddle() : x(WIDTH/2 - PADDLE_WIDTH/2), y(HEIGHT - 50), 
               width(PADDLE_WIDTH), height(PADDLE_HEIGHT), 
               color(RGB(0, 200, 255)), speed(8) {}

    void moveLeft() {
        if (x > 0) x -= speed;
    }

    void moveRight() {
        if (x + width < WIDTH) x += speed;
    }

    void draw() {
        // 绘制挡板主体
        setfillcolor(color);
        solidroundrect(x, y, x + width, y + height, 10, 10);
        
        // 绘制挡板边缘效果
        setlinecolor(RGB(0, 100, 200));
        setlinestyle(PS_SOLID, 3);
        roundrect(x, y, x + width, y + height, 10, 10);
    }
};

// 砖块类
class Brick {
public:
    float x, y;
    float width, height;
    COLORREF color;
    int health;
    bool active;
    int points;

    Brick(int col, int row, COLORREF c, int h = 1) 
        : x(col * BRICK_WIDTH + 20), y(row * BRICK_HEIGHT + 50),
          width(BRICK_WIDTH - 5), height(BRICK_HEIGHT - 5),
          color(c), health(h), active(true), points(h * 10) {}

    void draw() {
        if (!active) return;
        
        // 绘制砖块主体
        setfillcolor(color);
        solidrectangle(x, y, x + width, y + height);
        
        // 绘制砖块边框
        setlinecolor(RGB(0, 0, 0));
        setlinestyle(PS_SOLID, 2);
        rectangle(x, y, x + width, y + height);
        
        // 绘制砖块内部效果
        setfillcolor(RGB(255, 255, 255));
        solidrectangle(x + 2, y + 2, x + width - 2, y + height/2);
    }

    bool isHit(float ballX, float ballY, float ballRadius) {
        if (!active) return false;
        
        // 计算球与砖块的最近点
        float closestX = max(x, min(ballX, x + width));
        float closestY = max(y, min(ballY, y + height));
        
        // 计算距离
        float distanceX = ballX - closestX;
        float distanceY = ballY - closestY;
        
        // 检查碰撞
        return (distanceX * distanceX + distanceY * distanceY) <= (ballRadius * ballRadius);
    }
};

// 粒子效果类
class Particle {
public:
    float x, y;
    float dx, dy;
    COLORREF color;
    int life;
    int size;

    Particle(float px, float py, COLORREF c) 
        : x(px), y(py), color(c), life(30) {
        // 随机方向
        float angle = (rand() % 360) * 3.14159 / 180.0;
        float speed = (rand() % 5) / 2.0 + 1.0;
        dx = cos(angle) * speed;
        dy = sin(angle) * speed;
        size = rand() % 3 + 1;
    }

    void update() {
        x += dx;
        y += dy;
        life--;
    }

    void draw() {
        if (life <= 0) return;
        
        setfillcolor(color);
        solidcircle(x, y, size);
    }
};

// 游戏类
class BreakoutGame {
public:
    GameState state;
    int score;
    int lives;
    int level;
    Ball ball;
    Paddle paddle;
    vector<Brick> bricks;
    vector<Particle> particles;
    time_t startTime;
    bool specialPower; // 特殊能力

    BreakoutGame() : state(START), score(0), lives(3), level(1), specialPower(false) {
        srand(time(0));
        resetGame();
    }

    void resetGame() {
        ball.reset();
        paddle = Paddle();
        createBricks();
        lives = 3;
        score = 0;
        specialPower = false;
        startTime = time(0);
    }

    void createBricks() {
        bricks.clear();
        
        // 创建不同颜色和强度的砖块
        for (int row = 0; row < BRICK_ROWS; row++) {
            for (int col = 0; col < BRICK_COLS; col++) {
                COLORREF color;
                int health = 1;
                
                // 根据行数设置不同颜色和强度
                if (row == 0) {
                    color = RGB(255, 100, 100); // 红色
                    health = 3;
                } else if (row == 1) {
                    color = RGB(255, 200, 100); // 橙色
                    health = 2;
                } else if (row == 2) {
                    color = RGB(255, 255, 100); // 黄色
                } else if (row == 3) {
                    color = RGB(100, 255, 100); // 绿色
                } else {
                    color = RGB(100, 200, 255); // 蓝色
                }
                
                bricks.push_back(Brick(col, row, color, health));
            }
        }
    }

    void update() {
        if (state != PLAYING) return;
        
        ball.update();
        
        // 更新粒子
        for (auto it = particles.begin(); it != particles.end(); ) {
            it->update();
            if (it->life <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
        
        // 检查球与挡板的碰撞
        if (ball.y + BALL_RADIUS > paddle.y && 
            ball.y - BALL_RADIUS < paddle.y + paddle.height &&
            ball.x + BALL_RADIUS > paddle.x &&
            ball.x - BALL_RADIUS < paddle.x + paddle.width) {
            
            // 根据击中挡板的位置改变球的水平速度
            float relativeIntersectX = (paddle.x + paddle.width/2) - ball.x;
            float normalizedRelativeIntersectionX = relativeIntersectX / (paddle.width/2);
            float bounceAngle = normalizedRelativeIntersectionX * 1.2; // 最大角度为60度
            
            ball.dx = -5 * sin(bounceAngle);
            ball.dy = -abs(5 * cos(bounceAngle));
            
            // 创建粒子效果
            for (int i = 0; i < 20; i++) {
                particles.push_back(Particle(ball.x, ball.y, RGB(0, 200, 255)));
            }
        }
        
        // 检查球是否掉出屏幕底部
        if (ball.y - BALL_RADIUS > HEIGHT) {
            lives--;
            if (lives <= 0) {
                state = GAME_OVER;
            } else {
                ball.reset();
            }
        }
        
        // 检查球与砖块的碰撞
        for (auto& brick : bricks) {
            if (brick.active && brick.isHit(ball.x, ball.y, BALL_RADIUS)) {
                // 减少砖块生命值
                brick.health--;
                
                if (brick.health <= 0) {
                    brick.active = false;
                    score += brick.points;
                    
                    // 随机掉落特殊能力
                    if (rand() % 10 == 0) {
                        specialPower = true;
                    }
                    
                    // 创建粒子效果
                    for (int i = 0; i < 30; i++) {
                        particles.push_back(Particle(ball.x, ball.y, brick.color));
                    }
                }
                
                // 反弹球
                ball.dy = -ball.dy;
                
                // 调整球的位置防止卡在砖块内
                ball.y += ball.dy;
                
                break; // 一次只处理一个碰撞
            }
        }
        
        // 检查是否赢得游戏
        bool allBricksDestroyed = true;
        for (const auto& brick : bricks) {
            if (brick.active) {
                allBricksDestroyed = false;
                break;
            }
        }
        
        if (allBricksDestroyed) {
            state = WIN;
        }
    }

    void draw() {
        // 绘制背景
        setbkcolor(RGB(20, 20, 40));
        cleardevice();
        
        // 绘制星空背景
        setfillcolor(RGB(255, 255, 255));
        for (int i = 0; i < 100; i++) {
            int x = rand() % WIDTH;
            int y = rand() % HEIGHT;
            solidcircle(x, y, 1);
        }
        
        // 绘制粒子
        for (auto& particle : particles) {
            particle.draw();
        }
        
        // 绘制砖块
        for (auto& brick : bricks) {
            brick.draw();
        }
        
        // 绘制挡板
        paddle.draw();
        
        // 绘制球
        ball.draw();
        
        // 绘制UI
        drawUI();
        
        // 绘制特殊能力状态
        if (specialPower) {
            settextcolor(RGB(255, 215, 0));
            settextstyle(20, 0, _T("Arial"));
            outtextxy(WIDTH - 150, 10, _T("POWER UP!"));
        }
        
        // 根据游戏状态绘制不同界面
        if (state == START) {
            drawStartScreen();
        } else if (state == PAUSED) {
            drawPauseScreen();
        } else if (state == GAME_OVER) {
            drawGameOverScreen();
        } else if (state == WIN) {
            drawWinScreen();
        }
    }

    void drawUI() {
        // 绘制分数
        settextcolor(RGB(255, 255, 255));
        settextstyle(20, 0, _T("Arial"));
        
        TCHAR scoreStr[50];
        _stprintf(scoreStr, _T("Score: %d"), score);
        outtextxy(10, 10, scoreStr);
        
        // 绘制生命值
        TCHAR livesStr[50];
        _stprintf(livesStr, _T("Lives: %d"), lives);
        outtextxy(10, 40, livesStr);
        
        // 绘制等级
        TCHAR levelStr[50];
        _stprintf(levelStr, _T("Level: %d"), level);
        outtextxy(10, 70, levelStr);
        
        // 绘制游戏时间
        time_t currentTime = time(0);
        int elapsed = currentTime - startTime;
        TCHAR timeStr[50];
        _stprintf(timeStr, _T("Time: %d sec"), elapsed);
        outtextxy(WIDTH - 150, 40, timeStr);
        
        // 绘制生命指示器
        for (int i = 0; i < lives; i++) {
            setfillcolor(RGB(0, 200, 255));
            solidcircle(WIDTH - 50 - i*30, 20, 10);
        }
    }

    void drawStartScreen() {
        setfillcolor(RGB(0, 0, 0));
        solidrectangle(WIDTH/2 - 200, HEIGHT/2 - 150, WIDTH/2 + 200, HEIGHT/2 + 150);
        
        settextcolor(RGB(255, 255, 255));
        settextstyle(36, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 100, HEIGHT/2 - 100, _T("BREAKOUT"));
        
        settextstyle(24, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 120, HEIGHT/2, _T("Press SPACE to Start"));
        outtextxy(WIDTH/2 - 120, HEIGHT/2 + 50, _T("Press ESC to Quit"));
        
        settextstyle(18, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 150, HEIGHT/2 + 120, _T("Use LEFT/RIGHT arrows to move paddle"));
    }

    void drawPauseScreen() {
        setfillcolor(RGB(0, 0, 0));
        solidrectangle(WIDTH/2 - 150, HEIGHT/2 - 80, WIDTH/2 + 150, HEIGHT/2 + 80);
        
        settextcolor(RGB(255, 255, 255));
        settextstyle(36, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 70, HEIGHT/2 - 50, _T("PAUSED"));
        
        settextstyle(24, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 120, HEIGHT/2 + 10, _T("Press P to Resume"));
    }

    void drawGameOverScreen() {
        setfillcolor(RGB(50, 0, 0));
        solidrectangle(WIDTH/2 - 200, HEIGHT/2 - 100, WIDTH/2 + 200, HEIGHT/2 + 100);
        
        settextcolor(RGB(255, 100, 100));
        settextstyle(36, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 100, HEIGHT/2 - 50, _T("GAME OVER"));
        
        settextcolor(RGB(255, 255, 255));
        settextstyle(24, 0, _T("Arial"));
        
        TCHAR scoreStr[50];
        _stprintf(scoreStr, _T("Final Score: %d"), score);
        outtextxy(WIDTH/2 - 100, HEIGHT/2, scoreStr);
        
        outtextxy(WIDTH/2 - 120, HEIGHT/2 + 50, _T("Press SPACE to Restart"));
    }

    void drawWinScreen() {
        setfillcolor(RGB(0, 50, 0));
        solidrectangle(WIDTH/2 - 200, HEIGHT/2 - 100, WIDTH/2 + 200, HEIGHT/2 + 100);
        
        settextcolor(RGB(100, 255, 100));
        settextstyle(36, 0, _T("Arial"));
        outtextxy(WIDTH/2 - 70, HEIGHT/2 - 50, _T("YOU WIN!"));
        
        settextcolor(RGB(255, 255, 255));
        settextstyle(24, 0, _T("Arial"));
        
        TCHAR scoreStr[50];
        _stprintf(scoreStr, _T("Final Score: %d"), score);
        outtextxy(WIDTH/2 - 100, HEIGHT/2, scoreStr);
        
        time_t currentTime = time(0);
        int elapsed = currentTime - startTime;
        TCHAR timeStr[50];
        _stprintf(timeStr, _T("Time: %d seconds"), elapsed);
        outtextxy(WIDTH/2 - 100, HEIGHT/2 + 30, timeStr);
        
        outtextxy(WIDTH/2 - 120, HEIGHT/2 + 70, _T("Press SPACE to Next Level"));
    }

    void nextLevel() {
        level++;
        ball.reset();
        paddle = Paddle();
        createBricks();
        specialPower = false;
        startTime = time(0);
    }
};

int main() {
    // 初始化图形窗口
    initgraph(WIDTH, HEIGHT);
    setbkcolor(WHITE);
    cleardevice();
    
    BreakoutGame game;
    
    BeginBatchDraw(); // 开始批量绘图

    // 游戏主循环
    while (true) {
        // 处理键盘输入
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            if (game.state == PLAYING) {
                game.paddle.moveLeft();
                // 如果球粘在挡板上，移动球
                if (game.ball.sticky) {
                    game.ball.x = game.paddle.x + game.paddle.width/2;
                }
            }
        }
        
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            if (game.state == PLAYING) {
                game.paddle.moveRight();
                // 如果球粘在挡板上，移动球
                if (game.ball.sticky) {
                    game.ball.x = game.paddle.x + game.paddle.width/2;
                }
            }
        }
        
        // 发射球
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            if (game.state == START) {
                game.state = PLAYING;
                game.startTime = time(0);
            } 
            else if (game.state == PLAYING && game.ball.sticky) {
                game.ball.sticky = false;
            }
            else if (game.state == GAME_OVER) {
                game.resetGame();
                game.state = PLAYING;
            }
            else if (game.state == WIN) {
                game.nextLevel();
                game.state = PLAYING;
            }
        }
        
        // 暂停游戏
        if (GetAsyncKeyState('P') & 0x8000) {
            if (game.state == PLAYING) {
                game.state = PAUSED;
            }
            else if (game.state == PAUSED) {
                game.state = PLAYING;
            }
        }
        
        // 退出游戏
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        
        // 使用特殊能力
        if (GetAsyncKeyState('S') & 0x8000) {
            if (game.state == PLAYING && game.specialPower) {
                // 清除一行砖块
                for (int i = 0; i < game.bricks.size(); i++) {
                    if (game.bricks[i].active && game.bricks[i].y < 150) {
                        game.bricks[i].active = false;
                        game.score += game.bricks[i].points;
                        
                        // 创建粒子效果
                        for (int j = 0; j < 30; j++) {
                            game.particles.push_back(Particle(
                                game.bricks[i].x + game.bricks[i].width/2,
                                game.bricks[i].y + game.bricks[i].height/2,
                                RGB(255, 215, 0)
                            ));
                        }
                    }
                }
                game.specialPower = false;
            }
        }
        
        // 更新游戏状态
        game.update();
        
        // 绘制游戏画面
        game.draw();
        
        FlushBatchDraw(); // 刷新批量绘图

        // 延迟
        Sleep(10);
    }
    
    closegraph();
    EndBatchDraw(); // 结束批量绘图
    return 0;
}