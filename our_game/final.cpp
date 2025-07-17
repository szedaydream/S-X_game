#include <graphics.h>
#include <easyx.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <algorithm>
#include <windows.h>
#include <vector>
#include <cmath>

using namespace std;

const int PLAYER_ANIM_NUM = 6;
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

void putimage_alpha(int x, int y, IMAGE* img) {
    if(!img || img -> getwidth() <= 0 || img -> getheight() <= 0) {
        return; // 如果图像无效，直接返回
    }

    int w = img->getwidth();
    int h = img->getheight();

    DWORD* p_dst = GetImageBuffer(nullptr);
    DWORD* p_src = GetImageBuffer(img);

    if(!p_dst || !p_src) {
        return; // 如果获取缓冲区失败，直接返回
    }

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            int dst_x = x + j;
            int dst_y = y + i;
            if(dst_x < 0 || dst_x >= getwidth() || dst_y < 0 || dst_y >= getheight()) {
                continue; // 如果目标位置超出屏幕范围，跳过
            }

            DWORD src_color = p_src[i * w + j];
            int alpha = (src_color >> 24) & 0xFF; // 获取源颜色的 alpha 通道
            if(alpha == 0) {
                continue; // 如果 alpha 为 0，跳过透明像素
            }

            DWORD* p_dst_pixel = &p_dst[dst_y * getwidth() + dst_x];
            if(alpha == 255) {
                *p_dst_pixel = src_color; // 如果 alpha 为 255，直接覆盖
            }
            else {
                // 混合颜色
                DWORD dst_color = *p_dst_pixel;
                int dst_r = (dst_color & 0xFF0000) >> 16;
                int dst_g = (dst_color & 0x00FF00) >> 8;
                int dst_b = dst_color & 0x0000FF;
                int src_r = (src_color & 0xFF0000) >> 16;
                int src_g = (src_color & 0x00FF00) >> 8;
                int src_b = src_color & 0x0000FF;
                *p_dst_pixel = RGB((src_r * alpha + dst_r * (255 - alpha)) / 255,
                                   (src_g * alpha + dst_g * (255 - alpha)) / 255,
                                   (src_b * alpha + dst_b * (255 - alpha)) / 255);
            }
        }
    }
}

class Animation {
public:
    Animation(LPCTSTR path, int num, int interval) {
        internal_ms = interval;

        TCHAR path_file[256];
        for(size_t i = 0; i < num; i++) {
            _stprintf_s(path_file, path, i);

            IMAGE* frame = new IMAGE();
            loadimage(frame, path_file);
            frame_list.push_back(frame);
        }
    }

    ~Animation() {
        for(size_t i = 0; i < frame_list.size(); i++) {
            delete frame_list[i];
        }
    }

    void Play(int x, int y, int delta) {
        timer += delta;
        if(timer >= internal_ms) {
            idx_frame = (idx_frame + 1) % frame_list.size();
            timer = 0;
        }

        putimage_alpha(x, y, frame_list[idx_frame]);
    }

private:
    vector<IMAGE*> frame_list;
    int internal_ms = 0;
    int timer = 0;
    int idx_frame = 0;
};

class Player {
public:
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 69;

public:
    Player() {
        loadimage(&img_shadow, _T("shadow_player.png"));
        // anim_left = new Animation(
        //     _T("D:\\lessons\\Easyx\\task\\try\\img3\\player_left_%d.png"), 
        //     PLAYER_ANIM_NUM, 45
        // );
        // anim_right = new Animation(
        //     _T("D:\\lessons\\Easyx\\task\\try\\img3\\player_right_%d.png"), 
        //     PLAYER_ANIM_NUM, 45
        // );
        loadimage(&fire, _T("fire.png"));
    }

    ~Player() {
        // delete anim_left;
        // delete anim_right;
    }

    void ProcessEvent(const ExMessage& msg) {
        if(msg.message == WM_KEYDOWN) {
            switch (msg.vkcode)
            {
            case VK_UP:
                is_move_up = true; // 向上移动
                break;
            
            case VK_DOWN:
                is_move_down = true; // 向下移动
                break;

            case VK_LEFT:
                is_move_left = true; // 向左移动
                break;
            
            case VK_RIGHT:
                is_move_right = true; // 向右移动
                break;
            }
        }
        else if(msg.message == WM_KEYUP) {
            switch (msg.vkcode)
            {
            case VK_UP:
                is_move_up = false; // 停止向上移动
                break;
            
            case VK_DOWN:
                is_move_down = false; // 停止向下移动
                break;

            case VK_LEFT:
                is_move_left = false; // 停止向左移动
                break;
            
            case VK_RIGHT:
                is_move_right = false; // 停止向右移动
                break;
            
            case VK_ESCAPE: // 按 ESC 键退出
                PostQuitMessage(0);
                break;
            }
        }
    }

