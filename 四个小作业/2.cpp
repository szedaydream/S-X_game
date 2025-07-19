#include <graphics.h>
#include <conio.h>
#include <vector>

struct Shape {
    enum Type { Square, Circle } type;
    int x, y, size;
    COLORREF color;
};

int main() {
    initgraph(800, 600);
    setbkcolor(WHITE);
    cleardevice();

    std::vector<Shape> shapes;
    COLORREF currentColor = BLACK;

    while (true) {
        // 鼠标事件
        ExMessage msg;
        if (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
            // 鼠标左键或右键
            if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
                Shape s;
                s.x = msg.x;
                s.y = msg.y;
                s.size = 40;
                s.color = currentColor;
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000) s.size += 10;
                s.type = (msg.message == WM_LBUTTONDOWN) ? Shape::Square : Shape::Circle;
                shapes.push_back(s);
            }
            // 键盘事件
            if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == 'R') currentColor = RED;
                if (msg.vkcode == 'G') currentColor = GREEN;
                if (msg.vkcode == 'B') currentColor = BLUE;
            }
        }

        // 绘制所有图形
        cleardevice();
        for (const auto& s : shapes) {
            setfillcolor(s.color);
            if (s.type == Shape::Square)
                solidrectangle(s.x - s.size / 2, s.y - s.size / 2, s.x + s.size / 2, s.y + s.size / 2);
            else
                solidcircle(s.x, s.y, s.size / 2);
        }
        FlushBatchDraw();
        Sleep(10);
    }

    closegraph();
    return 0;
}