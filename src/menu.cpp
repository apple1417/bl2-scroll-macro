#include "pch.h"

#include <unrealsdk/hook_manager.h>
#include <unrealsdk/unreal/classes/properties/copyable_property.h>
#include <unrealsdk/unreal/classes/properties/uarrayproperty.h>
#include <unrealsdk/unreal/classes/properties/uobjectproperty.h>
#include <unrealsdk/unreal/classes/properties/ustrproperty.h>
#include <unrealsdk/unreal/classes/properties/ustructproperty.h>
#include <unrealsdk/unreal/classes/ufunction.h>
#include <unrealsdk/unreal/classes/uobject.h>
#include <unrealsdk/unreal/classes/uobject_funcs.h>
#include <unrealsdk/unreal/structs/fname.h>
#include <unrealsdk/unreal/wrappers/bound_function.h>
#include <unrealsdk/unreal/wrappers/wrapped_array.h>
#include <unrealsdk/unreal/wrappers/wrapped_struct.h>
#include <unrealsdk/unrealsdk.h>

#include "binds.h"
#include "config.h"

using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

namespace scroll::menu {

namespace {

const constexpr auto TOOLTIP_PATH = L"menu.inner.tooltips.htmlText";

const constexpr auto HOOK_KEY = L"ScrollMacro";

const constexpr auto POPULATE_KEYS_FUNC =
    L"WillowGame.WillowScrollingListDataProviderKeyboardMouseOptions:extOnPopulateKeys";
const constexpr auto BIND_KEY_FUNC =
    L"WillowGame.WillowScrollingListDataProviderKeyboardMouseOptions:BindCurrentSelection";
const constexpr auto UPDATE_TOOLTIPS_FUNC = L"WillowGame.FrontendGFxMovie:UpdateTooltips";

const constexpr auto SCROLL_UP_CAPTION = L"Scroll Up Macro";
const constexpr auto SCROLL_DOWN_CAPTION = L"Scroll Down Macro";

/**
 * @brief Hook called after the keybinds list is populated, which we use to add our own binds.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool post_populate_keys(Details& hook) {
    auto add_keybind_entry = hook.obj->get<UFunction, BoundFunction>(L"AddKeyBindEntry"_fn);
    auto add_key_data = hook.obj->get<UObjectProperty>(L"ControllerMappingClip"_fn)
                            ->get<UFunction, BoundFunction>(L"AddKeyData"_fn);
    auto localize = hook.obj->get<UFunction, BoundFunction>(L"GetLocalizedKeyName"_fn);

    auto keybind_list = hook.obj->get<UArrayProperty>(L"KeyBinds"_fn);

    auto inject_key = [&](FName tag, const std::wstring& caption, binds::ScrollType type) {
        auto idx = add_keybind_entry.call<UIntProperty, UNameProperty, UNameProperty, UStrProperty>(
            tag, tag, caption);

        auto obj = add_key_data.call<UObjectProperty, UStrProperty, UStrProperty, UStrProperty>(
            tag, caption, binds::get_bind_name(type, &localize));

        auto bind = keybind_list.get_at<UStructProperty>(idx);
        // Set our modded binds to a dummy value to make sure they'll never get swapped
        // We track display name separately
        bind.set<UNameProperty>(L"CurrentKey"_fn, L"DUMMY KEY"_fn);
        bind.set<UObjectProperty>(L"Object"_fn, obj);
    };

    inject_key(L"scroll.macro.up"_fn, SCROLL_UP_CAPTION, binds::ScrollType::UP);
    inject_key(L"scroll.macro.down"_fn, SCROLL_DOWN_CAPTION, binds::ScrollType::DOWN);

    return false;
}

/**
 * @brief Hook called when a keybind is rebound, which we use to track our custom binds.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool on_bind_key(Details& hook) {
    const auto keybinds = hook.obj->get<UArrayProperty>(L"KeyBinds"_fn);

    const auto scroll_up_idx = keybinds.size() - 2;
    const auto min_modded_bind_idx = scroll_up_idx;

    const auto selected_idx = (size_t)hook.obj->get<UIntProperty>(L"CurrentKeyBindSelection"_fn);

    auto new_key = hook.args->get<UNameProperty>(L"Key"_fn);

    // If this is a standard bind, let the base function handle the rest
    if (selected_idx < min_modded_bind_idx) {
        return false;
    }

    // If this is a modded bind, Set it to whatever requested, without checking for conflicts.

    auto type = selected_idx == scroll_up_idx ? binds::ScrollType::UP : binds::ScrollType::DOWN;
    binds::rebind(type, new_key);

    // Update the display name
    // Don't set `bNeedsToSaveKeyBinds`, or change the actual dummy key we stored
    auto localize = hook.obj->get<UFunction, BoundFunction>(L"GetLocalizedKeyName"_fn);

    keybinds.get_at<UStructProperty>(selected_idx)
        .get<UObjectProperty>(L"Object"_fn)
        ->get<UFunction, BoundFunction>(L"SetString"_fn)
        .call<void, UStrProperty, UStrProperty>(L"value", binds::get_bind_name(type, &localize));
    hook.obj->get<UObjectProperty>(L"ControllerMappingClip"_fn)
        ->get<UFunction, BoundFunction>(L"InvalidateKeyData"_fn)
        .call<void>();

    return true;
}

/**
 * @brief Hook called to add tooltips to the main menu. We replace them with our own.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool on_update_tooltips(Details& hook) {
    hook.obj->get<UFunction, BoundFunction>(L"SetVariableString"_fn)
        .call<void, UStrProperty, UStrProperty>(TOOLTIP_PATH, USING_MACRO_TOOLTIP);

    return true;
}

}  // namespace

void init(void) {
    add_hook(POPULATE_KEYS_FUNC, Type::POST, HOOK_KEY, post_populate_keys);
    add_hook(BIND_KEY_FUNC, Type::PRE, HOOK_KEY, on_bind_key);
    add_hook(UPDATE_TOOLTIPS_FUNC, Type::PRE, HOOK_KEY, on_update_tooltips);
}

}  // namespace scroll::menu