    void Move() {
        int dir_x = is_move_right - is_move_left; // 计算水平移动方向
        int dir_y = is_move_down - is_move_up; // 计算垂直移动方向
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if(len_dir != 0) {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += (int)(normalized_x * SPEED);
            position.y += (int)(normalized_y * SPEED);
        }

        if(position.x < 0) {
            position.x = 0;
        }
        if(position.y < 0) {
            position.y = 0;
        }
        if(position.x + PLAYER_WIDTH > WINDOW_WIDTH) {
            position.x = WINDOW_WIDTH - PLAYER_WIDTH;
        }
        if(position.y + PLAYER_HEIGHT > WINDOW_HEIGHT) {
            position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
        }
    }

    void Draw(int delta) {
        int pos_shadow_x = position.x + PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2;
        int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
        putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
        putimage_alpha(position.x, position.y, &fire);

        // static bool facing_left = false;
        // if(is_move_left) {
        //     facing_left = true;
        // }
        // else if(is_move_right) {
        //     facing_left = false;
        // }

        // if(facing_left) {
        //     anim_left->Play(position.x, position.y, delta);
        // }
        // else {
        //     anim_right->Play(position.x, position.y, delta);
        // }
    }

    const POINT& GetPosition() const {
        return position;
    }

private:
    const int SPEED = 2;
    const int PLAYER_WIDTH = 80;
    const int PLAYER_HEIGHT = 69;
    const int SHADOW_WIDTH = 32;

private:
    IMAGE img_shadow;
    IMAGE fire;
    // Animation* anim_left;
    // Animation* anim_right;
    POINT position = { 300, 600 };
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;
};

class End {
public:
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 69;

public:
    End() {
        loadimage(&img_shadow, _T("shadow_player.png"));
        loadimage(&ice, _T("ice.png"));
    }

    void Draw(int delta) {
        int pos_shadow_x = position.x + PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2;
        int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
        putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
        putimage_alpha(position.x, position.y, &ice);
    }

    // 将 CheckEndCollision 移动到 public 区域
    bool CheckEndCollision(const Player& player) {
        POINT check_position = { 
            position.x + FRAME_WIDTH / 2, 
            position.y + FRAME_HEIGHT / 2 
        };
        
        const POINT& player_pos = player.GetPosition();
        return check_position.x >= player_pos.x && 
               check_position.x <= player_pos.x + player.FRAME_WIDTH &&
               check_position.y >= player_pos.y && 
               check_position.y <= player_pos.y + player.FRAME_HEIGHT;
    }

    ~End() {}

private:  // 私有成员放在这里
    const int PLAYER_WIDTH = 80;
    const int PLAYER_HEIGHT = 69;
    const int SHADOW_WIDTH = 32;

    IMAGE img_shadow;
    IMAGE ice;
    POINT position = { 1100, 100 };
};

class Bullet {
public:
    POINT position = { 0, 0 }; // 子弹位置

public:
    Bullet() = default;

    ~Bullet() = default;

    void Draw() const {
        setlinecolor(RGB(255, 155, 50));
        setfillcolor(RGB(200, 75, 10));
        fillcircle(position.x, position.y, RADIUS);
    }

private:
    const int RADIUS = 10; // 子弹半径
};

class Enemy {
public:
    Enemy() {
        loadimage(&img_shadow, _T("shadow_enemy.png"));
        anim_left = new Animation(
            _T("enemy_left_%d.png"), 
            PLAYER_ANIM_NUM, 45
        );
        anim_right = new Animation(
            _T("enemy_right_%d.png"), 
            PLAYER_ANIM_NUM, 45
        );

        enum class SpawnEdge {
            Up = 0,
            Down,
            Left,
            Right
        };

        SpawnEdge edge = (SpawnEdge)(rand() % 4);
        switch(edge) {
        case SpawnEdge::Up:
            position.x = rand() % WINDOW_WIDTH;
            position.y = -FRAME_HEIGHT;
            break;
        case SpawnEdge::Down:
            position.x = rand() % WINDOW_WIDTH;
            position.y = WINDOW_HEIGHT;
            break;
        case SpawnEdge::Left:
            position.x = -FRAME_WIDTH;
            position.y = rand() % WINDOW_HEIGHT;
            break;
        case SpawnEdge::Right:
            position.x = WINDOW_WIDTH;
            position.y = rand() % WINDOW_HEIGHT;
            break;
        default:
            break;
        }
    }

