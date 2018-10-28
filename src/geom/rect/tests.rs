use rand::{Rng, XorShiftRng, SeedableRng};
use try_from::TryFrom;

use super::test_utils::TestRects;
use super::{Scalar, Point2, Vector2, Rect, IndexRect, RealRect, AbstractRect, Interval, AbstractInterval};


struct TestSuite<T: Scalar> {
    points: Vec<T>,
    rects: TestRects<T>,
}


impl<T> TestSuite<T>
    where T: Scalar + Default + AbstractInterval<T>,
          Point2<T>: AbstractRect<T>,
          <Point2<T> as AbstractRect<T>>::Element: AbstractInterval<T>,
{
    pub fn new(points: Vec<T>) -> Self {
        let rects = TestRects::new(&points);
        Self { points, rects }
    }

    pub fn accessors(&self) {
        for rect in self.rects.finite() {
            assert_eq!(rect.min().unwrap().x, rect.x.min().unwrap());
            assert_eq!(rect.min().unwrap().y, rect.y.min().unwrap());
            assert_eq!(rect.max().unwrap().x, rect.x.max().unwrap());
            assert_eq!(rect.max().unwrap().y, rect.y.max().unwrap());
            assert_eq!(rect.size().x, rect.width());
            assert_eq!(rect.size().y, rect.height());
            assert!(!rect.is_empty());
            assert_eq!(&Rect::hull(rect.corners().unwrap().iter().cloned()), rect);
        }
    }

    pub fn empty(&self) {
        for rect in self.rects.empty() {
            assert!(rect.is_empty());
            assert_eq!(rect.area(), T::zero());
            assert_eq!(rect.min(), None);
            assert_eq!(rect.max(), None);
            assert_eq!(rect.corners(), None);
        }
        for rect in self.rects.x_empty() {
            assert_eq!(rect.size().x, T::zero());
            assert!(rect.x.is_empty());
        }
        for rect in self.rects.y_empty() {
            assert_eq!(rect.size().y, T::zero());
            assert!(rect.y.is_empty());
        }
        for rect in self.rects.xy_empty() {
            assert_eq!(rect.size().x, T::zero());
            assert_eq!(rect.size().y, T::zero());
            assert!(rect.x.is_empty());
            assert!(rect.y.is_empty());
        }
    }

    pub fn hull(&self) {
        let mut rng = XorShiftRng::from_seed([0; 16]);
        for (n0x, p0x) in self.points.iter().enumerate() {
            for (n1x, p1x) in self.points.iter().enumerate().skip(n0x + 1) {
                for (n0y, p0y) in self.points.iter().enumerate() {
                    for (n1y, p1y) in self.points.iter().enumerate().skip(n0y + 1) {
                        let mut sx = self.points[n0x..=n1x].to_vec();
                        let mut sy = self.points[n0y..=n1y].to_vec();
                        let b = Rect { x: Interval::new(*p0x..=*p1x), y: Interval::new(*p0y..=*p1y) };
                        assert_eq!(
                            b,
                            Rect::hull(iproduct!(&sx, &sy).map(|(x, y)| Point2::new(*x, *y)))
                        );
                        rng.shuffle(&mut sx);
                        rng.shuffle(&mut sy);
                        assert_eq!(
                            b,
                            Rect::hull(iproduct!(&sx, &sy).map(|(x, y)| Point2::new(*x, *y)))
                        );
                        assert_eq!(
                            b,
                            Rect::hull(
                                iproduct!(sx.iter().rev(), sy.iter().rev())
                                    .map(|(x, y)| Point2::new(*x, *y))
                            )
                        );
                    }
                }
            }
        }
    }

    pub fn contains(&self) {
        for lhs in self.rects.finite() {
            for rhs in self.rects.finite() {
                assert_eq!(lhs.contains(rhs), lhs.x.contains(rhs.x) && lhs.y.contains(rhs.y));
                assert_eq!(lhs == rhs, lhs.contains(rhs) && rhs.contains(lhs));
            }
            for rhs in self.rects.xy_empty() {
                assert!(lhs.contains(rhs));
            }
            for (x, y) in iproduct!(&self.points, &self.points) {
                assert_eq!(lhs.contains(Point2::new(*x, *y)),
                           lhs.x.contains(*x) && lhs.y.contains(*y));
            }
        }
        for lhs in self.rects.empty() {
            for rhs in self.rects.finite() {
                assert!(!lhs.contains(rhs));
            }
            for rhs in self.rects.xy_empty() {
                assert!(lhs.contains(rhs));
            }
            for (x, y) in iproduct!(&self.points, &self.points) {
                assert!(!lhs.contains(Point2::new(*x, *y)));
            }
        }
    }

    pub fn intersects(&self) {
        for lhs in self.rects.finite() {
            for rhs in self.rects.all() {
                assert_eq!(lhs.intersects(rhs), lhs.x.intersects(rhs.x) && lhs.y.intersects(rhs.y));
            }
            for (x, y) in iproduct!(&self.points, &self.points) {
                let p = Point2::new(*x, *y);
                assert_eq!(lhs.contains(p), lhs.intersects(p));
            }
        }
        for lhs in self.rects.empty() {
            for rhs in self.rects.all() {
                assert!(!lhs.intersects(rhs));
            }
            for (x, y) in iproduct!(&self.points, &self.points) {
                assert!(!lhs.intersects(Point2::new(*x, *y)));
            }
        }
    }

    pub fn shifted_by(&self) {
        for lhs in self.rects.finite() {
            for rhs in iproduct!(&self.points, &self.points).map(|(x, y)| Vector2::new(*x, *y)) {
                let shifted = lhs.shifted_by(&rhs);
                assert_eq!(lhs.size(), shifted.size());
                assert_eq!(&lhs.min().unwrap() + &rhs, shifted.min().unwrap());
                assert_eq!(&lhs.max().unwrap() + &rhs, shifted.max().unwrap());
            }
        }
        for lhs in self.rects.empty() {
            for rhs in iproduct!(&self.points, &self.points).map(|(x, y)| Vector2::new(*x, *y)) {
                let shifted = lhs.shifted_by(&rhs);
                assert!(shifted.is_empty())
            }
        }
    }

    pub fn clipped_to(&self) {
        for lhs in self.rects.all() {
            for rhs in self.rects.all() {
                let clipped = lhs.clipped_to(rhs);
                assert!(lhs.contains(clipped));
                assert!(rhs.contains(clipped));
                assert_eq!(
                    clipped.is_empty(),
                    lhs.is_empty() || rhs.is_empty() || !lhs.intersects(rhs)
                );
                assert_eq!(&clipped == rhs, lhs.contains(rhs));
                assert_eq!(&clipped == lhs, rhs.contains(lhs));
            }
        }
    }

    pub fn expanded_to(&self) {
        for lhs in self.rects.all() {
            for rhs in self.rects.all() {
                let expanded = lhs.expanded_to(rhs);
                assert!(expanded.contains(lhs));
                assert!(expanded.contains(rhs));
                assert_eq!(
                    expanded.is_empty(),
                    (lhs.x.is_empty() && rhs.x.is_empty()) ||
                    (lhs.y.is_empty() && rhs.y.is_empty())
                );
                assert_eq!(&expanded == rhs, rhs.contains(lhs));
                assert_eq!(&expanded == lhs, lhs.contains(rhs));
            }
            for rhs in iproduct!(&self.points, &self.points).map(|(x, y)| Point2::new(*x, *y)) {
                println!("rhs={}, lhs={:?}", rhs, lhs);
                assert_eq!(lhs.expanded_to(rhs),
                           lhs.expanded_to(Rect::new(rhs..=rhs)))
            }
        }
    }

    pub fn run(&self) {
        self.accessors();
        self.empty();
        self.hull();
        self.contains();
        self.intersects();
        self.shifted_by();
        self.clipped_to();
        self.expanded_to();
    }
}

