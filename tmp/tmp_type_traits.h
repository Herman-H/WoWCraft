#ifndef TMP_TYPE_TRAITS_H
#define TMP_TYPE_TRAITS_H

#include <type_traits>

namespace tmp
{

    template <typename T>
    constexpr bool is_xvalue = !std::is_lvalue_reference<T>::value && std::is_rvalue_reference<T>::value;

    template <typename T>
    constexpr bool is_prvalue = !std::is_lvalue_reference<T>::value && !std::is_rvalue_reference<T>::value;


} // namespace tmp


#endif // TMP_TYPES_H
