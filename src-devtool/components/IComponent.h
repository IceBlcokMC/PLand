#pragma once
#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>

namespace devtool {

class IComponent {
public:
    virtual ~IComponent() = default;

    virtual void render() = 0;

    virtual void tick() = 0;
};


class IMenuElement : public IComponent {
protected:
    std::string label_;
    std::string shortCut_;
    bool        enable_{true};
    bool        selected_{false};

public:
    explicit IMenuElement(std::string label);
    explicit IMenuElement(std::string label, std::string shortCut);

    virtual std::string const& getLabel() const;

    virtual std::string const& getShortCut() const;

    virtual bool isEnable() const;

    virtual bool isSelected() const;

    virtual bool* getEnableFlag();

    virtual bool* getSelectFlag();

    void render() override;
};

class IMenu : public IComponent {
protected:
    std::unordered_map<std::string, std::unique_ptr<IMenuElement>> elements_;
    std::string                                                    label_;
    bool                                                           enable_{true};

    void registerElementImpl(std::unique_ptr<IMenuElement> element);

public:
    explicit IMenu(std::string label);

    virtual std::string const& getLabel() const;

    virtual bool isEnable() const;

    virtual bool* getEnableFlag();

    // this->render -> element->render
    void render() override;

    // 向 element 广播 tick
    virtual void tick() override;

    template <typename T, typename... Args>
        requires std::derived_from<T, IMenuElement> && std::is_final_v<T>
    void registerElement(Args&&... args) {
        registerElementImpl(std::make_unique<T>(std::forward<Args>(args)...));
    }
};


class IWindow : IComponent {
    bool open_{false};

public:
    virtual bool isOpen() const;

    virtual bool* getOpenFlag();

    virtual void setOpenFlag(bool open);

    // open_ 为 true 时执行渲染
    virtual void tick() override;
};


} // namespace devtool
