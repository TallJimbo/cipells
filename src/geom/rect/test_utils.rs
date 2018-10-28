use std::slice::Iter;
use std::iter::Chain;

use super::{Rect, Scalar, AbstractInterval, Interval};
use super::super::TestIntervals;

#[derive(Default)]
pub struct TestRects<T: Scalar> {
    finite: Vec<Rect<T>>,
    x_empty: Vec<Rect<T>>,
    y_empty: Vec<Rect<T>>,
    xy_empty: Vec<Rect<T>>,
}


impl<T> TestRects<T>
    where T: Scalar + Default + AbstractInterval<T>
{

    fn interval_iters_to_rects<'a, 'b, U, V>(x: U, y: V) -> Vec<Rect<T>>
        where U: Iterator<Item=&'a Interval<T>> + Clone,
              V: Iterator<Item=&'b Interval<T>> + Clone,
    {
        iproduct!(x, y).map(|xy| Rect { x: xy.0.clone(), y: xy.1.clone() }).collect()
    }

    pub fn new(points: &Vec<T>) -> Self {
        let intervals = TestIntervals::new(points);
        Self {
            finite: Self::interval_iters_to_rects(intervals.finite(), intervals.finite()),
            x_empty: Self::interval_iters_to_rects(intervals.empty(), intervals.finite()),
            y_empty: Self::interval_iters_to_rects(intervals.finite(), intervals.empty()),
            xy_empty: Self::interval_iters_to_rects(intervals.empty(), intervals.empty()),
        }
    }

    pub fn finite(&self) -> Iter<Rect<T>> {
        self.finite.iter()
    }

    pub fn x_empty(&self) -> Iter<Rect<T>> {
        self.x_empty.iter()
    }

    pub fn y_empty(&self) -> Iter<Rect<T>> {
        self.y_empty.iter()
    }

    pub fn xy_empty(&self) -> Iter<Rect<T>> {
        self.xy_empty.iter()
    }

    pub fn empty(&self) -> Chain<Iter<Rect<T>>, Chain<Iter<Rect<T>>, Iter<Rect<T>>>> {
        self.xy_empty().chain(self.x_empty().chain(self.y_empty()))
    }

    pub fn all(&self) -> Chain<Iter<Rect<T>>, Chain<Iter<Rect<T>>, Chain<Iter<Rect<T>>, Iter<Rect<T>>>>> {
        self.finite().chain(self.empty())
    }

}
