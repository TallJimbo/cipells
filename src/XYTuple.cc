#define CIPELLS_XYTuple_cc
#include "fmt/format.h"
#include "cipells/XYTuple.h"

namespace cipells {

namespace {

std::string getScalarName(Index *) {
    return "Index";
}

std::string getScalarName(Real *) {
    return "Real";
}

} // anonymous

template <typename T>
std::string XYTuple<T>::str() const {
    return fmt::format("(x={}, y={})", x(), y());
}

template <typename T>
std::string XYTuple<T>::repr() const {
    return fmt::format("{}2(x={}, y={})", getScalarName((T*)nullptr), x(), y());
}

template class XYTuple<Index>;
template class XYTuple<Real>;

} // namespace cipells
