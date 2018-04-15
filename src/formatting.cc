#define CIPELLS_formatting_cc_SRC
#include <cstring>
#include "impl/formatting.h"

namespace cipells { namespace detail {

char const * const ScalarFormatTraits<Index>::NAME = "Index";
char const * const ScalarFormatTraits<Index>::TMPL = "{:d}";

char const * const ScalarFormatTraits<Real>::NAME = "Real";
char const * const ScalarFormatTraits<Real>::TMPL = "{:0.15g}";

bool compareFormatSpec(FormatSpec const & spec, char const * cs) {
    char const * c1 = spec.first;
    char const * c2 = cs;
    while (true) {
        if (c1 == spec.second) {
            return *c2 == '\0'; // return true if we reach the ends together
        }
        if (*c2 == '\0') return false;
        if (*c1 != *c2) return false;
        ++c1;
        ++c2;
    }
}

FormatSpec extractFormatSpec(char const *& tmpl) {
    if (*tmpl == ':') {
        ++tmpl;
    }
    FormatSpec result(tmpl, std::strchr(tmpl, '}'));
    if (result.second == nullptr) {
        throw fmt::FormatError("missing '}' in format string");
    }
    tmpl = result.second + 1;
    return result;
}

template class ScalarFormatProxy<Index>;
template class ScalarFormatProxy<Real>;

}} // namespace cipells::detail
