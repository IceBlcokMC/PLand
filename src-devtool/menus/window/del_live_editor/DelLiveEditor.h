#pragma once
#include "core/Menu.h"


namespace devtool {
class WindowManager;
} // namespace devtool

namespace devtool::menus {

class DelLiveEditorMenuElement final : public IMenuElement {
public:
    DelLiveEditorMenuElement(WindowManager& wm);
};

} // namespace devtool::menus