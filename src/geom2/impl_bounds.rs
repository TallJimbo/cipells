use super::detail::BoundedValue;
use super::{AbstractBounds, Bounded, Bounds, Empty};

impl<R: BoundedValue> Bounds<R> {
    pub fn new(lower: R::Point, upper: R::Point) -> Self {
        BoundedValue::new(lower, upper)
    }

    pub fn hull<I>(points: I) -> Self
    where
        I: IntoIterator,
        Bounds<R>: From<I::Item>,
        Bounds<R>: AbstractBounds<R>,
    {
        let mut result = Empty;
        for point in points {
            result = result.expanded_to(point)
        }
        result
    }
}

impl<R> AbstractBounds<R> for Bounds<R>
where
    R: BoundedValue + AbstractBounds<R> + PartialEq,
{
    fn is_empty(&self) -> bool {
        self == &Empty
    }

    fn contains<U>(&self, other: U) -> bool
    where
        Bounds<R>: From<U>,
    {
        if let Bounded(ref b) = self {
            b.contains(other)
        } else {
            Bounds::from(other).is_empty()
        }
    }

    fn intersects<U>(&self, other: U) -> bool
    where
        Bounds<R>: From<U>,
    {
        if let Bounded(ref b) = self {
            b.intersects(other)
        } else {
            false
        }
    }

    fn clipped_to<U>(&self, other: U) -> Bounds<R>
    where
        Bounds<R>: From<U>,
    {
        if let Bounded(ref b) = self {
            b.clipped_to(other)
        } else {
            Empty
        }
    }

    fn expanded_to<U>(&self, other: U) -> Bounds<R>
    where
        Bounds<R>: From<U>,
    {
        if let Bounded(ref b) = self {
            b.expanded_to(other)
        } else {
            Bounds::from(other)
        }
    }
}
