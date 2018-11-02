use super::detail::BoundedValue;
use super::{AbstractBounds, Bounded, Bounds, Empty, Interval, Rect, Scalar};
use nalgebra::{Point2, Vector2};

impl<T: Scalar> BoundedValue for Rect<T> {
    type Point = Point2<T>;
    fn new(lower: Self::Point, upper: Self::Point) -> Bounds<Self> {
        let x = Interval::new(lower.x, upper.x);
        let y = Interval::new(lower.y, upper.y);
        if let (Bounded(x), Bounded(y)) = (x, y) {
            Bounded(Rect { x, y })
        } else {
            Empty
        }
    }
}

impl<T: Scalar> Rect<T> {
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

impl<T: Scalar> AbstractBounds<Rect<T>> for Rect<T> {
    fn is_empty(&self) -> bool {
        false
    }

    fn contains<U>(&self, other: U) -> bool
    where
        Bounds<Rect<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            self.x.contains(other.x) && self.y.contains(other.y)
        } else {
            true
        }
    }

    fn intersects<U>(&self, other: U) -> bool
    where
        Bounds<Rect<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            self.x.intersects(other.x) && self.y.intersects(other.y)
        } else {
            false
        }
    }

    fn clipped_to<U>(&self, other: U) -> Bounds<Rect<T>>
    where
        Bounds<Rect<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            Bounds::from_xy(self.x.clipped_to(other.x), self.y.clipped_to(other.y))
        } else {
            Empty
        }
    }

    fn expanded_to<U>(&self, other: U) -> Bounds<Rect<T>>
    where
        Bounds<Rect<T>>: From<U>,
    {
        if let Bounded(other) = Bounds::from(other) {
            Bounds::from_xy(self.x.expanded_to(other.x), self.y.expanded_to(other.y))
        } else {
            Bounded(self.clone())
        }
    }
}
