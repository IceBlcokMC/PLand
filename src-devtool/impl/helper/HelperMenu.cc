#include "HelperMenu.h"

#include <imgui.h>

namespace devtool {

HelperMenu::HelperMenu() : IMenu("帮助") {
    // 注册菜单元素
    this->registerElement<internals::About>();
}

namespace internals {

About::About() : IMenuElement("关于") { window_ = std::make_unique<AboutWindow>(); }

bool* About::getSelectFlag() { return window_->getOpenFlag(); }

bool About::isSelected() const { return window_->isOpen(); }

void About::tick() { window_->tick(); }


AboutWindow::AboutWindow() = default;
void AboutWindow::render() {
    if (!ImGui::Begin("关于", getOpenFlag())) {
        ImGui::End();
        return;
    }

    ImGui::Text("PLand DevTools - 开发者工具");
    ImGui::Text("此工具用于快捷调试、查看、修改 PLand 的运行状态");
    ImGui::Text("\n\n");
    ImGui::Separator();
    ImGui::Text("Built with Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

    ImGui::End();
}

} // namespace internals


} // namespace devtool