    bool CheckBulletCollision(const Bullet& bullet) {
        bool is_overlap_x = bullet.position.x >= position.x && 
                            bullet.position.x <= position.x + FRAME_WIDTH;
        bool is_overlap_y = bullet.position.y >= position.y && 
                            bullet.position.y <= position.y + FRAME_HEIGHT;
        return is_overlap_x && is_overlap_y; // 这里可以添加子弹与敌人碰撞检测逻辑
    }

    bool CheckPlayerCollision(const Player& player) {
        POINT check_position = { position.x + FRAME_WIDTH / 2, 
                                  position.y + FRAME_HEIGHT / 2 };
        return check_position.x >= player.GetPosition().x && 
               check_position.x <= player.GetPosition().x + player.FRAME_WIDTH &&
               check_position.y >= player.GetPosition().y && 
               check_position.y <= player.GetPosition().y + player.FRAME_HEIGHT;
    }

    void Move(const Player& player) {
        const POINT& player_pos = player.GetPosition();
        int dir_x = player_pos.x - position.x; // 计算水平方向
        int dir_y = player_pos.y - position.y; // 计算垂直方向
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if(len_dir != 0) {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += (int)(normalized_x * SPEED);
            position.y += (int)(normalized_y * SPEED);
        }

        if(dir_x < 0) {
            facing_left = true; // 如果玩家在左侧，敌人面向左侧
        }
        else if(dir_x > 0) {
            facing_left = false; // 如果玩家在右侧，敌人面向右侧
        }
    }

    void Draw(int delta) {
        int pos_shadow_x = position.x + FRAME_WIDTH / 2 - SHADOW_WIDTH / 2;
        int pos_shadow_y = position.y + FRAME_HEIGHT - 35;
        putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

        if(facing_left) {
            anim_left->Play(position.x, position.y, delta);
        }
        else {
            anim_right->Play(position.x, position.y, delta);
        }
    }

    ~Enemy() {
        delete anim_left;
        delete anim_right;
    }

    void Hurt() {
        alive = false; // 伤害处理逻辑
    }

    bool CheckAlive() {
        return alive; // 检查敌人是否存活
    }

private:
    const int SPEED = 8;
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 69;
    const int SHADOW_WIDTH = 48;

private:
    IMAGE img_shadow;
    Animation* anim_left;
    Animation* anim_right;
    POINT position = { 0, 0 };
    bool facing_left = false;
    bool alive = true; // 敌人是否存活
};

void TryGenerateEnemy(vector<Enemy*>& enemy_list) {
    const int INTERVAL = 100;
    static int counter = 0;
    if((++counter) % INTERVAL == 0) {
        enemy_list.push_back(new Enemy()); // 每隔一定时间生成一个敌人
    }
}

void UpdateBullets(vector<Bullet>& bullet_list, const Player& player) {
    const double RADIAL_SPEED = 0.0045; // 子弹速度
    const double TANGENT_SPEED = 0.0055; // 子弹速度
    double radian_interval = 2 * M_PI / bullet_list.size(); // 子弹间隔角度
    POINT player_pos = player.GetPosition();
    double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
    for(size_t i = 0; i < bullet_list.size(); i++) {
        double radian = GetTickCount() * TANGENT_SPEED + i * radian_interval;
        bullet_list[i].position.x = player_pos.x + player.FRAME_WIDTH / 2 + (int)(radius * sin(radian));
        bullet_list[i].position.y = player_pos.y + player.FRAME_HEIGHT / 2 + (int)(radius * cos(radian));
    }
}

void DrawPlayerScore(int score) {
    static TCHAR score_text[32];
    _stprintf_s(score_text, _T("Score: %d"), score);
    settextcolor(RGB(255, 85, 185));
    settextstyle(20, 0, _T("Arial"));
    outtextxy(10, 10, score_text); // 在左上角显示分数
}

