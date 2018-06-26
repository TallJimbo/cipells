#ifndef CIPELLS_MultiImage_h_INCLUDED
#define CIPELLS_MultiImage_h_INCLUDED

#include "cipells/Box.h"
#include "cipells/fwd/Image.h"

namespace cipells {


class MultiImage {
public:

    enum class OnConflict { ERROR, IGNORE, REPLACE };

    class Element {
    public:

        Element(Element const &) = delete;
        Element & operator=(Element const &) = delete;

        template <typename T>
        operator T const & () const;

        explicit operator bool() const { return _impl != nullptr; }

        virtual ~Element();

    private:

        explicit Element(std::unique_ptr<Impl> impl);

        friend class MultiImage;

        class Impl;
        std::unique_ptr<Impl> _impl;
    };

    using Map = std::unordered_map<std::string, Element>;

    using const_iterator = Map::const_iterator;

    explicit MultiImage(IndexBox const & bbox);

    const_iterator begin() const { return _map.begin(); }
    const_iterator end() const { return _map.end(); }
    std::size_t size() const { return _map.size(); }

    Element const & operator[](std::string const &) const;

    MultiImage operator[](std::vector<std::string> const & names) const;

    MultiImage operator[](IndexBox const & box) const;

    template <typename T>
    void insert(std::string name, Image<T> plane, Role role, OnConflict on_conflict=OnConflict::ERROR);

    IndexBox const & bbox() const { return _bbox; }

    MultiImage operator+(MultiImage const & rhs) const;
    MultiImage operator-(MultiImage const & rhs) const;

    MultiImage operator*(MultiImage const & rhs) const;
    MultiImage operator/(MultiImage const & rhs) const;

private:
    Map _map;
    IndexBox _bbox;
};

} // namespace cipells

#endif // !CIPELLS_MultiImage_h_INCLUDED
