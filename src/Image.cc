#define CIPELLS_Image_cc_SRC
#include "cipells/Image.h"

namespace cipells {

namespace {

template <typename T>
T div_ceil(T x, T y) {
    return x/y + (x % y != 0);
}

ImageOwner allocate(std::size_t item_size, Index area) {
    return ImageOwner(new ImageStorage[div_ceil(area*item_size, sizeof(ImageStorage))],
                      std::default_delete<ImageStorage[]>());
}

} // anonymous

template <typename T>
Image<T const>::Image() : _data(nullptr), _stride(0), _owner(nullptr), _bbox() {}

template <typename T>
Image<T const>::Image(T const * data, IndexBox const & bbox, ImageOwner owner, Index stride) :
    _data(data), _stride(stride ? stride : bbox.width()), _owner(std::move(owner)), _bbox(bbox)
{}

template <typename T>
Image<T const>::Image(IndexBox const & box) :
    _data(nullptr), _stride(box.width()),
    _owner(allocate(sizeof(T), box.area())),
    _bbox(box)
{
    _data = reinterpret_cast<T const*>(_owner.get());
}

template <typename T>
Image<T const>::Image(Image const &) = default;

template <typename T>
Image<T const>::Image(Image && other) :
    _data(other._data), _stride(other._stride), _owner(std::move(other._owner)), _bbox(other._bbox)
{
    other._data = nullptr;
    other._stride = 0;
    other._bbox = IndexBox();
}

template <typename T>
Image<T const> & Image<T const>::operator=(Image const & other) = default;

template <typename T>
Image<T const> & Image<T const>::operator=(Image && other) {
    if (&other != this) {
        _data = other._data;
        _stride = other._stride;
        _owner = std::move(other._owner);
        _bbox = other._bbox;
        other._data = nullptr;
        other._stride = 0;
        other._bbox = IndexBox();
    }
    return *this;
}

template <typename T>
Image<T> Image<T const>::copy() const {
    Image<T> result(this->bbox());
    result.array() = this->array();
    return result;
}

template <typename T>
Image<T const> Image<T const>::freeze() && {
    Image<T const> result(std::move(*this));
    if (result.owner().use_count() > 1) {
        // n.b. use_count is not reliable when multiple threads have access to
        // this owner, but that shoudn't matter for our use cases, and the
        // worst-case scenario for us is an unnecessary copy.
        result = result.copy();
    }
    return result;
}

template <typename T>
Image<T>::Image(IndexBox const & box) : Base(box) {
    array().setZero();
}

template class Image<float const>;
template class Image<float>;
template class Image<std::complex<float> const>;
template class Image<std::complex<float>>;

} // namespace cipells
