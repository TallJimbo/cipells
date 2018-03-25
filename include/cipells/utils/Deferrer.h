#ifndef CIPELLS_UTILS_Deferrer_h_INCLUDED
#define CIPELLS_UTILS_Deferrer_h_INCLUDED

#include <list>
#include <functional>

namespace cipells { namespace utils {

class Deferrer {
public:

    using Function = std::function<void()>;

    Deferrer() : _funcs() {}

    Deferrer(Deferrer const &) = delete;
    Deferrer(Deferrer &&) = default;

    Deferrer & operator=(Deferrer const &) = delete;
    Deferrer & operator=(Deferrer &&) = delete;

    template <typename Arg, typename Func>
    Arg add(Arg arg, Func func) {
        _funcs.emplace_back([arg, func]() mutable { func(arg); });
        return arg;
    }

    void add(Function func) {
        _funcs.push_back(std::move(func));
    }

    ~Deferrer() {
        for (auto const & func : _funcs) {
            func();
        }
    }

private:
    std::list<Function> _funcs;
};

}} // namespace cipells::utils

#endif // !CIPELLS_UTILS_Deferrer_h_INCLUDED
