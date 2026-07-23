/**
 * @file menu_state.h
 * @brief 菜单状态机（纯逻辑，零硬件依赖）
 */

#ifndef __MENU_STATE_HPP__
#define __MENU_STATE_HPP__

#include <stdint.h>

struct MenuEntry
{
    const char *title;
    const char *desc;
};

class Menu
{
public:
    void init(const MenuEntry *entries, int count);

    void up();
    void down();
    void ok();

    void enterTask();
    void exitTask();
    bool inTask() const { return running_; }

    int  cursor() const          { return cursor_; }
    const MenuEntry *currentEntry() const { return &entries_[cursor_]; }
    bool consumeConfirm();        ///< 返回 true 并清零，只触发一次

private:
    const MenuEntry *entries_  = nullptr;
    int              count_    = 0;
    int              cursor_   = 0;
    bool             confirmed_ = false;
    bool             running_   = false;
};

#endif
