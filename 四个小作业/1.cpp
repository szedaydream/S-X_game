#include <iostream>
#include <graphics.h>
#include <conio.h>
using namespace std;

const int GRID_SIZE = 40;    // 每格像素
const int BOARD_SIZE = 15;   // 棋盘格数
const int MARGIN = 40;       // 边距
const int WIN_SIZE = GRID_SIZE * (BOARD_SIZE - 1) + MARGIN * 2; // 窗口大小

// 星位点坐标（以格为单位）
const int star_pos[5][2] = {
    {3, 3}, {3, 11}, {7, 7}, {11, 3}, {11, 11}
};

int main() {
    initgraph(WIN_SIZE, WIN_SIZE);
    setbkcolor(RGB(245, 222, 179)); // 浅木色背景
    cleardevice();

    // 画棋盘外边框
    setlinecolor(BROWN);
    setlinestyle(PS_SOLID, 3);
    rectangle(MARGIN - 5, MARGIN - 5, WIN_SIZE - MARGIN + 5, WIN_SIZE - MARGIN + 5);

    // 画棋盘格线
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i < BOARD_SIZE; ++i) {
        // 横线
        line(MARGIN, MARGIN + i * GRID_SIZE, WIN_SIZE - MARGIN, MARGIN + i * GRID_SIZE);
        // 竖线
        line(MARGIN + i * GRID_SIZE, MARGIN, MARGIN + i * GRID_SIZE, WIN_SIZE - MARGIN);
    }

    // 画星位点
    for (int i = 0; i < 5; ++i) {
        int x = MARGIN + star_pos[i][0] * GRID_SIZE;
        int y = MARGIN + star_pos[i][1] * GRID_SIZE;
        setfillcolor(BLACK);
        solidcircle(x, y, 5);
    }

    // 保持窗口
    system("pause");
    closegraph();
    return 0;
}