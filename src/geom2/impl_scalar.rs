use super::{Index, Real, Scalar};

impl Scalar for Index {
    fn min(self, other: Self) -> Self {
        Ord::min(self, other)
    }
    fn max(self, other: Self) -> Self {
        Ord::max(self, other)
    }
    fn size(lower: Self, upper: Self) -> Self {
        1 + upper - lower
    }
}

impl Scalar for Real {
    fn min(self, other: Self) -> Self {
        self.min(other)
    }
    fn max(self, other: Self) -> Self {
        self.max(other)
    }
    fn size(lower: Self, upper: Self) -> Self {
        upper - lower
    }
}
