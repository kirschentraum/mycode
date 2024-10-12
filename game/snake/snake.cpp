#include<list>
#include<windows.h>
#include<stdlib.h>
#include<time.h>
#include <stdio.h>

using namespace std;

class snake;//蛇类声明
class food;//食物类声明
enum class direction : short;//移动方向枚举类声明

constexpr int IDTIMER{ 1 };//定时器标识符
constexpr int speed{ 200 };//运动一格所需要的毫秒数

constexpr long game_height{ 30 }, game_width{ 60 };//蛇的移动范围

constexpr COLORREF snake_head_color = RGB(0, 255, 0);
constexpr COLORREF snake_body_color = RGB(0, 255, 255);
constexpr COLORREF food_color = RGB(255, 0, 0);//蛇和食物的颜色

constexpr COLORREF wall_color = RGB(0x8B, 0x47, 0x26);//墙的颜色

constexpr COLORREF back_color = RGB(0xFF, 0xE7, 0xBA);//背景颜色

constexpr double wall_out = 0.8;//外墙右侧占客户区窗口比例
constexpr double wall_wid_y = 0.05;//墙的纵向宽度占客户区窗口比例
constexpr double wall_wid_x = 0.02;//墙的横向宽度占客户区窗口比例

constexpr long init_length = 2;//蛇的初始长度

long score;//分数
long client_x, client_y;//客户区尺寸

//绘制并填充矩形函数
void rect_draw_fill(HDC, long, long, long, long, COLORREF);
void rect_draw_fill(HDC, RECT, COLORREF);
//游戏坐标转设备矩形坐标函数
RECT coord_convert(const POINT);
void game_init(HWND);//游戏初始化函数
void game_end(HWND);//游戏结束函数
//移动方向枚举类
enum class direction : short
{
    up, down, left, right
};

//蛇类
class snake
{
private:
    POINT head;//蛇的头
    list<POINT> body;//蛇的身体
    direction move_direction;//蛇的移动方向

public:
    //蛇的初始化
    snake()
    {
        fresh();
    }
    void move(HWND);//蛇的移动
    void die(HWND);//蛇的死亡
    void set_direction(direction set_direction)
    {
        move_direction = set_direction;
    }
    direction get_direction()
    {
        return move_direction;
    }
    bool if_snake(POINT pos) const;//判断一个位置是否属于蛇头或蛇身
    void draw(HDC) const;//显示蛇
    void fresh();//蛇刷新
    friend class food;
};
//蛇对象
snake* game_snake;

//食物类
class food
{
private:
    POINT food_pos;
public:
    //食物初始化
    food(HWND hwnd)
    {
        srand(static_cast<unsigned>(time(nullptr)));
        fresh(hwnd);
    }
    void draw(HDC) const;//显示食物
    void fresh(HWND);//食物刷新
    friend class snake;
};
//食物对象
food* game_food;

//显示食物
void food::draw(HDC hdc) const
{
    RECT rect = coord_convert(food_pos);
    rect_draw_fill(hdc, rect, food_color);
}

//食物刷新
void food::fresh(HWND hwnd)
{
    do
    {
        food_pos.x = rand() % game_width;
        food_pos.y = rand() % game_height;
    }while (game_snake->if_snake(food_pos));
    RECT rect=coord_convert(food_pos);
    InvalidateRect(hwnd, &rect, TRUE);
}

//蛇刷新
void snake::fresh()
{
    body.clear();
    head.x = game_width / 2;
    head.y = game_height / 2;
    for(long i=1; i<init_length;i++)
        body.push_back(POINT{ game_width / 2 - i, game_height / 2 });
    move_direction = direction::right;
}

//判断一个位置是否属于蛇头或蛇身
bool snake::if_snake(POINT pos) const
{
    list<POINT>::const_iterator iter;
    if ((pos.x == head.x)&&(pos.y==head.y))
        return true;
    else
    {
        iter = body.begin();
        do
        {
            if (((* iter).x == pos.x)&& ((*iter).y == pos.y))
                return true;
            iter++;
        }while (iter != body.end());
        return false;
    }
}

//显示蛇
void snake::draw(HDC hdc) const
{
    RECT rect= coord_convert(head);
    rect_draw_fill(hdc, rect, snake_head_color);
    list<POINT>::const_iterator iter;
    iter = body.begin();
    do
    {
        rect= coord_convert(*iter);
        rect_draw_fill(hdc, rect, snake_body_color);
        iter++;
    }while (iter != body.end());
}

//移动蛇
void snake::move(HWND hwnd)
{
    RECT rect;
    list<POINT>::iterator iter;
    body.push_front(head);
    rect = coord_convert(head);
    InvalidateRect(hwnd, &rect, TRUE);
    POINT temp = head;
    switch (move_direction)
    {
    case direction::left:
        temp.x--;
        break;
    case direction::right:
        temp.x++;
        break;
    case direction::up:
        temp.y--;
        break;
    case direction::down:
        temp.y++;
    }
    
    if ((temp.x == game_food->food_pos.x) && (temp.y == game_food->food_pos.y))//判断是否吃到食物.
    {
        head = temp;
        rect = coord_convert(head);
        InvalidateRect(hwnd, &rect, TRUE);
        game_food->fresh(hwnd);
        score++;
        SetRect(&rect, static_cast<int>(client_x * wall_out), 0, client_x, client_y);
        InvalidateRect(hwnd, &rect, TRUE);

    }
    else
    {
        iter = body.end();
        iter--;
        rect = coord_convert(*iter);
        InvalidateRect(hwnd, &rect, TRUE);
        body.pop_back();
        if (if_snake(temp))//判断是否撞到自己
        {
            die(hwnd);
        }
        else if ((temp.x < 0) || (temp.y < 0) || (temp.x >= game_width) || (temp.y >= game_height))//判断是否撞到墙
        {
            die(hwnd);
        }
        else
        {
            head = temp;
            rect = coord_convert(head);
            InvalidateRect(hwnd, &rect, TRUE);
        }
    }
}

