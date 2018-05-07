#ifndef CIPELLS_PYTHON_XYTuple_h_INCLUDED
#define CIPELLS_PYTHON_XYTuple_h_INCLUDED

#include "pybind11/pybind11.h"

#include "cipells/XYTuple.h"


namespace pybind11 { namespace detail {

// Custom type caster that adds tuple->XYTuple implicit conversion.
template <typename T>
struct type_caster<cipells::XYTuple<T>> {
private:
    using Base = type_caster_base<cipells::XYTuple<T>>;
public:

    static PYBIND11_DESCR name() { return Base::name(); }

    bool load(handle src, bool convert) {
        if (_base.load(src, convert)) {
            return true;
        }
        if (convert && isinstance<tuple>(src)) {
            tuple t = reinterpret_borrow<tuple>(src);
            if (t.size() != 2u) {
                PyErr_SetString(PyExc_ValueError, "Only 2-element tuples are interoperable with XYTuple.");
                throw error_already_set();
            }
            T x = pybind11::cast<T>(t[0]);
            T y = pybind11::cast<T>(t[1]);
            _value = cipells::XYTuple<T>(x, y);
            _is_value_set = true;
            return true;
        }
        return false;
    }

    template <typename U>
    static handle cast(U && src, return_value_policy policy, handle parent) {
        return Base::cast(std::forward<U>(src), policy, parent);
    }

    operator cipells::XYTuple<T> & () {
        if (_is_value_set) {
            return _value;
        }
        return _base;
    }

    operator cipells::XYTuple<T> * () {
        if (_is_value_set) {
            return &_value;
        }
        return _base;
    }

    template <typename T_> using cast_op_type = pybind11::detail::cast_op_type<T_>;

private:
    bool _is_value_set = false;
    cipells::XYTuple<T> _value;   // TODO: could be std::optional in C++17.
    Base _base;
};

}} // namespace pybind11::detail

#endif // !CIPELLS_PYTHON_XYTuple_h_INCLUDED
