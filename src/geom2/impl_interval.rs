use super::detail::BoundedValue;
use super::{AbstractBounds, Bounded, Bounds, Empty, Interval, Scalar};

impl<T: Scalar> BoundedValue for Interval<T> {
    type Point = T;
    fn new(lower: Self::Point, upper: Self::Point) -> Bounds<Self> {
        if lower <= upper {
            Bounded(Interval { lower, upper })
        } else {
            Empty
        }
    }
}

impl<T: Scalar> Interval<T> {
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

impl<T: Scalar> AbstractBounds<Interval<T>> for Interval<T> {
    fn is_empty(&self) -> bool {
        false
    }

    fn contains<U>(&self, other: U) -> bool
    where
        Bounds<Interval<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            self.lower <= other.lower && self.upper >= other.upper
        } else {
            true
        }
    }

    fn intersects<U>(&self, other: U) -> bool
    where
        Bounds<Interval<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            !(self.upper < other.lower || self.lower > other.upper)
        } else {
            false
        }
    }

    fn clipped_to<U>(&self, other: U) -> Bounds<Interval<T>>
    where
        Bounds<Interval<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            Interval::new(self.lower.max(other.lower), self.upper.min(other.upper))
        } else {
            Empty
        }
    }

    fn expanded_to<U>(&self, other: U) -> Bounds<Interval<T>>
    where
        Bounds<Interval<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            Interval::new(self.lower.min(other.lower), self.upper.max(other.upper))
        } else {
            Bounded(self.clone())
        }
    }
}

impl<T: Scalar> From<T> for Bounds<Interval<T>> {
    fn from(other: T) -> Bounds<Interval<T>> {
        Interval::new(other, other)
    }
}

impl<T: Scalar> From<Interval<T>> for Bounds<Interval<T>> {
    fn from(other: Interval<T>) -> Bounds<Interval<T>> {
        Bounded(other)
    }
}

impl<T: Scalar> From<std::ops::RangeInclusive<T>> for Bounds<Interval<T>> {
    fn from(other: std::ops::RangeInclusive<T>) -> Bounds<Interval<T>> {
        Interval::new(*other.start(), *other.end())
    }
}
