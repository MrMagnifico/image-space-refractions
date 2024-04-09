#pragma once
#ifndef _MENU_H_
#define _MENU_H_

#include <utils/config.h>


class Menu {
public:
    Menu(Config& config);

    void draw();

private:
    Config& m_config;
};

#endif