#include "pch.h"

#include <unrealsdk/format.h>
#include <unrealsdk/unreal/classes/properties/copyable_property.h>
#include <unrealsdk/unreal/classes/properties/ustrproperty.h>
#include <unrealsdk/unreal/structs/fname.h>
#include <unrealsdk/unreal/wrappers/bound_function.h>
#include <fstream>

#include "binds.h"

using namespace unrealsdk::unreal;

namespace scroll::binds {

namespace {

const constexpr auto UNBOUND = 0;

const std::unordered_map<DWORD, const wchar_t*> KEYCODE_TO_UNREAL = {
    // 0x00
    {UNBOUND, L"None"},  // Custom
    {VK_LBUTTON, L"LeftMouseButton"},
    {VK_RBUTTON, L"RightMouseButton"},
    // VK_CANCEL
    {VK_MBUTTON, L"MiddleMouseButton"},
    {VK_XBUTTON1, L"ThumbMouseButton"},
    {VK_XBUTTON2, L"ThumbMouseButton2"},
    // undefined
    {VK_BACK, L"BackSpace"},
    {VK_TAB, L"Tab"},
    // reserved
    // reserved
    // VK_CLEAR
    {VK_RETURN, L"Enter"},
    // undefined
    // undefined

    // 0x10
    // VK_SHIFT (using more specific forms)
    // VK_CONTROL (using more specific forms)
    // VK_MENU (using more specific forms)
    {VK_PAUSE, L"Pause"},
    {VK_CAPITAL, L"CapsLock"},
    // VK_KANA / VK_HANGUL
    // VK_IME_ON
    // VK_JUNJA
    // VK_FINAL
    // VK_HANJA
    // VK_KANJI
    // VK_IME_OF
    {VK_ESCAPE, L"Escape"},
    // VK_CONVERT
    // VK_NONCONVERT
    // VK_ACCEPT
    // VK_MODECHANGE

    // 0x20
    {VK_SPACE, L"SpaceBar"},
    {VK_PRIOR, L"PageUp"},
    {VK_NEXT, L"PageDown"},
    {VK_END, L"End"},
    {VK_HOME, L"Home"},
    {VK_LEFT, L"Left"},
    {VK_UP, L"Up"},
    {VK_RIGHT, L"Right"},
    {VK_DOWN, L"Down"},
    // VK_SELECT
    // VK_PRINT
    // VK_EXECUTE
    // VK_SNAPSHOT
    {VK_INSERT, L"Insert"},
    {VK_DELETE, L"Delete"},
    // VK_HELP

    // 0x30 - Digits are defined as the same as ASCII
    {'0', L"Zero"},
    {'1', L"One"},
    {'2', L"Two"},
    {'3', L"Three"},
    {'4', L"Four"},
    {'5', L"Five"},
    {'6', L"Six"},
    {'7', L"Seven"},
    {'8', L"Eight"},
    {'9', L"Nine"},
    // undefined
    // undefined
    // undefined
    // undefined
    // undefined
    // undefined

    // 0x40 - Letters are defined as the same as ASCII
    {'A', L"A"},
    {'B', L"B"},
    {'C', L"C"},
    {'D', L"D"},
    {'E', L"E"},
    {'F', L"F"},
    {'G', L"G"},
    {'H', L"H"},
    {'I', L"I"},
    {'J', L"J"},
    {'K', L"K"},
    {'L', L"L"},
    {'M', L"M"},
    {'N', L"N"},
    {'O', L"O"},

    // 0x50 - Letters are defined as the same as ascii
    {'P', L"P"},
    {'Q', L"Q"},
    {'R', L"R"},
    {'S', L"S"},
    {'T', L"T"},
    {'U', L"U"},
    {'V', L"V"},
    {'W', L"W"},
    {'X', L"X"},
    {'Y', L"Y"},
    {'Z', L"Z"},
    // VK_LWIN
    // VK_RWIN
    // VK_APPS
    // reserved
    // VK_SLEEP

    // 0x60
    {VK_NUMPAD0, L"NumPadZero"},
    {VK_NUMPAD1, L"NumPadOne"},
    {VK_NUMPAD2, L"NumPadTwo"},
    {VK_NUMPAD3, L"NumPadThree"},
    {VK_NUMPAD4, L"NumPadFour"},
    {VK_NUMPAD5, L"NumPadFive"},
    {VK_NUMPAD6, L"NumPadSix"},
    {VK_NUMPAD7, L"NumPadSeven"},
    {VK_NUMPAD8, L"NumPadEight"},
    {VK_NUMPAD9, L"NumPadNine"},
    {VK_MULTIPLY, L"Multiply"},
    {VK_ADD, L"Add"},
    // VK_SEPARATOR
    {VK_SUBTRACT, L"Subtract"},
    {VK_DECIMAL, L"Decimal"},
    {VK_DIVIDE, L"Divide"},

    // 0x70
    {VK_F1, L"F1"},
    {VK_F2, L"F2"},
    {VK_F3, L"F3"},
    {VK_F4, L"F4"},
    {VK_F5, L"F5"},
    {VK_F6, L"F6"},
    {VK_F7, L"F7"},
    {VK_F8, L"F8"},
    {VK_F9, L"F9"},
    {VK_F10, L"F10"},
    {VK_F11, L"F11"},
    {VK_F12, L"F12"},
    // VK_F13
    // VK_F14
    // VK_F15
    // VK_F16

    // 0x80
    // VK_F17
    // VK_F18
    // VK_F19
    // VK_F20
    // VK_F21
    // VK_F22
    // VK_F23
    // VK_F24
    // VK_NAVIGATION_VIEW
    // VK_NAVIGATION_MENU
    // VK_NAVIGATION_UP
    // VK_NAVIGATION_DOWN
    // VK_NAVIGATION_LEFT
    // VK_NAVIGATION_RIGHT
    // VK_NAVIGATION_ACCEPT
    // VK_NAVIGATION_CANCEL

    // 0x90
    {VK_NUMLOCK, L"NumLock"},
    {VK_SCROLL, L"ScrollLock"},
    // VK_OEM_NEC_EQUAL / VK_OEM_FJ_JISHO
    // VK_OEM_FJ_MASSHOU
    // VK_OEM_FJ_TOUROKU
    // VK_OEM_FJ_LOYA
    // VK_OEM_FJ_ROYA
    // unassigned
    // unassigned
    // unassigned
    // unassigned
    // unassigned
    // unassigned
    // unassigned
    // unassigned
    // unassigned

    // 0xA0
    {VK_LSHIFT, L"LeftShift"},
    {VK_RSHIFT, L"RightShift"},
    {VK_LCONTROL, L"LeftControl"},
    {VK_RCONTROL, L"RightControl"},
    {VK_LMENU, L"LeftAlt"},
    {VK_RMENU, L"RightAlt"},
    // VK_BROWSER_BACK
    // VK_BROWSER_FORWARD
    // VK_BROWSER_REFRESH
    // VK_BROWSER_STOP
    // VK_BROWSER_SEARCH
    // VK_BROWSER_FAVORITES
    // VK_BROWSER_HOME
    // VK_VOLUME_MUTE
    // VK_VOLUME_DOWN
    // VK_VOLUME_UP

    // 0xB0
    // K_MEDIA_NEXT_TRACK
    // VK_MEDIA_PREV_TRACK
    // VK_MEDIA_STOP
    // VK_MEDIA_PLAY_PAUSE
    // VK_LAUNCH_MAIL
    // VK_LAUNCH_MEDIA_SELECT
    // VK_LAUNCH_APP1
    // VK_LAUNCH_APP2
    // reserved
    // reserved
    {VK_OEM_1, L"Semicolon"},
    {VK_OEM_PLUS, L"Equals"},
    {VK_OEM_COMMA, L"Comma"},
    {VK_OEM_MINUS, L"Underscore"},
    {VK_OEM_PERIOD, L"Period"},
    {VK_OEM_2, L"Slash"},

    // 0xC0
    {VK_OEM_3, L"Tilde"},
    // reserved
    // reserved
    {VK_GAMEPAD_A, L"XboxTypeS_A"},
    {VK_GAMEPAD_B, L"XboxTypeS_B"},
    {VK_GAMEPAD_X, L"XboxTypeS_X"},
    {VK_GAMEPAD_Y, L"XboxTypeS_Y"},
    {VK_GAMEPAD_RIGHT_SHOULDER, L"XboxTypeS_RightShoulder"},
    {VK_GAMEPAD_LEFT_SHOULDER, L"XboxTypeS_LeftShoulder"},
    {VK_GAMEPAD_LEFT_TRIGGER, L"XboxTypeS_LeftTrigger"},
    {VK_GAMEPAD_RIGHT_TRIGGER, L"XboxTypeS_RightTrigger"},
    {VK_GAMEPAD_DPAD_UP, L"XboxTypeS_DPad_Up"},
    {VK_GAMEPAD_DPAD_DOWN, L"XboxTypeS_DPad_Down"},
    {VK_GAMEPAD_DPAD_LEFT, L"XboxTypeS_DPad_Left"},
    {VK_GAMEPAD_DPAD_RIGHT, L"XboxTypeS_DPad_Right"},
    {VK_GAMEPAD_MENU, L"XboxTypeS_Start"},  // Not 100% sure I don't have start/back swapped

    // 0xD0
    {VK_GAMEPAD_VIEW, L"XboxTypeS_Back"},
    {VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, L"XboxTypeS_LeftThumbStick"},
    {VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, L"XboxTypeS_RightThumbStick"},
    // VK_GAMEPAD_LEFT_THUMBSTICK_UP
    // VK_GAMEPAD_LEFT_THUMBSTICK_DOWN
    // VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT
    // VK_GAMEPAD_LEFT_THUMBSTICK_LEFT
    // VK_GAMEPAD_RIGHT_THUMBSTICK_UP
    // VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN
    // VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT
    // VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT
    {VK_OEM_4, L"LeftBracket"},
    {VK_OEM_5, L"Backslash"},
    {VK_OEM_6, L"RightBracket"},
    {VK_OEM_7, L"Quote"},
    // VK_OEM_8

    // 0xE0
    // reserved
    // VK_OEM_AX
    // VK_OEM_102
    // VK_ICO_HELP
    // VK_ICO_00
    // VK_PROCESSKEY
    // VK_ICO_CLEAR
    // VK_PACKET
    // unassigned
    // VK_OEM_RESET
    // VK_OEM_JUMP
    // VK_OEM_PA1
    // VK_OEM_PA2
    // VK_OEM_PA3
    // VK_OEM_WSCTRL
    // VK_OEM_CUSEL

    // 0xF0
    // VK_OEM_ATTN
    // VK_OEM_FINISH
    // VK_OEM_COPY
    // VK_OEM_AUTO
    // VK_OEM_ENLW
    // VK_OEM_BACKTAB
    // VK_ATTN
    // VK_CRSEL
    // VK_EXSEL
    // VK_EREOF
    // VK_PLAY
    // VK_ZOOM
    // VK_NONAME
    // VK_PA1
    // VK_OEM_CLEAR
    // reserved
};

// The inverse of the above, calculated at runtime
std::unordered_map<FName, DWORD> fname_to_keycode{};

DWORD up_keycode = 0;
DWORD down_keycode = 0;

/**
 * @brief Loads the configured keys from the config file.
 */
void load_config(void) {
    // Don't need anything special, it's just two ints
    std::ifstream stream("scroll-macro.cfg");
    stream >> up_keycode;
    stream >> down_keycode;
}

/**
 * @brief Writes the configured keys to the config file.
 */
void save_config(void) {
    std::ofstream stream("scroll-macro.cfg");
    stream << up_keycode << " " << down_keycode;
}

}  // namespace

void init(void) {
    load_config();

    for (const auto& pair : KEYCODE_TO_UNREAL) {
        fname_to_keycode[FName(pair.second)] = pair.first;
    }
}

std::wstring get_bind_name(ScrollType type, BoundFunction* localize) {
    auto keycode = type == ScrollType::UP ? up_keycode : down_keycode;

    // There are a few keys which display in dumb ways, override them
    switch (keycode) {
        case UNBOUND:
            return L"";
        case VK_OEM_7:  // "Quote"
            return L"'";
        case VK_OEM_5:      // "Backslash"
            return L"\\ ";  // The space is important for some reason
    }

    if (KEYCODE_TO_UNREAL.contains(keycode)) {
        auto name = KEYCODE_TO_UNREAL.at(keycode);
        if (localize == nullptr) {
            return name;
        }

        auto localized = localize->call<UStrProperty, UNameProperty>(FName(name));
        if (localized.starts_with(L"?INT?")) {
            LOG(ERROR, L"Failed to localize key '{}' / VK 0x{:x}", name, keycode);
            return name;
        }
        return localized;
    }

    // Give up
    return unrealsdk::fmt::format(L"VK 0x{:02x}", keycode);
}

void rebind(ScrollType type, const unrealsdk::unreal::FName& key) {
    if (!fname_to_keycode.contains(key)) {
        LOG(ERROR, "Encountered unknown key '{}', unable to bind to it!", key);
        return;
    }

    auto new_keycode = fname_to_keycode[key];
    auto current_keycode = (type == ScrollType::UP) ? &up_keycode : &down_keycode;

    // If bound to the same thing, unbind
    *current_keycode = new_keycode == *current_keycode ? UNBOUND : new_keycode;

    save_config();
}

}  // namespace scroll::binds
