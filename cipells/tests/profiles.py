import unittest
import numpy as np

from cipells import Gaussian, Identity, Affine


class GaussianTestCase(unittest.TestCase):

    def testStandard(self):
        r = np.linspace(-5.0, 5.0, 101)
        x, y = np.meshgrid(r, r)
        flux = np.random.randn()
        g = Gaussian(Identity(), flux=flux)
        np.testing.assert_allclose(g(x, y), flux*np.exp(-0.5*(x**2 + y**2))/(2.0*np.pi))


if __name__ == "__main__":
    unittest.main()
