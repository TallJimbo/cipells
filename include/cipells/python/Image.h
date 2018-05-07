#ifndef CIPELLS_PYTHON_Image_h_INCLUDED
#define CIPELLS_PYTHON_Image_h_INCLUDED

#include "pybind11/pybind11.h"

#include "cipells/fwd/Image.h"

namespace cipells {

// Images are exposed to Python via this proxy, which holds an actual Pybind11-wrapped
// Image<T> inside its 'wrapped' member.
// Most of its Python methods delegate to that (just using Python calls, via the pybind11 Python API),
// but it also caches the array view on first use for fast repeated usage.
class PyImage {
public:

    explicit PyImage(pybind11::object const & wrapped_) : wrapped(wrapped_) {}

    pybind11::object wrapped;
    pybind11::object array;
};


} // namespace cipells


namespace pybind11 { namespace detail {

// Custom type caster that returns all Image objects by stuffing them in
// a PyImage proxy that provides a more Pythonic interface.  The reverse
// conversion is provided just by declaring PyImage implicitly convertible
// to Image.
template <typename T>
struct type_caster<cipells::Image<T>> {
private:
    using Base = type_caster_base<cipells::Image<T>>;
    using Proxy = type_caster_base<cipells::PyImage>;
public:

    static PYBIND11_DESCR name() { return Base::name(); }

    bool load(handle src, bool convert) {
        if (_base.load(src, convert)) {
            return true;
        }
        if (_proxy.load(src, convert)) {
            cipells::PyImage const * wrapper = _proxy;
            if (_base.load(wrapper->wrapped, convert)) {
                return true;
            }
        }
        return false;
    }

    operator cipells::Image<T> & () {
        return _base;
    }

    operator cipells::Image<T> * () {
        return _base;
    }

    static handle cast(cipells::Image<T> const * image, return_value_policy policy, handle parent) {
        handle wrapped = Base::cast(image, policy, parent);
        return Proxy::cast(cipells::PyImage(reinterpret_steal<object>(wrapped)),
                           return_value_policy::move, parent);
    }

    static handle cast(cipells::Image<T> const & image, return_value_policy policy, handle parent) {
        handle wrapped = Base::cast(image, policy, parent);
        return Proxy::cast(cipells::PyImage(reinterpret_steal<object>(wrapped)),
                           return_value_policy::move, parent);
    }

    static handle cast(cipells::Image<T> && image, return_value_policy policy, handle parent) {
        handle wrapped = Base::cast(std::move(image), policy, parent);
        return Proxy::cast(cipells::PyImage(reinterpret_steal<object>(wrapped)),
                           return_value_policy::move, parent);
    }

    template <typename T_>
    using cast_op_type = pybind11::detail::cast_op_type<T_>;

private:
    Base _base;
    Proxy _proxy;
};

}} // namespace pybind11::detail

#endif // !CIPELLS_PYTHON_Image_h_INCLUDED
