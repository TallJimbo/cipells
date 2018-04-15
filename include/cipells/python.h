#ifndef CIPELLS_python_h_INCLUDED
#define CIPELLS_python_h_INCLUDED

#include "pybind11/pybind11.h"

#include "cipells/fwd/XYTuple.h"
#include "cipells/utils/Deferrer.h"

namespace pybind11 { namespace detail {

// Custom type caster for tuple->Index2 implicit conversion.
// Defined in python/XYTuple.cc.
template <>
struct type_caster<cipells::Index2> : public type_caster_base<cipells::Index2> {
    using base = type_caster_base<cipells::Index2>;
public:
    bool load(handle src, bool convert);
};

// Custom type caster for tuple->Real2 implicit conversion.
// Defined in python/XYTuple.cc.
template <>
struct type_caster<cipells::Real2> : public type_caster_base<cipells::Real2> {
    using base = type_caster_base<cipells::Real2>;
public:
    bool load(handle src, bool convert);
};

}} // namespace pybind11::detail


namespace cipells {

utils::Deferrer pyXYTuple(pybind11::module & module);

utils::Deferrer pyInterval(pybind11::module & module);

utils::Deferrer pyBox(pybind11::module & module);

} // namespace cipells

#endif // !CIPELLS_python_h_INCLUDED