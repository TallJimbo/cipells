#define CIPELLS_formatting_cc_SRC
#include "impl/formatting.h"

namespace cipells { namespace detail {

char const * const ScalarFormatTraits<Index>::NAME = "Index";
char const * const ScalarFormatTraits<Index>::TMPL = "{:d}";

char const * const ScalarFormatTraits<Real>::NAME = "Real";
char const * const ScalarFormatTraits<Real>::TMPL = "{:0.15g}";

bool isTemplateRepr(char const *& tmpl) {
    if (*tmpl == ':') {
        ++tmpl;
    }
    char const * end = tmpl;
    while (*end && *end != '}') {
        ++end;
    }
    if (*end != '}') {
        throw fmt::FormatError("missing '}' in format string");
    }
    bool result = false;
    if (tmpl + 1 == end) {
        if (*tmpl == 'r') {
            result = true;
        } else if (*tmpl != 's') {
            throw fmt::FormatError("bad format string: must be '', 's', or 'r'");
        }
    } else if (tmpl != end) {
        throw fmt::FormatError(
            fmt::format("format string '{}' too long: must be '', 's', or 'r'", std::string(tmpl, end)));
    }
    tmpl = end + 1;
    return result;
}

template class ScalarFormatProxy<Index>;
template class ScalarFormatProxy<Real>;

}} // namespace cipells::detail
