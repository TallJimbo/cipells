#include "cipells/python.h"

PYBIND11_MODULE(_cipells, m) {
    auto pyXYTuple = cipells::pyXYTuple(m);
}
