#pragma once
#include "components/IComponent.h"
#include <GLFW/glfw3.h>
#include <concepts>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace devtool {

class DevToolApp final {
public:
    explicit DevToolApp();
    ~DevToolApp();

    DevToolApp(DevToolApp&)                  = delete;
    DevToolApp& operator=(const DevToolApp&) = delete;

public:
    void show() const;

    void hide() const;

    void appendError(std::string msg);

    template <typename T, typename... Args>
        requires std::derived_from<T, IMenu> && std::is_final_v<T>
    void registerMenu(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        menus_.emplace_back(std::move(ptr));
    }

    [[nodiscard]] static std::unique_ptr<DevToolApp> make();

private:
    void render();                     // 核心渲染 (renderErrors、renderMainMenuBar、postTick)
    void renderErrors() const;         // 渲染错误信息
    void renderMainMenuBar() const;    // 渲染主菜单栏 (renderAppOptionsMenu、renderFramerateInfo)
    void renderFramerateInfo() const;  // 渲染帧率信息
    void renderAppOptionsMenu() const; // 渲染应用选项菜单
    void postTick() const;

    void checkAndUpdateScale();

    void initImGuiAndOpenGlWithGLFW();


    GLFWwindow*                         glfwWindow_{nullptr};
    std::thread                         renderThread_;
    std::atomic<bool>                   renderThreadStopFlag_;
    std::vector<std::string>            errors_;
    std::vector<std::unique_ptr<IMenu>> menus_;
};


namespace internals {

void handleGlfwError(int error, const char* description);

void handleWindowClose(GLFWwindow* window);

} // namespace internals


} // namespace devtool
