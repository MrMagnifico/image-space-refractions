#include "menu.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <nativefiledialog/nfd.h>
DISABLE_WARNINGS_POP()

#include <utils/magic_enum.hpp>

#include <algorithm>
#include <iterator>


Menu::Menu(Config& config, MeshManager& meshManager)
    : m_config(config)
    , m_meshManager(meshManager) {}

void Menu::draw() {
    ImGui::Begin("Controls");

    // Button to select model
    if (ImGui::Button("Change model")) {
        nfdchar_t *outPath  = nullptr;
        nfdresult_t result  = NFD_OpenDialog("obj;cache", nullptr, &outPath);
        if (result == NFD_OKAY)         { m_meshManager.loadNewMesh(outPath); }
        else if (result == NFD_ERROR)   { throw std::runtime_error("NFD encountered an error"); }
        free(outPath);
    }

    // Selection controls for which thing to draw
    constexpr auto renderOptions = magic_enum::enum_names<RenderOption>();
    std::vector<const char*> renderOptionsPointers;
    std::transform(std::begin(renderOptions), std::end(renderOptions), std::back_inserter(renderOptionsPointers),
        [](const auto& str) { return str.data(); });
    ImGui::Combo("Render mode", (int*) &m_config.currentRender, renderOptionsPointers.data(), static_cast<int>(renderOptionsPointers.size()));

    // Draw combined rendering controls only if the combined result is being viewed
    if (m_config.currentRender == RenderOption::Combined) {
        ImGui::Checkbox("Show environment map", &m_config.showEnvironmentMap);
        ImGui::SliderFloat("Refractive index ratio", &m_config.refractiveIndexRatio, 1.0f, 2.0f);
        ImGui::ColorEdit3("Per-color transparency", glm::value_ptr(m_config.transparency));
    }

    ImGui::End();
}
