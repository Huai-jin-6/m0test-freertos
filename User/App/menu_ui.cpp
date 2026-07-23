/**
 * @file menu_ui.cpp
 * @brief 菜单 UI 类 — 按键消抖 + OLED 渲染
 */

#include "menu_ui.h"
#include "ssd1306.h"
#include "ti_msp_dl_config.h"

extern Ssd1306 ssd1306;

/* ---- 消抖: 连续 DEBOUNCE 次按下触发, 松开后才重新触发 ---- */

bool MenuUI::readBtn_(int raw, int &cnt, int &latched)
{
    if (raw)   /* 按下 (低电平) */
    {
        if (!latched && ++cnt >= DEBOUNCE)
        {
            latched = 1;
            cnt     = 0;
            return true;
        }
    }
    else
    {
        cnt     = 0;
        latched = 0;
    }
    return false;
}

/* ---- 公开方法 ---- */

void MenuUI::init(Menu *state) { state_ = state; }

void MenuUI::poll()
{
    int up_raw = DL_GPIO_readPins(Keys_PORT, Keys_Key1_up_PIN) == 0;
    int dn_raw = DL_GPIO_readPins(Keys_PORT, Keys_Key2_dn_PIN) == 0;
    int ok_raw = DL_GPIO_readPins(Keys_PORT, Keys_Key3_ok_PIN) == 0;

    if (readBtn_(up_raw, upCnt_, upLatched_)) state_->up();
    if (readBtn_(dn_raw, dnCnt_, dnLatched_)) state_->down();
    if (readBtn_(ok_raw, okCnt_, okLatched_)) state_->ok();
}

void MenuUI::render()
{
    const MenuEntry *e = state_->currentEntry();

    ssd1306.printf(0, 0, "%s", e->title);
    ssd1306.printf(0, 2, "%s", e->desc);

    if (state_->inTask())
        ssd1306.printf(0, 6, "OK: Exit");
    else
        ssd1306.printf(0, 6, "UP/DN:Sel OK:Go");
}

int  MenuUI::selected() const { return state_->consumeConfirm() ? state_->cursor() : -1; }
bool MenuUI::inTask()   const { return state_->inTask(); }
