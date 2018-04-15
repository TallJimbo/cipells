#ifndef CIPELLS_FWD_Box_h_INCLUDED
#define CIPELLS_FWD_Box_h_INCLUDED

#include "cipells/common.h"

namespace cipells {

class RealBox;
class IndexBox;

namespace detail {

template <typename T>
struct SelectBox;

template <>
struct SelectBox<Real> {
    using Type = RealBox;
};

template <>
struct SelectBox<Index> {
    using Type = IndexBox;
};

} // namespace detail

template <typename T>
using Box = typename detail::SelectBox<T>::Type;

} // namespace cipells

#endif // !CIPELLS_FWD_Box_h_INCLUDED
