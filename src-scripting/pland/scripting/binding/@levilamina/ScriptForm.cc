#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"

namespace land::scripting {


using ll::form::CustomForm;
decltype(Modules::kScriptCustomForm) Modules::kScriptCustomForm = //
    jspp::binding::defClass<CustomForm>("CustomForm")
        .ctor<>()
        .ctor<std::string>()
        .method("setTitle", &CustomForm::setTitle)
        .method("setSubmitButton", &CustomForm::setSubmitButton)
        .method("appendHeader", &CustomForm::appendHeader)
        .method("appendLabel", &CustomForm::appendLabel)
        .method("appendDivider", &CustomForm::appendDivider)
        .method("appendInput", &CustomForm::appendInput)
        .method("appendToggle", &CustomForm::appendToggle)
        .method("appendDropdown", &CustomForm::appendDropdown)
        .method("appendSlider", &CustomForm::appendSlider)
        .method("appendStepSlider", &CustomForm::appendStepSlider)
        .method("sendTo", &CustomForm::sendTo)
        .method("sendUpdate", &CustomForm::sendUpdate)
        .method("getFormData", &CustomForm::getFormData)
        .build();

using ll::form::ModalForm;
decltype(Modules::kScriptModalForm) Modules::kScriptModalForm = //
    jspp::binding::defClass<ModalForm>("ModalForm")
        .ctor<>()
        .method("setTitle", &ModalForm::setTitle)
        .method("setContent", &ModalForm::setContent)
        .method("setUpperButton", &ModalForm::setUpperButton)
        .method("setLowerButton", &ModalForm::setLowerButton)
        .method("sendTo", &ModalForm::sendTo)
        .method("sendUpdate", &ModalForm::sendUpdate)
        .build();

decltype(Modules::kScriptModalFormSelectedButton) Modules::kScriptModalFormSelectedButton = //
    helper::auto_gen_enum_def<ll::form::ModalFormSelectedButton>("ModalFormSelectedButton");

using ll::form::SimpleForm;
decltype(Modules::kScriptSimpleForm) Modules::kScriptSimpleForm = //
    jspp::binding::defClass<SimpleForm>("SimpleForm")
        .ctor<>()
        .method("setTitle", &SimpleForm::setTitle)
        .method("setContent", &SimpleForm::setContent)
        .method("appendHeader", &SimpleForm::appendHeader)
        .method("appendLabel", &SimpleForm::appendLabel)
        .method("appendDivider", &SimpleForm::appendDivider)
        .method(
            "appendButton",
            static_cast<SimpleForm& (SimpleForm::*)(std::string const&,
                                                    std::string const&,
                                                    std::string const&,
                                                    SimpleForm::ButtonCallback)>(&SimpleForm::appendButton),
            static_cast<SimpleForm& (SimpleForm::*)(std::string const&, SimpleForm::ButtonCallback)>(
                &SimpleForm::appendButton
            )
        )
        .method("sendTo", &SimpleForm::sendTo)
        .method("sendUpdate", &SimpleForm::sendUpdate)
        .build();

} // namespace land::scripting