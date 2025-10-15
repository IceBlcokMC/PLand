#include "IComponent.h"
#include <imgui.h>
#include <ranges>

namespace devtool {

IMenuElement::IMenuElement(std::string label) : label_(std::move(label)) {}
IMenuElement::IMenuElement(std::string label, std::string shortCut)
: label_(std::move(label)),
  shortCut_(std::move(shortCut)) {}

std::string const& IMenuElement::getLabel() const { return label_; }

std::string const& IMenuElement::getShortCut() const { return shortCut_; }

bool IMenuElement::isEnable() const { return enable_; }

bool IMenuElement::isSelected() const { return selected_; }

bool* IMenuElement::getEnableFlag() { return &enable_; }

bool* IMenuElement::getSelectFlag() { return &selected_; }

void IMenuElement::render() { ImGui::MenuItem(label_.data(), shortCut_.data(), getSelectFlag()); }


IMenu::IMenu(std::string label) : label_(std::move(label)) {}

void IMenu::registerElementImpl(std::unique_ptr<IMenuElement> element) {
    this->elements_.emplace(element->getLabel(), std::move(element));
}

std::string const& IMenu::getLabel() const { return label_; }

bool IMenu::isEnable() const { return enable_; }

bool* IMenu::getEnableFlag() { return &enable_; }

void IMenu::render() {
    for (auto const& element : elements_ | std::views::values) {
        if (ImGui::BeginMenu(label_.data(), getEnableFlag())) {
            element->render();
            ImGui::EndMenu();
        }
    }
}

void IMenu::tick() {
    for (auto const& element : elements_ | std::views::values) {
        element->tick();
    }
}


bool* IWindow::getOpenFlag() { return &open_; }

bool IWindow::isOpen() const { return open_; }

void IWindow::setOpenFlag(bool open) { open_ = open; }

void IWindow::tick() {
    if (this->isOpen()) {
        this->render();
    }
}


} // namespace devtool