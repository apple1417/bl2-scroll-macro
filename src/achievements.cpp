#include "pch.h"

#include <unrealsdk/hook_manager.h>
#include <unrealsdk/unreal/classes/properties/copyable_property.h>
#include <unrealsdk/unreal/classes/properties/uarrayproperty.h>
#include <unrealsdk/unreal/classes/properties/uboolproperty.h>
#include <unrealsdk/unreal/classes/properties/uobjectproperty.h>
#include <unrealsdk/unreal/classes/properties/ustrproperty.h>
#include <unrealsdk/unreal/classes/ufunction.h>
#include <unrealsdk/unreal/classes/uobject.h>
#include <unrealsdk/unreal/classes/uobject_funcs.h>
#include <unrealsdk/unreal/structs/fname.h>
#include <unrealsdk/unreal/wrappers/bound_function.h>
#include <unrealsdk/unreal/wrappers/wrapped_array.h>
#include <unrealsdk/unreal/wrappers/wrapped_struct.h>

#include "config.h"

using namespace unrealsdk::hook_manager;
using namespace unrealsdk::unreal;

namespace scroll::achievements {

namespace {

const constexpr std::wstring_view HOOK_IDENTIFIER = L"ScrollMacro";

const constexpr std::wstring_view SHOW_ACHIEVEMENTS_FUNC =
    L"WillowGame.WillowGFxMovie:ShowAchievementsUI";
const constexpr std::wstring_view POPULATE_GAME_OPTIONS_FUNC =
    L"WillowGame.WillowScrollingListDataProviderGameOptions:Populate";
const constexpr std::wstring_view HANDLE_SPINNER_CHANGE_FUNC =
    L"WillowGame.WillowScrollingListDataProviderGameOptions:HandleSpinnerChange";

const constexpr auto ACHIEVEMENTS_EVENT_ID = 14171417;

/**
 * @brief Hook called to try show achievements - we block it if needed.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool show_achievements_hook(Details& /*hook*/) {
    return config::suppress_achievements;
}

/**
 * @brief Hook called when the gameplay options menu is populated, used to inject our own.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool populate_game_options_hook(Details& hook) {
    auto add_spinner = hook.args->get<UObjectProperty>(L"TheList"_fn)
                           ->get<UFunction, BoundFunction>(L"AddSpinnerListItem"_fn);

    WrappedStruct args{add_spinner.func};
    args.set<UIntProperty>(L"EventID"_fn, ACHIEVEMENTS_EVENT_ID);
    args.set<UStrProperty>(L"Caption"_fn, L"SUPRESS ACHIEVEMENTS");
    args.set<UBoolProperty>(L"bDisabled"_fn, false);
    args.set<UIntProperty>(L"StartingChoiceIndex"_fn, config::suppress_achievements ? 1 : 0);
    auto choices = args.get<UArrayProperty>(L"Choices"_fn);
    choices.resize(2);
    choices.set_at<UStrProperty>(0, L"Off");
    choices.set_at<UStrProperty>(1, L"On");

    add_spinner.call<void>(args);

    return false;
}

/**
 * @brief Hook called after the gameplay options menu is populated, used to inject our descriptions.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool post_populate_game_options_hook(Details& hook) {
    hook.obj->get<UFunction, BoundFunction>(L"AddDescription"_fn)
        .call<void, UIntProperty, UStrProperty>(
            ACHIEVEMENTS_EVENT_ID,
            L"When enabled, clicking the achievements button will simply do nothing.");
    return false;
}

/**
 * @brief Hook called when spinners are changed, used to detect our option bein edited.
 *
 * @param hook Hooked function details.
 * @return True if to block execution.
 */
bool handle_spinner_change_hook(Details& hook) {
    if (hook.args->get<UIntProperty>(L"EventID"_fn) == ACHIEVEMENTS_EVENT_ID) {
        auto new_idx = hook.args->get<UIntProperty>(L"NewChoiceIndex"_fn);
        config::suppress_achievements = new_idx != 0;
        config::save();
        return true;
    }

    return false;
}

}  // namespace

void init(void) {
    add_hook(SHOW_ACHIEVEMENTS_FUNC, Type::PRE, HOOK_IDENTIFIER, &show_achievements_hook);
    add_hook(POPULATE_GAME_OPTIONS_FUNC, Type::PRE, HOOK_IDENTIFIER, &populate_game_options_hook);
    add_hook(POPULATE_GAME_OPTIONS_FUNC, Type::POST, HOOK_IDENTIFIER,
             &post_populate_game_options_hook);
    add_hook(HANDLE_SPINNER_CHANGE_FUNC, Type::PRE, HOOK_IDENTIFIER, &handle_spinner_change_hook);
}

}  // namespace scroll::achievements