int main() {
    initgraph(1280, 720);

    // mciSendString(_T("open D:\\lessons\\Easyx\\task\\try\\mus3\\bgm.mp3 alias bgm"), NULL, 0, NULL);
    // mciSendString(_T("open D:\\lessons\\Easyx\\task\\try\\mus3\\hit.wav alias hit"), NULL, 0, NULL);

    // mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL); // 播放背景音乐

    bool running = true;
    
    int score = 0;
    Player player; // 创建玩家对象
    End end;
    ExMessage msg;
    IMAGE img_background;
    vector<Enemy*>enemy_list;
    vector<Bullet> bullet_list(3);

    loadimage(&img_background, _T("background.png"));
    // loadAnimation(); // 加载动画

    BeginBatchDraw(); // 开始批量绘图

    while(running) {
        DWORD start_time = GetTickCount(); // 获取当前时间戳

        while(peekmessage(&msg)) {
            player.ProcessEvent(msg); // 处理玩家事件

            if(msg.message == WM_QUIT) {
                running = false; // 如果收到退出消息，停止循环
            }
        }

        // if(is_move_up) {
        //     player_pos.y -= PLAYER_SPEED; // 向上移动
        // }
        // if(is_move_down) {
        //     player_pos.y += PLAYER_SPEED; // 向下移动
        // }
        // if(is_move_left) {
        //     player_pos.x -= PLAYER_SPEED; // 向左移动
        // }
        // if(is_move_right) {
        //     player_pos.x += PLAYER_SPEED; // 向右移动
        // }

        // static int counter = 0;
        // if(++counter % 5 == 0) {
        //     idx_current_anim++;
        // }

        // idx_current_anim %= PLAYER_ANIM_NUM;

        player.Move(); // 移动玩家
        UpdateBullets(bullet_list, player); // 更新子弹列表
        TryGenerateEnemy(enemy_list); // 尝试生成敌人
        for(Enemy* enemy : enemy_list) {
            enemy->Move(player); // 移动敌人
        }

        for(Enemy* enemy : enemy_list) {
            if(enemy->CheckPlayerCollision(player)) {
                static TCHAR score_text[128];
                _stprintf_s(score_text, _T("Score: %d"), score);
               MessageBox(GetHWnd(), _T("Game Over!"), _T("Collision Detected"), MB_OK);
               running = false; // 如果敌人与玩家碰撞，结束游戏
               break;
            }
        }

        for(Enemy* enemy : enemy_list) {
            for(Bullet& bullet : bullet_list) {
                if(enemy->CheckBulletCollision(bullet)) {
                    // mciSendString(_T("play hit from 0"), NULL, 0, NULL); // 播放击中音效
                    enemy->Hurt(); // 敌人受伤
                    score++;
                }
            }
        }

        for(size_t i = 0; i < enemy_list.size(); i++) {
            Enemy* enemy = enemy_list[i];
            if(!enemy->CheckAlive()) {
                swap(enemy_list[i], enemy_list.back()); // 将死亡的敌人移到最后
                enemy_list.pop_back(); // 移除死亡的敌人
                delete enemy; // 释放内存
            }
        }

        if(end.CheckEndCollision(player)) {
            static TCHAR score_text[128];
            _stprintf_s(score_text, _T("Score: %d"), score);
            MessageBox(GetHWnd(), _T("Win!"), _T("Collision Detected"), MB_OK);
            running = false; // 如果与玩家碰撞，结束游戏
            break;
        }
        
        cleardevice(); // 清除屏幕

        putimage(0, 0, &img_background); // 绘制背景
        end.Draw(1000 / 144);
        // putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]); // 绘制左侧动画
        player.Draw(1000 / 144); // 绘制玩家
        for(Enemy* enemy : enemy_list) {
            enemy->Draw(1000 / 144); // 绘制敌人
        }
        for(const Bullet& bullet : bullet_list) {
            bullet.Draw(); // 绘制子弹
        }

        DrawPlayerScore(score); // 绘制玩家分数

        FlushBatchDraw(); // 刷新批量绘图

        DWORD end_time = GetTickCount(); // 获取结束时间戳
        DWORD elapsed_time = end_time - start_time; // 计算时间差
        if (elapsed_time < 16) { // 保持60FPS
            Sleep(16 - elapsed_time);
        }
    }

    EndBatchDraw(); // 结束批量绘图

    return 0;
}