#[test]
fn real_suite() {
    let suite: TestSuite<f64> = TestSuite::new(vec![-1.5, 5.0, 6.75, 8.625]);
    suite.run();
}

#[test]
fn index_suite() {
    let suite = TestSuite::new(vec![-2, 4, 7, 11]);
    suite.run();
}

#[test]
fn real_construction() {
    let r1 = Rect::new(Point2::new(2.5, 3.25)..=Point2::new(5.0, 7.75));
    let r2 = Rect::new(Point2::new(2.5, 3.25)..Point2::new(5.0, 7.75));
    assert_eq!(&r1, &r2);
    let r3 = Rect::new(&r1);
    let r4 = Rect::new(r2);
    assert_eq!(&r3, &r4);
    assert_eq!(Rect::new(Point2::new(2.5, 3.25)..),
               Rect { x: Interval::new(2.5..), y: Interval::new(3.25..) });
    let r5 = Rect { x: Interval::new(..5.0), y: Interval::new(..7.75)};
    assert_eq!(Rect::new(..Point2::new(5.0, 7.75)), r5);
    assert_eq!(Rect::new(..=Point2::new(5.0, 7.75)), r5);
    assert_eq!(Rect::new(..), Rect { x: Interval::new(..), y: Interval::new(..) });
}

#[test]
fn index_construction() {
    let r1 = Rect::new(Point2::new(3, 4)..=Point2::new(5, 7));
    let r2 = Rect::new(Point2::new(3, 4)..Point2::new(6, 8));
    assert_eq!(&r1, &r2);
    let r3 = Rect::new(&r1);
    let r4 = Rect::new(r2);
    assert_eq!(&r3, &r4);
}

#[test]
fn conversion() {
    let f1 = Rect::with_intervals(2.5..=5.75, 3.25..=8.5);
    let i1 = IndexRect::try_from(&f1).unwrap();
    let f2 = RealRect::from(&i1);
    let i2 = IndexRect::try_from(&f2).unwrap();
    assert_eq!(i1, i2);
    assert!(f2.contains(&f1));

    assert!(IndexRect::try_from(&RealRect::with_intervals(2.5..=5.75, ..6.0)).is_err());
    assert!(IndexRect::try_from(&RealRect::with_intervals(..=5.75, 3.5..6.0)).is_err());
    assert!(IndexRect::try_from(&RealRect::with_intervals(2.5..=5.75, 3.5..)).is_err());
    assert!(IndexRect::try_from(&RealRect::with_intervals(2.5.., 3.5..6.0)).is_err());
    assert!(IndexRect::try_from(&RealRect::with_intervals(.., ..)).is_err());
    assert_eq!(IndexRect::try_from(&RealRect::with_intervals((), ())).unwrap(),
               IndexRect::with_intervals((), ()));
    assert_eq!(RealRect::from(&IndexRect::with_intervals((), ())),
               RealRect::with_intervals((), ()));
}
