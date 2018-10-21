use std::{f64, i32};
use std::error::Error;
use std::fmt;
use std::cmp::Ordering;
use std::ops::Range;

use num::{ToPrimitive};
use try_from::TryFrom;

mod element;
pub use self::element::IntervalElement;

mod abstract_;
pub use self::abstract_::AbstractInterval;

#[cfg(test)]
pub mod test_utils;

#[cfg(test)]
mod tests;


#[derive(Debug, Clone, Copy)]
pub struct Interval<T: IntervalElement> {
    min: T,
    max: T,
}


pub type RealInterval = Interval<f64>;
pub type IndexInterval = Interval<i32>;


impl<T: IntervalElement> Interval<T> {

    pub fn new<I: AbstractInterval<T>>(other: I) -> Self {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_interval(min, max) {
            Default::default()
        } else {
            Self { min, max }
        }
    }

    pub fn hull<I>(iter: I) -> Self
        where I: IntoIterator,
              I::Item: AbstractInterval<T>
    {
        let mut result: Self = Default::default();
        for x in iter {
            result = result.expanded_to(x);
        }
        result
    }

    pub fn is_empty(&self) -> bool { is_empty_interval(self.min, self.max) }

    pub fn min(&self) -> Option<T> {
        if self.is_empty() {
            None
        } else {
            Some(self.min)
        }
    }

    pub fn max(&self) -> Option<T> {
        if self.is_empty() {
            None
        } else {
            Some(self.max)
        }
    }

    pub fn size(&self) -> T {
        if self.is_empty() {
            T::zero()
        } else {
            self.max - self.min
        }
    }

    pub fn contains<I: AbstractInterval<T>>(&self, other: I) -> bool {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_interval(min, max) {
            true
        } else if self.is_empty() {
            false
        } else {
            self.min <= min && self.max >= max
        }
    }

    pub fn intersects<I: AbstractInterval<T>>(&self, other: I) -> bool {
        let (min, max) = other.to_min_max_tuple();
        if self.is_empty() || is_empty_interval(min, max) {
            false
        } else {
            !(self.max < min || self.min > max)
        }
    }

    pub fn shifted_by(&self, x: T) -> Self {
        Self { min: self.min + x, max: self.max + x }
    }

    pub fn clipped_to<I: AbstractInterval<T>>(&self, other: I) -> Self {
        if self.is_empty() {
            Default::default()
        } else {
            let (min, max) = other.to_min_max_tuple();
            if is_empty_interval(min, max) {
                Default::default()
            } else {
                Self::new(self.min.max(min)..=self.max.min(max))
            }
        }
    }

    pub fn expanded_to<I: AbstractInterval<T>>(&self, other: I) -> Self {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_interval(min, max) {
            self.clone()
        } else if self.is_empty() {
            Self::new(min..=max)
        } else {
            Self { min: self.min.min(min), max: self.max.max(max) }
        }
    }

    pub fn dilated_by(&self, x: T) -> Self {
        if self.is_empty() {
            self.clone()
        } else {
            Self::new((self.min - x)..=(self.max + x))
        }
    }

    pub fn eroded_by(self, x: T) -> Self {
        self.dilated_by(-x)
    }

}

impl<T: IntervalElement> PartialEq for Interval<T> {
    fn eq(&self, other: &Self) -> bool {
        if self.is_empty() && other.is_empty() {
            true
        } else {
            self.min == other.min && self.max == other.max
        }
    }
}

impl<T: IntervalElement> Eq for Interval<T> {}

impl<T: IntervalElement> PartialOrd for Interval<T> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        if self.max < other.min {
            Some(Ordering::Less)
        } else if self.min > other.max {
            Some(Ordering::Greater)
        } else if self.min == other.min && self.max == other.max {
            Some(Ordering::Equal)
        } else {
            None
        }
    }
}

impl<T> Default for Interval<T>
    where T: IntervalElement
{
    fn default() -> Self {
        Self { min: T::empty_min(), max: T::empty_max() }
    }
}


impl Interval<f64> {

    pub fn with_center_and_size(x: f64, size: f64) -> Self {
        let min = x - 0.5*size;
        Self::new(min..=min + size)
    }

    pub fn center(&self) -> f64 { 0.5*(self.min + self.max) }

}

impl Interval<i32> {

    pub fn to_range_with_origin(&self, x0: i32) -> Range<usize> {
        if self.is_empty() {
            0..0
        } else {
            let start = self.min + x0;
            let end = self.max + x0 + 1;
            (start as usize)..(end as usize)
        }
    }

}

impl<'a> From<&'a Interval<i32>> for Interval<f64> {
    fn from(value: &'a Interval<i32>) -> Self {
        if value.is_empty() {
            Default::default()
        } else {
            let min = value.min.to_f64().unwrap() - 0.5;
            let max = value.max.to_f64().unwrap() + 0.5;
            Self::new(min..=max)
        }
    }
}


#[derive(Debug, Clone, PartialEq)]
pub struct RealToIndexError {
    from: RealInterval,
}

impl fmt::Display for RealToIndexError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "RealInterval {} cannot be converted to an IndexInterval.", self.from)
    }
}

impl Error for RealToIndexError {}


impl<'a> TryFrom<&'a Interval<f64>> for Interval<i32> {
    type Err = RealToIndexError;
    fn try_from(value: &'a Interval<f64>) -> Result<Self, Self::Err> {
        if let (Some(min), Some(max)) = (value.min(), value.max()) {
            if min < (i32::MIN as f64) || max > (i32::MAX as f64) {
                Err(RealToIndexError { from: value.clone() })
            } else {
                let min = round_up(min).to_i32().unwrap();
                let max = round_down(max).to_i32().unwrap();
                Ok(Interval::new(min..=max))
            }
        } else {
            Ok(Default::default())
        }
    }
}

impl fmt::Display for Interval<f64> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.min.is_finite() {
            write!(f, "[")?;
        } else {
            write!(f, "(")?;
        }
        if !self.is_empty() {
            fmt::Display::fmt(&self.min, f)?;
            write!(f, ", ")?;
            fmt::Display::fmt(&self.max, f)?;
        }
        if self.max.is_finite() {
            write!(f, "]")?;
        } else {
            write!(f, ")")?;
        }
        Ok(())
    }
}

impl fmt::Display for Interval<i32> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if !self.is_empty() {
            write!(f, "[")?;
            fmt::Display::fmt(&self.min, f)?;
            write!(f, ", ")?;
            fmt::Display::fmt(&self.max, f)?;
            write!(f, "]")?;
        } else {
            write!(f, "()")?;
        }
        Ok(())
    }
}

fn is_empty_interval<T: PartialOrd>(min: T, max: T) -> bool { !(min <= max) }

fn round_up(v: f64) -> f64 {
    if v.fract() < 0.5 {
        v.trunc()
    } else {
        v.trunc() + 1.0
    }
}

fn round_down(v: f64) -> f64{
    if v.fract() <= 0.5 {
        v.trunc()
    } else {
        v.trunc() + 1.0
    }
}
