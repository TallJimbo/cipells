import numpy as np

from .continuous1d import *  # noqa


class TruncatedGaussian:

    @staticmethod
    def fitPoly(m1, m2):
        n = 4
        m1e = np.exp(-0.5*m1**2)/np.sqrt(2*np.pi)
        p = np.arange(n)
        A = np.zeros((n, n), dtype=float)
        b = np.zeros(n, dtype=float)
        A[0, :] = m1**p
        b[0] = m1e
        A[1, :] = m2**p
        b[1] = 0.0
        A[2, :] = p*m1**(p - 1)
        b[2] = -m1*m1e
        A[3, :] = p*m2**(p - 1)
        b[3] = 0.0
        return np.linalg.solve(A, b)

    def __init__(self, m1, m2):
        self.m1 = m1
        self.m2 = m2
        self.poly = np.poly1d(self.fitPoly(m1, m2)[::-1])

    def __call__(self, r):
        z = np.zeros(r.shape, dtype=float)
        r = np.abs(r)
        z[r > self.m2] = 0.0
        mask1 = np.logical_and(r > self.m1, r <= self.m2)
        z[mask1] = self.poly(r[mask1])
        mask2 = r <= self.m1
        z[mask2] = np.exp(-0.5*r[mask2]**2)/np.sqrt(2*np.pi)
        return z
