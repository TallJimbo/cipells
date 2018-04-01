#ifndef CIPELLS_FWD_XYTuple_h_INCLUDED
#define CIPELLS_FWD_XYTuple_h_INCLUDED

#include <cstdint>

namespace cipells {

using Index = std::int32_t;
using Real = double;

template <typename T>
class XYTuple;

using Index2 = XYTuple<Index>;
using Real2 = XYTuple<Real>;

} // namespace cipells

#endif // !CIPELLS_FWD_XYTuple_h_INCLUDED