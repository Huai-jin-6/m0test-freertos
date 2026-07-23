/**
 * @file menu_ui.h
 * @brief 菜单 UI 类 — 按键消抖 + OLED 渲染
 */

#ifndef __MENU_UI_HPP__
#define __MENU_UI_HPP__

#include "menu_state.h"

class Menu;
class Ssd1306;

class MenuUI
{
public:
    void init(Menu *state);      ///< 绑定状态机
    void poll();                 ///< 每 50ms: 扫描按键
    void render();               ///< 渲染菜单到 OLED
    int  selected() const;       ///< -1=未确认, >=0=菜单索引
    bool inTask() const;         ///< 任务运行中?

private:
    Menu  *state_ = nullptr;
    int    upCnt_ = 0,  upLatched_ = 0;
    int    dnCnt_ = 0,  dnLatched_ = 0;
    int    okCnt_ = 0,  okLatched_ = 0;

    static constexpr int DEBOUNCE = 3;
    bool readBtn_(int raw, int &cnt, int &latched);
};

#endif
