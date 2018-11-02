use super::detail::GuaranteedBounded;
use super::{AbstractBounds, Bounded, Bounds, Empty};

impl<R> Bounds<R>
where
    R: GuaranteedBounded,
{
    pub fn new(lower: R::Point, upper: R::Point) -> Self {
        GuaranteedBounded::_new(lower, upper)
    }

    pub fn expand_to<U>(&mut self, other: U)
    where
        U: AbstractBounds<R>,
    {
        if let Bounded(ref mut b) = self {
            b._expand_to(other);
        } else {
            *self = other.to_bounds();
        }
    }

    pub fn clip_to<U>(&mut self, other: U)
    where
        U: AbstractBounds<R>,
    {
        // extract current value of self and set it to Empty (for now)
        if let Bounded(b) = std::mem::replace(self, Empty) {
            // if self was not Empty, do the clip and assign to it (which
            // may or may not assign Empty)
            *self = b._clipped_to(other);
        }
        // if self was Empty, it should stay Empty
    }

    pub fn shift_by(&mut self, offset: R::Vector) {
        if let Bounded(ref mut b) = self {
            b._shift_by(offset);
        }
    }

    pub fn hull<'a, I>(points: I) -> Self
    where
        I: IntoIterator,
        I::Item: AbstractBounds<R>,
        &'a Bounds<R>: AbstractBounds<R>,
    {
        let mut result: Bounds<R> = Empty;
        for point in points {
            result.expand_to(point)
        }
        result
    }
}

impl<'a, R> AbstractBounds<R> for &'a Bounds<R>
where
    R: GuaranteedBounded,
    R: PartialEq + Clone + 'static,
    &'a R: AbstractBounds<R>,
{
    fn to_bounds(self) -> Bounds<R> {
        (*self).clone()
    }

    fn is_empty(self) -> bool {
        self == &Empty
    }

    fn contains<U>(self, other: U) -> bool
    where
        U: AbstractBounds<R>,
    {
        if let Bounded(b) = self {
            b.contains(other)
        } else {
            other.is_empty()
        }
    }

    fn intersects<U>(self, other: U) -> bool
    where
        U: AbstractBounds<R>,
    {
        if let Bounded(b) = self {
            b.intersects(other)
        } else {
            false
        }
    }

    fn clipped_to<U>(self, other: U) -> Bounds<R>
    where
        U: AbstractBounds<R>,
    {
        if let Bounded(b) = self {
            b.clipped_to(other)
        } else {
            Empty
        }
    }

    fn expanded_to<U>(self, other: U) -> Bounds<R>
    where
        U: AbstractBounds<R>,
    {
        if let Bounded(b) = self {
            b.expanded_to(other)
        } else {
            other.to_bounds()
        }
    }

    fn shifted_by(self, offset: R::Vector) -> Bounds<R> {
        if let Bounded(b) = self {
            b.shifted_by(offset)
        } else {
            Empty
        }
    }
}
