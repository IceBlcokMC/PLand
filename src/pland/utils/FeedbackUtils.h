#pragma once
#include <string>

#include "fmt/format.h"

#include <ll/api/form/ModalForm.h>
#include <ll/api/i18n/I18n.h>

#include "mc/network/packet/ToastRequestPacket.h"
#include <mc/network/packet/SetTitlePacket.h>
#include <mc/network/packet/TextPacket.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/world/actor/player/Player.h>

inline ToastRequestPacket::ToastRequestPacket()               = default;
inline ToastRequestPacketPayload::ToastRequestPacketPayload() = default;

inline SetTitlePacket::SetTitlePacket()               = default;
inline SetTitlePacketPayload::SetTitlePacketPayload() = default;

namespace land ::feedback_utils {

template <typename... Args>
inline std::string fmt_str(std::string_view fmt, Args&&... args) noexcept {
    try {
        return fmt::vformat(fmt, fmt::make_format_args(args...));
    } catch (...) {
        return fmt.data();
    }
}

// 普通聊天栏消息 (聊天栏)
template <typename... Args>
void sendText(Player& p, std::string_view fmt, Args&&... args) {
    p.sendMessage("§b[PLand] §r" + fmt_str(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void sendText(CommandOutput& output, std::string_view fmt, Args&&... args) {
    output.success("§b[PLand] §r" + fmt_str(fmt, std::forward<Args>(args)...));
}

// 红色报错消息 (聊天栏)
template <typename... Args>
void sendErrorText(Player& p, std::string_view fmt, Args&&... args) {
    p.sendMessage("§b[PLand] §c" + fmt_str(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void sendErrorText(CommandOutput& output, std::string_view fmt, Args&&... args) {
    output.error("§b[PLand] §c" + fmt_str(fmt, std::forward<Args>(args)...));
}

inline void sendError(Player& p, ll::Error const& err) { sendErrorText(p, err.message()); }
inline void sendError(CommandOutput& output, ll::Error const& error) { sendErrorText(output, error.message()); }


// 文本提示 (物品栏上方)
template <typename... Args>
void sendTextTip(Player& p, std::string_view fmt, Args&&... args) {
    TextPacket pkt{};
    pkt.mBody = TextPacketPayload::MessageOnly{TextPacketType::Tip, fmt_str(fmt, std::forward<Args>(args)...)};
    pkt.sendTo(p);
}

// ActionBar 提示 (物品栏上方)
template <typename... Args>
void sendActionBar(Player& p, std::string_view fmt, Args&&... args) {
    SetTitlePacket pkt{};
    pkt.mType      = SetTitlePacket::TitleType::Actionbar;
    pkt.mTitleText = fmt_str(fmt, std::forward<Args>(args)...);
    pkt.sendTo(p);
}

// Toast 弹窗 (成就消息)
template <typename... Args>
inline void sendToast(Player& p, std::string_view fmt, Args&&... args) {
    ToastRequestPacket pkt{};
    pkt.mTitle   = "§b[PLand]§r";
    pkt.mContent = fmt_str(fmt, std::forward<Args>(args)...);
    pkt.sendTo(p);
}

// Title 大字 (大标题、小标题)
inline void sendTitle(Player& p, std::string const& mainTitle, std::string const& subTitle = "") {
    SetTitlePacket pkt{};
    pkt.mType      = SetTitlePacket::TitleType::Title;
    pkt.mTitleText = mainTitle;
    pkt.sendTo(p);
    if (!subTitle.empty()) {
        pkt.mType      = SetTitlePacket::TitleType::Subtitle;
        pkt.mTitleText = subTitle;
        pkt.sendTo(p);
    }
}

using RetryCallback = std::function<void(Player&)>;
inline void askRetry(Player& p, std::string const& content, RetryCallback retry, RetryCallback cancel = nullptr) {
    using ll::i18n_literals::operator""_trl;
    auto localeCode = p.getLocaleCode();
    ll::form::ModalForm{
        "§bPLand§r - §c操作失败"_trl(localeCode),
        content,
        "§a重试"_trl(localeCode),
        "§c放弃/取消"_trl(localeCode),
    }
        .sendTo(
            p,
            [retry  = std::move(retry),
             cancel = std::move(cancel)](Player& player, ll::form::ModalFormResult const& result, auto) {
                if (result && (bool)result.value()) {
                    retry(player);
                } else {
                    if (cancel) cancel(player);
                }
            }
        );
}

inline void askRetry(Player& player, ll::Error const& error, RetryCallback retry, RetryCallback cancel = nullptr) {
    askRetry(player, error.message(), std::move(retry), std::move(cancel));
}

template <typename... Args>
void notifySuccess(Player& p, std::string const& fmt, Args&&... args) {
    std::string content = fmt_str(fmt, std::forward<Args>(args)...);
    sendToast(p, "§a{}§r", content);
    p.sendMessage(fmt_str("§7[PLand] {}", content));
}

} // namespace land::feedback_utils