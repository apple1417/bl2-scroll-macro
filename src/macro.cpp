#include "pch.h"

#include <unrealsdk/format.h>

#include "binds.h"
#include "config.h"
#include "macro.h"

// This file is heavily based on:
// https://github.com/henyK/doom-eternal-macro/blob/master/main.cpp

namespace scroll::macro {

namespace {

bool spamming_up = false;
bool spamming_down = false;

HHOOK mouse_hook{};
HHOOK keyboard_hook{};

/**
 * @brief Updates the spamming flags with a new key event.
 *
 * @param key_code The key code of the event.
 * @param is_press True if the event was a press, false if it was a release.
 */
void update_spamming_state(DWORD key_code, bool is_press) {
    if (key_code == binds::get_key_code(binds::ScrollType::UP)) {
        spamming_up = is_press;
    }

    if (key_code == binds::get_key_code(binds::ScrollType::DOWN)) {
        spamming_down = is_press;
    }
}

/**
 * @brief Mouse hook used to detect macro presses.
 *
 * @param code A code the hook procedure uses to determine how to process the message.
 * @param w_param The identifier of the mouse message.
 * @param l_param A pointer to an MSLLHOOKSTRUCT structure.
 * @return 1 if the hook was blocked, otherwise what the next hook returned.
 */
LRESULT CALLBACK low_level_mouse_proc(int code, WPARAM w_param, LPARAM l_param) {
    if (code == HC_ACTION) {
        DWORD key_code = 0;
        bool is_press = false;

        switch (w_param) {
            case WM_LBUTTONDOWN: {
                is_press = true;
                [[fallthrough]];
            }
            case WM_LBUTTONUP: {
                key_code = VK_LBUTTON;
                break;
            }

            case WM_RBUTTONDOWN: {
                is_press = true;
                [[fallthrough]];
            }
            case WM_RBUTTONUP: {
                key_code = VK_RBUTTON;
                break;
            }

            case WM_MBUTTONDOWN: {
                is_press = true;
                [[fallthrough]];
            }
            case WM_MBUTTONUP: {
                key_code = VK_MBUTTON;
                break;
            }

            case WM_XBUTTONDOWN: {
                is_press = true;
                [[fallthrough]];
            }
            case WM_XBUTTONUP: {
                auto info = *reinterpret_cast<MSLLHOOKSTRUCT*>(l_param);
                key_code = VK_XBUTTON1 + HIWORD(info.mouseData) - XBUTTON1;
                break;
            }
        }

        update_spamming_state(key_code, is_press);
    }

    return CallNextHookEx(mouse_hook, code, w_param, l_param);
}

enum class KeyRepeatState { NOT_PRESSED, FIRST_PRESS, HELD };

KeyRepeatState up_repeat_state;
KeyRepeatState down_repeat_state;

/**
 * @brief Keyboard hook used to detect macro presses.
 *
 * @param code A code the hook procedure uses to determine how to process the message.
 * @param w_param The identifier of the keyboard message.
 * @param l_param A pointer to a KBDLLHOOKSTRUCT structure.
 * @return 1 if the hook was blocked, otherwise what the next hook returned.
 */
LRESULT CALLBACK low_level_keyboard_proc(int code, WPARAM w_param, LPARAM l_param) {
    if (code == HC_ACTION) {
        auto info = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

        auto up_code = binds::get_key_code(binds::ScrollType::UP);
        auto down_code = binds::get_key_code(binds::ScrollType::DOWN);

        bool is_press = false;

        switch (w_param) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                is_press = true;
                if (info.vkCode == up_code) {
                    up_repeat_state = up_repeat_state == KeyRepeatState::NOT_PRESSED
                                          ? KeyRepeatState::FIRST_PRESS
                                          : KeyRepeatState::HELD;
                } else if (info.vkCode == down_code) {
                    down_repeat_state = down_repeat_state == KeyRepeatState::NOT_PRESSED
                                            ? KeyRepeatState::FIRST_PRESS
                                            : KeyRepeatState::HELD;
                }
                break;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP: {
                is_press = false;
                if (info.vkCode == up_code) {
                    up_repeat_state = KeyRepeatState::NOT_PRESSED;
                } else if (info.vkCode == down_code) {
                    down_repeat_state = KeyRepeatState::NOT_PRESSED;
                }
                break;
            }
        }

        if ((info.vkCode == up_code && up_repeat_state != KeyRepeatState::HELD)
            || (info.vkCode == down_code && down_repeat_state != KeyRepeatState::HELD)) {
            update_spamming_state(info.vkCode, is_press);
        }
    }

    return CallNextHookEx(keyboard_hook, code, w_param, l_param);
}

/**
 * @brief Timer callback used to inject scroll events.
 */
void CALLBACK timer_proc(UINT /*u_timer_id*/,
                         UINT /*u_msg*/,
                         DWORD_PTR /*dw_user*/,
                         DWORD_PTR /*dw1*/,
                         DWORD_PTR /*dw2*/) {
    static const DWORD our_pid = GetCurrentProcessId();

    DWORD focused_pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &focused_pid);

    if (our_pid == focused_pid) {
        if (spamming_up) {
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA, 0);
        }
        if (spamming_down) {
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, (DWORD)-WHEEL_DELTA, 0);
        }

    } else {
        spamming_up = false;
        spamming_down = false;
    }
}

}  // namespace

void run(void) {
    mouse_hook = SetWindowsHookExW(WH_MOUSE_LL, low_level_mouse_proc, nullptr, 0);
    if (mouse_hook == nullptr) {
        throw std::runtime_error(
            unrealsdk::fmt::format("Failed to start mouse hook: {}", GetLastError()));
    }

    keyboard_hook = SetWindowsHookExW(WH_KEYBOARD_LL, low_level_keyboard_proc, nullptr, 0);
    if (keyboard_hook == nullptr) {
        throw std::runtime_error(
            unrealsdk::fmt::format("Failed to start keyboard hook: {}", GetLastError()));
    }

    timeSetEvent(SPAM_TIMER_MS, 0, timer_proc, 0, TIME_PERIODIC);
    while (true) {
        MSG message;
        while (GetMessageW(&message, nullptr, 0, 0) != 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
}

}  // namespace scroll::macro
