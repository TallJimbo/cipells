use super::detail::GuaranteedBounded;
use super::{AbstractBounds, Bounded, Bounds, Empty, Interval, Rect, Scalar};
use nalgebra::{Point2, Vector2};

impl<T: Scalar> GuaranteedBounded for Rect<T> {
    // We would prefer references, but that's basically impossible with Rust today
    type Point = Point2<T>;
    type Vector = Vector2<T>;
    fn _new(lower: Self::Point, upper: Self::Point) -> Bounds<Self> {
        let x = Interval::_new(lower.x, upper.x);
        let y = Interval::_new(lower.y, upper.y);
        if let (Bounded(x), Bounded(y)) = (x, y) {
            Bounded(Rect { x, y })
        } else {
            Empty
        }
    }
    fn _expand_to<U: AbstractBounds<Self>>(&mut self, other: U) {
        if let Bounded(other) = other.to_bounds() {
            self.x.expand_to(&other.x);
            self.y.expand_to(&other.y);
        }
    }
    fn _clipped_to<U: AbstractBounds<Self>>(&self, other: U) -> Bounds<Rect<T>> {
        if let Bounded(other) = other.to_bounds() {
            Bounds::from_xy(self.x.clipped_to(&other.x), self.y.clipped_to(&other.y))
        } else {
            Empty
        }
    }
    fn _shift_by(&mut self, offset: Self::Vector) {
        self.x._shift_by(offset.x);
        self.y._shift_by(offset.y);
    }
}

impl<T: Scalar> Rect<T> {
    pub fn expand_to<U: AbstractBounds<Self>>(&mut self, other: U) {
        self._expand_to(other);
    }
    pub fn shift_by(&mut self, offset: Vector2<T>) {
        self._shift_by(offset);
    }
    pub fn lower(&self) -> Point2<T> {
        Point2::new(self.x.lower(), self.y.lower())
    }
    pub fn upper(&self) -> Point2<T> {
        Point2::new(self.x.upper(), self.y.upper())
    }
    pub fn size(&self) -> Vector2<T> {
        Vector2::new(self.x.size(), self.y.size())
    }
    pub fn area(&self) -> T {
        self.x.size() * self.y.size()
    }
    pub fn width(&self) -> T {
        self.x.size()
    }
    pub fn height(&self) -> T {
        self.y.size()
    }
}

impl<T: Scalar> Bounds<Rect<T>> {
    pub fn from_xy(x: Bounds<Interval<T>>, y: Bounds<Interval<T>>) -> Self {
        if let (Bounded(x), Bounded(y)) = (x, y) {
            Bounded(Rect { x, y })
        } else {
            Empty
        }
    }
}

impl<'a, T: Scalar> AbstractBounds<Rect<T>> for &'a Rect<T> {
    fn to_bounds(self) -> Bounds<Rect<T>> {
        Bounded(self.clone())
    }

    fn is_empty(self) -> bool {
        false
    }

    fn contains<U>(self, other: U) -> bool
    where
        U: AbstractBounds<Rect<T>>,
    {
        if let Bounded(other) = other.to_bounds() {
            self.x.contains(&other.x) && self.y.contains(&other.y)
        } else {
            true
        }
    }

    fn intersects<U>(self, other: U) -> bool
    where
        U: AbstractBounds<Rect<T>>,
    {
        if let Bounded(other) = other.to_bounds() {
            self.x.intersects(&other.x) && self.y.intersects(&other.y)
        } else {
            false
        }
    }

    fn clipped_to<U>(self, other: U) -> Bounds<Rect<T>>
    where
        U: AbstractBounds<Rect<T>>,
    {
        self._clipped_to(other)
    }

    fn expanded_to<U>(self, other: U) -> Bounds<Rect<T>>
    where
        U: AbstractBounds<Rect<T>>,
    {
        let mut result = self.clone();
        result._expand_to(other);
        Bounded(result)
    }

    fn shifted_by(self, offset: <Rect<T> as GuaranteedBounded>::Vector) -> Bounds<Rect<T>> {
        let mut result = self.clone();
        result._shift_by(offset);
        Bounded(result)
    }
}
