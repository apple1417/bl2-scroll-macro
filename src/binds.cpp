#include "pch.h"

#include "binds.h"
#include "unrealsdk/unreal/structs/fname.h"

using namespace unrealsdk::unreal;

namespace scroll::binds {

FName up_bind{0, 0};
FName down_bind{0, 0};

FName get_bind(ScrollType type) {
    return type == ScrollType::UP ? up_bind : down_bind;
}

std::wstring get_bind_name(ScrollType type, const unrealsdk::unreal::BoundFunction* localize) {
    (void)localize;
    return type == ScrollType::UP ? up_bind : down_bind;
}

void change_bind(ScrollType type, const unrealsdk::unreal::FName& key) {
    if (type == ScrollType::UP) {
        up_bind = key;
    } else {
        down_bind = key;
    }
}

}  // namespace scroll::binds
