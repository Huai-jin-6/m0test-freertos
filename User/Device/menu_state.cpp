/**
 * @file menu_state.cpp
 * @brief 菜单状态机实现
 */

#include "menu_state.h"

void Menu::init(const MenuEntry *entries, int count)
{
    entries_   = entries;
    count_     = count;
    cursor_    = 0;
    confirmed_ = false;
    running_   = false;
}

void Menu::up()
{
    if (running_) return;
    cursor_--;
    if (cursor_ < 0) cursor_ = count_ - 1;
}

void Menu::down()
{
    if (running_) return;
    cursor_++;
    if (cursor_ >= count_) cursor_ = 0;
}

void Menu::ok()
{
    if (running_)
        exitTask();
    else {
        confirmed_ = true;
        running_   = true;
    }
}

void Menu::enterTask()           { running_ = true; }
void Menu::exitTask()            { running_ = false; confirmed_ = false; }
bool Menu::consumeConfirm()      { bool r = confirmed_; confirmed_ = false; return r; }
