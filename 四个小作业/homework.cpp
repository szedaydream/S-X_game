#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480
#define CENTER_X WIDTH / 2
#define CENTER_Y HEIGHT / 2
#define PI 3.1415926

// 绘制表盘
void drawClockFace() {
	setfillcolor(RGB(240, 255, 240));			// 用RGB宏合成颜色
	solidcircle(CENTER_X, CENTER_Y, 189);

    setlinestyle(PS_SOLID, 3);
    setcolor(BLACK);
    circle(CENTER_X, CENTER_Y, 200);
    setlinestyle(PS_SOLID, 2);
    circle(CENTER_X, CENTER_Y, 190);

    int X_START, Y_START, X_FINISH, Y_FINISH;
    for(int i = 1; i <= 60; i ++)
    {
        double angle = i * (2 * PI / 60);
        X_START = CENTER_X + int(175 * sin(angle));
        X_FINISH = CENTER_X + int(180 * sin(angle));
        Y_START = CENTER_Y + int(175 * cos(angle));
        Y_FINISH = CENTER_Y + int(180 * cos(angle));
        setlinestyle(PS_SOLID, 1);
        if(i % 5 == 0)
        {
            setlinestyle(PS_SOLID, 2);
        }
        line(X_START, Y_START, X_FINISH, Y_FINISH);
    }

    settextstyle(30, 0, "Arial");
    for (int i = 1; i <= 12; i++) {
        double angle = i * (2 * PI / 12);
        int x = CENTER_X + int(150 * sin(angle));
        int y = CENTER_Y - int(150 * cos(angle));
        char num[3];
        sprintf(num, "%d", i);
        outtextxy(x - 10, y - 10, num);
    }
}

// 绘制指针
void drawHands(SYSTEMTIME time) {
    // 时针
    float hourAngle = (time.wHour % 12 + time.wMinute / 60.0 + time.wSecond / 3600.0) * (PI / 6);
    int hourX = CENTER_X + int(100 * sin(hourAngle));
    int hourY = CENTER_Y - int(100 * cos(hourAngle));
    setlinestyle(PS_SOLID, 6);
    setcolor(BLACK);
    line(CENTER_X, CENTER_Y, hourX, hourY);

    // 分针
    float minuteAngle = (time.wMinute + time.wSecond / 60.0) * (PI / 30);
    int minuteX = CENTER_X + int(150 * sin(minuteAngle));
    int minuteY = CENTER_Y - int(150 * cos(minuteAngle));
	setfillcolor(BLACK);
	solidcircle(CENTER_X, CENTER_Y, 9);
    setlinestyle(PS_SOLID, 4);
    setcolor(BLACK);
    line(CENTER_X, CENTER_Y, minuteX, minuteY);

    // 秒针
    float secondAngle = time.wSecond * (PI / 30);
    int secondX = CENTER_X + int(180 * sin(secondAngle));
    int secondY = CENTER_Y - int(180 * cos(secondAngle));
	setfillcolor(RED);
	solidcircle(CENTER_X, CENTER_Y, 6);
    setlinestyle(PS_SOLID, 2);
    setcolor(RED);
    line(CENTER_X, CENTER_Y, secondX, secondY);
}

int main() {
    initgraph(WIDTH, HEIGHT);
    setbkcolor(RGB(250, 255, 240));
    cleardevice();

    SYSTEMTIME time;
    BeginBatchDraw();
    while (!_kbhit()) {
        GetLocalTime(&time);
        cleardevice();
        drawClockFace();
        drawHands(time);
        FlushBatchDraw();
        Sleep(1000);
    }
    EndBatchDraw();
    closegraph();
    return 0;
}