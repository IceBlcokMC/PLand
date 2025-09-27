#pragma once
#include "PaginatedSimpleForm.h"
#include "pland/gui/form/BackSimpleForm.h"


namespace land {


namespace internal {


class PaginatedFormWrapperImpl {

    std::shared_ptr<PaginatedSimpleForm> impl;

public:
    PaginatedFormWrapperImpl() : impl(PaginatedSimpleForm::make()) {}

    PaginatedFormWrapperImpl& setTitle(std::string title) {
        impl->setTitle(std::move(title));
        return *this;
    }

    PaginatedFormWrapperImpl& setContent(std::string content) {
        impl->setContent(std::move(content));
        return *this;
    }

    // concept: HasAppendButtonMethods
    template <typename... Args>
    PaginatedFormWrapperImpl& appendButton(Args&&... args) {
        impl->appendButton(std::forward<Args>(args)...);
        return *this;
    }

    // concept: HasSendToMethod
    template <typename... Args>
    void sendTo(Args&&... args) {
        impl->sendTo(std::forward<Args>(args)...);
    }

    template <typename... Args>
    PaginatedFormWrapperImpl& onFormCanceled(Args&&... args) {
        impl->onFormCanceled(std::forward<Args>(args)...);
        return *this;
    }
};

static_assert(HasSendToMethod<PaginatedFormWrapperImpl>, "PaginatedFormWrapperImpl must satisfy HasSendToMethod");
static_assert(
    HasAppendButtonMethods<PaginatedFormWrapperImpl>,
    "PaginatedFormWrapperImpl must satisfy HasAppendButtonMethods"
);
static_assert(
    DisallowEnableSharedFromThis<PaginatedFormWrapperImpl>,
    "PaginatedFormWrapperImpl must not satisfy std::enable_shared_from_this<T>"
);

} // namespace internal


using BackPaginatedSimpleForm = BackSimpleForm<internal::PaginatedFormWrapperImpl>;


} // namespace land