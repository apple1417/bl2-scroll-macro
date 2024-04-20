#include "pch.h"

#include "config.h"

namespace scroll::config {

DWORD up_keycode = 0;
DWORD down_keycode = 0;
bool suppress_achievements = true;

void load(void) {
    std::ifstream stream{CONFIG_FILE};
    stream >> up_keycode;
    stream >> down_keycode;
    stream >> suppress_achievements;
}

void save(void) {
    std::ofstream stream(CONFIG_FILE);
    stream << up_keycode << " " << down_keycode << " " << suppress_achievements;
}

}  // namespace scroll::config
