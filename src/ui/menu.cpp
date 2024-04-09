#include "menu.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()

#include <utils/magic_enum.hpp>

#include <algorithm>
#include <iterator>


Menu::Menu(Config& config)
    : m_config(config) {}

void Menu::draw() {
    ImGui::Begin("Controls");

    // Selection controls for which thing to draw
    constexpr auto renderOptions = magic_enum::enum_names<RenderOption>();
    std::vector<const char*> renderOptionsPointers;
    std::transform(std::begin(renderOptions), std::end(renderOptions), std::back_inserter(renderOptionsPointers),
        [](const auto& str) { return str.data(); });
    ImGui::Combo("Render mode", (int*) &m_config.currentRender, renderOptionsPointers.data(), static_cast<int>(renderOptionsPointers.size()));

    ImGui::SliderFloat("Refraction Index", &m_config.refractiveIndex, 0.0f, 1.0f);

    ImGui::End();
}