//蛇死亡
void snake::die(HWND hwnd)
{
    wchar_t output_info[100];
    KillTimer(hwnd, IDTIMER);
    swprintf_s(output_info, L"游戏结束！\n你的得分是：%ld\n是否重新开始游戏？", score);
    int ans=MessageBoxW(hwnd, output_info, L"游戏结束", MB_YESNO);
    if (ans == IDYES)
    {
        delete game_snake;
        delete game_food;
        game_init(hwnd);
    }
    else
        PostMessageW(hwnd, WM_DESTROY, NULL, NULL);
}

//游戏初始化函数
void game_init(HWND hwnd)
{
    score = 0;
    game_snake = new snake;;
    game_food = new food(hwnd);
    SetTimer(hwnd, IDTIMER, speed, NULL);
    InvalidateRect(hwnd, NULL, FALSE);
}

//游戏结束函数
void game_end(HWND hwnd)
{
    KillTimer(hwnd, IDTIMER);
    delete game_snake;
    delete game_food;
}

//绘制并填充矩形函数
void rect_draw_fill(HDC hdc, long left, long top, long right, long bottom, COLORREF color)
{
    RECT rect;
    HBRUSH hbr = CreateSolidBrush(color);
    Rectangle(hdc, left, top, right, bottom);
    SetRect(&rect, left, top, right, bottom);
    FillRect(hdc, &rect, hbr);
    DeleteObject(hbr);
}

void rect_draw_fill(HDC hdc, RECT rect, COLORREF color)
{
    HBRUSH hbr = CreateSolidBrush(color);
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    FillRect(hdc, &rect, hbr);
    DeleteObject(hbr);
}

//游戏坐标转设备矩形坐标函数
RECT coord_convert(const POINT game_pos)
{
    double scale_x, scale_y;
    RECT rect;
    long left, top, right, bottom;
    scale_x = client_x * (wall_out - 2 * wall_wid_x) / game_width;
    scale_y = client_y * (1 - 2 * wall_wid_y) / game_height;
    left = static_cast<long>(client_x * wall_wid_x + game_pos.x * scale_x);
    top = static_cast<long>(client_y * wall_wid_y + game_pos.y * scale_y);
    right = left + static_cast<long>(scale_x);
    bottom = top + static_cast<long>(scale_y);
    SetRect(&rect, left, top, right, bottom);
    return rect;
}

//消息处理函数
LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    wchar_t score_info[100];
    static FILE* log;
    switch (message)
    {
    case WM_CREATE:
        game_init(hwnd);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            if((game_snake->get_direction() == direction::left) || (game_snake->get_direction() == direction::right))
                game_snake->set_direction(direction::up);
            if (game_snake->get_direction() == direction::up)
            {
                game_snake->move(hwnd);
            }
            break;
        case VK_DOWN:
            if ((game_snake->get_direction() == direction::left) || (game_snake->get_direction() == direction::right))
                game_snake->set_direction(direction::down);
            if (game_snake->get_direction() == direction::down)
            {
                game_snake->move(hwnd);
            }
            break;
        case VK_LEFT:
            if ((game_snake->get_direction() == direction::up) || (game_snake->get_direction() == direction::down))
                game_snake->set_direction(direction::left);
            if (game_snake->get_direction() == direction::left)
            {
                game_snake->move(hwnd);
            }
            break;
        case VK_RIGHT:
            if ((game_snake->get_direction() == direction::up) || (game_snake->get_direction() == direction::down))
                game_snake->set_direction(direction::right);
            if (game_snake->get_direction() == direction::right)
            {
                game_snake->move(hwnd);
            }
            break;
        }
        return 0;
    case WM_SIZE:
        client_x = LOWORD(lParam);
        client_y = HIWORD(lParam);
        return 0;
    case WM_TIMER:
        game_snake->move(hwnd);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        rect_draw_fill(hdc, 0, 0, static_cast<long>(client_x * wall_out), client_y, wall_color);//画墙
        rect_draw_fill(hdc, static_cast<long>(client_x * wall_wid_x), static_cast<long>(client_y * wall_wid_y),
            static_cast<long>(client_x * (wall_out- wall_wid_x)), static_cast<long>(client_y*(1- wall_wid_y)), back_color);//画游戏区
        rect_draw_fill(hdc, static_cast<long>(client_x * wall_out), 0, client_x, client_y, back_color);//画信息区
        game_snake->draw(hdc);//画食物
        game_food->draw(hdc);//画蛇
        SetTextAlign(hdc, TA_CENTER);
        swprintf_s(score_info, L"你的分数：%ld", score);
        TextOutW(hdc, static_cast<int>(client_x * 0.9), static_cast<int>(client_y / 2), score_info, static_cast<int>(wcslen(score_info)));
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        game_end(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
//主函数
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int nCmdShow)
{
    static wchar_t szAppName[] = L"snake";
    MSG msg;
    WNDCLASSW wndclass;
    HWND hwnd;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    RegisterClassW(&wndclass);
    hwnd = CreateWindowExW(0L, szAppName, L"贪吃蛇", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}