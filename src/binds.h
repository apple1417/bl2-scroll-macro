#ifndef BINDS_H
#define BINDS_H

#include "pch.h"

namespace unrealsdk::unreal {

struct FName;
class BoundFunction;

}  // namespace unrealsdk::unreal

namespace scroll::binds {

enum class ScrollType { UP, DOWN };

/**
 * @brief Initalizes the binds module, loading defaults from the config file.
 */
void init(void);

/**
 * @brief Get the display name to use for one of the current binds.
 *
 * @param type Which type of bind to get.
 * @param localize A pointer to the `GetLocalizedKeyName` bound function to use for localization.
 *                 Ignored if null.
 * @return The key's display name.
 */
std::wstring get_bind_name(ScrollType type, unrealsdk::unreal::BoundFunction* localize);

/**
 * @brief Rebinds one of the macros.
 *
 * @param type Which type of bind to update.
 * @param key The new key.
 */
void rebind(ScrollType type, const unrealsdk::unreal::FName& key);

}  // namespace scroll::binds

#endif /* BINDS_H */
