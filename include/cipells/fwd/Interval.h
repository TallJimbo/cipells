#ifndef CIPELLS_FWD_Interval_h_INCLUDED
#define CIPELLS_FWD_Interval_h_INCLUDED

#include "cipells/fwd/XYTuple.h"

namespace cipells {

class RealInterval;
class IndexInterval;

namespace detail {

template <typename T>
struct SelectInterval;

template <>
struct SelectInterval<Real> {
    using Type = RealInterval;
};

template <>
struct SelectInterval<Index> {
    using Type = IndexInterval;
};

} // namespace detail

template <typename T>
using Interval = typename detail::SelectInterval<T>::Type;

} // namespace cipells

#endif // !CIPELLS_FWD_Interval_h_INCLUDED
