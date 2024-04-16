#pragma once
#ifndef _MENU_H_
#define _MENU_H_

#include <render/mesh_manager.h>
#include <utils/config.h>


class Menu {
public:
    Menu(Config& config, MeshManager& meshManager);

    void draw();

private:
    Config& m_config;
    MeshManager& m_meshManager;
};

#endif