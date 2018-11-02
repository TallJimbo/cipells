use super::detail::GuaranteedBounded;
use super::{AbstractBounds, Bounded, Bounds, Empty, Interval, Scalar};

impl<T: Scalar> GuaranteedBounded for Interval<T> {
    type Point = T;
    fn _new(lower: Self::Point, upper: Self::Point) -> Bounds<Self> {
        if lower <= upper {
            Bounded(Interval { lower, upper })
        } else {
            Empty
        }
    }
    fn _expand_to<U: AbstractBounds<Self>>(&mut self, other: U) {
        if let Bounded(other) = other.to_bounds() {
            self.lower = self.lower.min(other.lower);
            self.upper = self.upper.max(other.upper);
        }
    }
    fn _clipped_to<U: super::AbstractBounds<Self>>(&self, other: U) -> super::Bounds<Self> {
        if let Bounded(other) = other.to_bounds() {
            Interval::_new(self.lower.max(other.lower), self.upper.min(other.upper))
        } else {
            Empty
        }
    }
    fn _shift_by(&mut self, point: Self::Point) {
        self.lower += point;
        self.upper += point;
    }
}

impl<T: Scalar> Interval<T> {
    pub fn expand_to<U: AbstractBounds<Self>>(&mut self, other: U) {
        self._expand_to(other);
    }
    pub fn shift_by(&mut self, other: T) {
        self._shift_by(other);
    }
    pub fn lower(&self) -> T {
        self.lower
    }
    pub fn upper(&self) -> T {
        self.upper
    }
    pub fn size(&self) -> T {
        Scalar::size(self.lower, self.upper)
    }
}

impl<'a, T: Scalar> AbstractBounds<Interval<T>> for &'a Interval<T> {
    fn to_bounds(self) -> Bounds<Interval<T>> {
        Bounded(self.clone())
    }

    fn is_empty(self) -> bool {
        false
    }

    fn contains<U>(self, other: U) -> bool
    where
        U: AbstractBounds<Interval<T>>,
    {
        if let Bounded(other) = other.to_bounds() {
            self.lower <= other.lower && self.upper >= other.upper
        } else {
            true
        }
    }

    fn intersects<U>(self, other: U) -> bool
    where
        U: AbstractBounds<Interval<T>>,
    {
        if let Bounded(other) = other.to_bounds() {
            !(self.upper < other.lower || self.lower > other.upper)
        } else {
            false
        }
    }

    fn clipped_to<U>(self, other: U) -> Bounds<Interval<T>>
    where
        U: AbstractBounds<Interval<T>>,
    {
        self._clipped_to(other)
    }

    fn expanded_to<U>(self, other: U) -> Bounds<Interval<T>>
    where
        U: AbstractBounds<Interval<T>>,
    {
        let mut result = self.clone();
        result._expand_to(other);
        Bounded(result)
    }
}

impl<T: Scalar> From<T> for Bounds<Interval<T>> {
    fn from(other: T) -> Bounds<Interval<T>> {
        Interval::_new(other, other)
    }
}

impl<T: Scalar> From<Interval<T>> for Bounds<Interval<T>> {
    fn from(other: Interval<T>) -> Bounds<Interval<T>> {
        Bounded(other)
    }
}

impl<T: Scalar> From<std::ops::RangeInclusive<T>> for Bounds<Interval<T>> {
    fn from(other: std::ops::RangeInclusive<T>) -> Bounds<Interval<T>> {
        Interval::_new(*other.start(), *other.end())
    }
}
