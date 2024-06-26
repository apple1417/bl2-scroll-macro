#include "pch.h"

#include <unrealsdk/game/selector.h>
#include <unrealsdk/logging.h>
#include <unrealsdk/unrealsdk.h>

#include "achievements.h"
#include "binds.h"
#include "config.h"
#include "macro.h"
#include "menu.h"

namespace {

/**
 * @brief Main startup thread.
 * @note Instance of `LPTHREAD_START_ROUTINE`.
 *
 * @return unused.
 */
DWORD WINAPI startup_thread(LPVOID /*unused*/) {
    scroll::config::load();
    unrealsdk::logging::init(scroll::config::LOG_FILE);

    try {
        unrealsdk::init(&unrealsdk::game::select_based_on_executable);
    } catch (std::exception& ex) {
        LOG(ERROR, "Exception occurred while initializing the sdk: {}", ex.what());
    }

    try {
        scroll::binds::init();
        scroll::menu::init();
        scroll::achievements::init();
    } catch (std::exception& ex) {
        LOG(ERROR, "Exception occurred while initializing the macro: {}", ex.what());
    }

    try {
        scroll::macro::run();
    } catch (std::exception& ex) {
        LOG(ERROR, "Exception occurred while running the macro, stopping: {}", ex.what());
    }

    return 1;
}

}  // namespace

/**
 * @brief Main entry point.
 *
 * @param h_module Handle to module for this dll.
 * @param ul_reason_for_call Reason this is being called.
 * @return True if loaded successfully, false otherwise.
 */
// NOLINTNEXTLINE(readability-identifier-naming)  - for `DllMain`
BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID /*unused*/) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(h_module);
            CreateThread(nullptr, 0, &startup_thread, nullptr, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
