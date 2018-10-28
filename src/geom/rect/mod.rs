use try_from::TryFrom;
use std::fmt;

use super::{
    Scalar, Real, Index, Point2, Vector2, Interval, AbstractInterval,
    RealToIndexError,
};

mod abstract_;
pub use self::abstract_::AbstractRect;

#[cfg(test)]
pub mod test_utils;

#[cfg(test)]
mod tests;


#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rect<T: Scalar> {
    pub x: Interval<T>,
    pub y: Interval<T>,
}

pub type RealRect = Rect<Real>;
pub type IndexRect = Rect<Index>;

impl<T: Scalar + AbstractInterval<T>> Rect<T> {

    pub fn new<R>(other: R) -> Self
        where R: AbstractRect<T>,
              R::Element: AbstractInterval<T>,
    {
        let (x, y) = other.to_xy_tuple();
        Self {
            x: Interval::new(x),
            y: Interval::new(y),
        }
    }

    pub fn with_intervals<U: AbstractInterval<T>, V: AbstractInterval<T>>(x: U, y: V) -> Self {
        Rect { x: Interval::new(x), y: Interval::new(y) }
    }

    pub fn hull<I>(iter: I) -> Self
        where I: IntoIterator,
              I::Item: AbstractRect<T>,
              <I::Item as AbstractRect<T>>::Element: AbstractInterval<T>,
    {
        let mut result: Self = Default::default();
        for x in iter {
            result = result.expanded_to(x);
        }
        result
    }

    pub fn is_empty(&self) -> bool { self.x.is_empty() || self.y.is_empty() }

    pub fn min(&self) -> Option<Point2<T>> {
        if self.x.is_empty() || self.y.is_empty() {
            None
        } else {
            Some(Point2::new(self.x.min().unwrap(), self.y.min().unwrap()))
        }
    }

    pub fn max(&self) -> Option<Point2<T>> {
        if self.x.is_empty() || self.y.is_empty() {
            None
        } else {
            Some(Point2::new(self.x.max().unwrap(), self.y.max().unwrap()))
        }
    }

    pub fn size(&self) -> Vector2<T> {
        Vector2::new(self.x.size(), self.y.size())
    }

    pub fn width(&self) -> T { self.x.size() }
    pub fn height(&self) -> T { self.y.size() }
    pub fn area(&self) -> T { self.width()*self.height() }

    pub fn corners(&self) -> Option<[Point2<T>; 4]> {
        if self.is_empty() {
            None
        } else {
            Some([Point2::new(self.x.min().unwrap(), self.y.min().unwrap()),
                  Point2::new(self.x.min().unwrap(), self.y.max().unwrap()),
                  Point2::new(self.x.max().unwrap(), self.y.max().unwrap()),
                  Point2::new(self.x.max().unwrap(), self.y.min().unwrap())])
        }
    }

    pub fn contains<R>(&self, other: R) -> bool
        where R: AbstractRect<T>,
              R::Element: AbstractInterval<T>,
    {
        let (x, y) = other.to_xy_tuple();
        self.x.contains(x) && self.y.contains(y)
    }

    pub fn intersects<R>(&self, other: R) -> bool
        where R: AbstractRect<T>,
              R::Element: AbstractInterval<T>,
    {
        let (x, y) = other.to_xy_tuple();
        self.x.intersects(x) && self.y.intersects(y)
    }

    pub fn shifted_by(&self, v: &Vector2<T>) -> Self {
        Self {
            x: self.x.shifted_by(v.x),
            y: self.y.shifted_by(v.y),
        }
    }

    pub fn clipped_to<R>(&self, other: R) -> Self
        where R: AbstractRect<T>,
              R::Element: AbstractInterval<T>,
    {
        let (x, y) = other.to_xy_tuple();
        Self {
            x: self.x.clipped_to(x),
            y: self.y.clipped_to(y),
        }
    }

    pub fn expanded_to<R>(&self, other: R) -> Self
        where R: AbstractRect<T>,
              R::Element: AbstractInterval<T>,
    {
        let (x, y) = other.to_xy_tuple();
        Self {
            x: self.x.expanded_to(x),
            y: self.y.expanded_to(y),
        }
    }

    pub fn dilated_by(&self, v: &Vector2<T>) -> Self {
        Self {
            x: self.x.dilated_by(v.x),
            y: self.y.dilated_by(v.y),
        }
    }

    pub fn eroded_by(self, v: &Vector2<T>) -> Self {
        self.dilated_by(&-v)
    }

}

impl Rect<Real> {

    pub fn with_center_and_size(center: &Point2<Real>, size: &Vector2<Real>) -> Self {
        let min = center - 0.5*size;
        Self::new(min..=min + size)
    }

    pub fn center(&self) -> Point2<Real> {
        Point2::new(self.x.center(), self.y.center())
    }

}


impl<T: Scalar> Eq for Rect<T> {}

impl<T: Scalar> Default for Rect<T> {
    fn default() -> Self {
        Self { x: Default::default(), y: Default::default() }
    }
}

impl<'a> From<&'a Rect<Index>> for Rect<Real> {
    fn from(value: &'a Rect<Index>) -> Self {
        Self {
            x: Interval::from(&value.x),
            y: Interval::from(&value.y),
        }
    }
}

impl<'a> TryFrom<&'a Rect<Real>> for Rect<Index> {
    type Err = RealToIndexError;
    fn try_from(value: &'a Rect<Real>) -> Result<Self, Self::Err> {
        let x = Interval::try_from(&value.x)?;
        let y = Interval::try_from(&value.y)?;
        Ok(Self { x, y })
    }
}

impl<T> fmt::Display for Rect<T>
    where T: Scalar,
          Interval<T>: fmt::Display
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Display::fmt(&self.x, f)?;
        write!(f, "×")?;
        fmt::Display::fmt(&self.y, f)?;
        Ok(())
    }
}
