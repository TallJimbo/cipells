from ._cipells import (
    Real2, Index2,
    RealInterval, IndexInterval,
    RealBox, IndexBox,
    Identity, Translation, Jacobian, Affine,
    Image,
    Interpolant,
)
import numpy as np

__all__ = ("Real", "Real2", "Index", "Index2",
           "RealInterval", "IndexInterval",
           "RealBox", "IndexBox",
           "Identity", "Translation", "Jacobian", "Affine",
           "Image",
           "Interpolant",
           )

Real = np.float64
Index = np.int32

Real2.Scalar = Real
Index2.Scalar = Index

RealInterval.Scalar = Real
IndexInterval.Scalar = Index
IndexInterval.__iter__ = lambda self: iter(range(self.min, self.max + 1))
IndexInterval.__len__ = lambda self: self.size
IndexInterval.slice = property(lambda self: slice(self.min, self.max + 1, 1))
IndexInterval.arange = lambda self, dtype=Index: np.arange(self.min, self.max + 1, 1, dtype=dtype)

RealBox.Scalar = Real
RealBox.Interval = RealInterval
IndexBox.Scalar = Index
IndexBox.Interval = IndexInterval
IndexBox.slice = property(lambda self: (self.y.slice, self.x.slice))
IndexBox.meshgrid = lambda self, dtype=Index: np.meshgrid(self.x.arange(dtype), self.y.arange(dtype))
