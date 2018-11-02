use geom::{IndexPoint, IndexRect, IndexVector};
use ndarray::{s, ArrayBase, Data, Ix2, ViewRepr};

pub struct Image<S: Data> {
    pub array: ArrayBase<S, Ix2>,
    pub origin: IndexVector,
}

impl<A, S> Image<S>
where
    S: Data<Elem = A>,
{
    pub fn size(&self) -> IndexVector {
        IndexVector::from_iterator(self.array.shape().iter().map(|r| *r as i32))
    }

    pub fn bbox(&self) -> IndexRect {
        let min = IndexPoint {
            coords: self.origin,
        };
        IndexRect::new(min..min + self.size())
    }

    pub fn slice(&self, rect: &IndexRect) -> Image<ViewRepr<&A>> {
        let y = rect.y.shifted_by(-self.origin.y);
        let x = rect.x.shifted_by(-self.origin.x);
        Image {
            array: self.array.slice(s![
                y.min().unwrap()..=y.max().unwrap(),
                x.min().unwrap()..=x.max().unwrap()
            ]),
            origin: self.origin + rect.min().unwrap().coords,
        }
    }
}
