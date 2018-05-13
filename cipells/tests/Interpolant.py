import unittest
import numpy as np

from cipells import Interpolant


class InterpolantTestCase(unittest.TestCase):

    def testCubicNodes(self):
        self.assertEqual(Interpolant.cubic(0.0), 1.0)
        np.testing.assert_array_equal(Interpolant.cubic(np.arange(1, 3, 1)), 0)
        np.testing.assert_array_equal(Interpolant.cubic(np.arange(-1, -3, -1)), 0)
        np.testing.assert_array_equal(Interpolant.cubic(np.linspace(2, 5, 10)), 0)
        np.testing.assert_array_equal(Interpolant.cubic(np.linspace(-2, -5, 10)), 0)

    def testQuinticNodes(self):
        self.assertEqual(Interpolant.quintic(0.0), 1.0)
        np.testing.assert_array_equal(Interpolant.quintic(np.arange(1, 4, 1)), 0)
        np.testing.assert_array_equal(Interpolant.quintic(np.arange(-1, -4, -1)), 0)
        np.testing.assert_array_equal(Interpolant.quintic(np.linspace(3, 6, 10)), 0)
        np.testing.assert_array_equal(Interpolant.quintic(np.linspace(-3, -6, 10)), 0)

    def testSincNodes(self):
        self.assertEqual(Interpolant.sinc(0.0), 1.0)
        np.testing.assert_allclose(Interpolant.sinc(np.arange(1, 10, 1)), 0, atol=1E-16)
        np.testing.assert_allclose(Interpolant.sinc(np.arange(-1, -11, -1)), 0, atol=1E-16)

    def testInterpolatePolynomial(self):
        poly2 = np.poly1d([0.05, -0.35, 0.25])
        poly4 = np.poly1d([-0.10, 0.05, 0.30, -0.85, 0.25])
        xa = np.arange(-10, 11, 1)
        xb = np.linspace(-5, 5, 81)
        # Cubic and Quartic interpolants can interpolate a quadratic
        # polynomial exactly (up to round-off error).
        np.testing.assert_allclose(
            np.dot(Interpolant.cubic(np.subtract.outer(xb, xa)), poly2(xa)),
            poly2(xb),
            rtol=1E-12
        )
        np.testing.assert_allclose(
            np.dot(Interpolant.quintic(np.subtract.outer(xb, xa)), poly2(xa)),
            poly2(xb),
            rtol=1E-12
        )
        # Quintic interpolant can interpolate a quartic polynomial
        # exactly (up to round-off error).
        np.testing.assert_allclose(
            np.dot(Interpolant.quintic(np.subtract.outer(xb, xa)), poly4(xa)),
            poly4(xb),
            rtol=1E-12
        )

    def testInterpolateGaussian(self):
        xa = np.arange(-20, 21, 1)
        xb = np.linspace(-5, 5, 81)

        def gaussian(x):
            return np.exp(-0.5*x**2/9.0)

        # Sinc interpolant can interpolate Nyquist-sampled functions (which
        # this Gaussian approximately is) exactly (up to round-off error).
        np.testing.assert_allclose(
            np.dot(Interpolant.sinc(np.subtract.outer(xb, xa)), gaussian(xa)),
            gaussian(xb),
            rtol=1E-12
        )
        # Quintic and cubic introduce some errors into the interpolation, but
        # are still pretty close.
        np.testing.assert_allclose(
            np.dot(Interpolant.quintic(np.subtract.outer(xb, xa)), gaussian(xa)),
            gaussian(xb),
            rtol=1E-3
        )
        np.testing.assert_allclose(
            np.dot(Interpolant.cubic(np.subtract.outer(xb, xa)), gaussian(xa)),
            gaussian(xb),
            rtol=1E-2
        )


if __name__ == "__main__":
    unittest.main()
