#ifndef BINDS_H
#define BINDS_H

namespace unrealsdk::unreal {

struct FName;
class BoundFunction;

}  // namespace unrealsdk::unreal

namespace scroll::binds {

enum class ScrollType { UP, DOWN };

unrealsdk::unreal::FName get_bind(ScrollType type);

std::wstring get_bind_name(ScrollType type, const unrealsdk::unreal::BoundFunction* localize);

void change_bind(ScrollType type, const unrealsdk::unreal::FName& key);

void init(void);

}  // namespace scroll::binds

#endif /* BINDS_H */
