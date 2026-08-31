#include "DelLiveEditor.h"
#include "core/Window.h"
#include "core/WindowManager.h"

#include "del-editor/bootstrap.h"
#include "del/template_engine.h"

#include "ll/api/Expected.h"
#include "nlohmann/json_fwd.hpp"
#include "pland/internal/import_framework/ImportProcesser.h"

#include <memory>

namespace devtool::menus {


class DelEditor final : public IWindow, public del_editor::EditorBootstrap {
    WindowManager& wm_;
    bool           lastOpen_{false}; // 上一帧编辑器窗口是否显示(检测用户点 X)

public:
    inline static del_editor::EditorConfig cfg = {
        .window_name    = kWindowName,
        .own_window     = true,
        .fullscreen     = false,
        .initial_width  = 900.0f,
        .initial_height = 560.0f,
    };

    explicit DelEditor(std::string const& title, WindowManager& wm)
    : IWindow(title),
      del_editor::EditorBootstrap(land::internal::import_framework::ImportProcesser::newTemplateEngine(), cfg),
      wm_(wm) {}

    void render() override {
        // X 关闭 → 同步菜单取消勾选; 菜单勾选 → 重开
        if (lastOpen_ && !this->IsOpen()) {
            setVisible(false);
        }
        lastOpen_ = this->IsOpen();

        if (!visible()) {
            this->SetOpen(false);
            return;
        }
        if (!this->IsOpen()) {
            this->SetOpen(true);
        }
        if (this->hostDockspace() == 0) {
            this->SetHostDockspace(wm_.dockspaceId());
        }
        this->Render();
    }


    auto parseTemplate(std::string_view content) {
        auto jtmp = nlohmann::ordered_json::parse(content);
        return land::internal::import_framework::ImportProcesser::parseTemplate(jtmp, this->source_path_);
    }

    std::string LoadSourceContent(const char* /* path */) override { return ""; }
    std::string LoadTemplateContent(const char* path) override {
        auto ct = del_editor::EditorBootstrap::LoadTemplateContent(path);
        if (auto def = this->parseTemplate(ct)) {
            this->SetSourceText(def->sources.dump(2)); // 动态解析模板，并生成源码
        } else {
            this->monitor_panel_.AddError(def.error().message());
        }
        return ct;
    }
    nlohmann::json ParseSourceContent(std::string_view /* content */) override {
        auto editor_content = this->editor_panel_.GetText();
        if (auto def = this->parseTemplate(editor_content)) {
            return def->sources;
        } else throw std::runtime_error(def.error().message());
    }
    nlohmann::ordered_json ParseTemplateContent(std::string_view content) override {
        if (auto def = this->parseTemplate(content)) {
            return def->landTmp;
        } else throw std::runtime_error(def.error().message());
    }
};


DelLiveEditorMenuElement::DelLiveEditorMenuElement(WindowManager& wm) : IMenuElement(DelEditor::kWindowName) {
    auto& wind = wm.create<DelEditor>(DelEditor::kWindowName, wm);
    this->setWindow(&wind);
}

} // namespace devtool::menus
