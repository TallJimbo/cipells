import unittest
import numpy as np

from cipells import Image, IndexBox, Index2
from cipells.tests import passImage, passImageToConst, passConstImage


class ImageTestCase(unittest.TestCase):

    def checkImage(self, image, array, box):
        self.assertEqual(image.bbox, box)
        image.array = array
        np.testing.assert_array_equal(image.array, array)
        for point in box.corners:
            self.assertEqual(image[point], array[point.y - box.y0, point.x - box.x0])
            self.assertEqual(image[point], image[point.x, point.y])
        with self.assertRaises(IndexError):
            image[box.min - Index2(0, 1)]
        with self.assertRaises(IndexError):
            image[box.max + Index2(1, 0)]
        subbox = box.erodedBy(1)
        subimage = image[subbox]
        self.assertEqual(subimage.bbox, subbox)
        array_subbox = subbox.shiftedBy(-box.min)
        np.testing.assert_array_equal(subimage.array, array[array_subbox.y.slice, array_subbox.x.slice])
        image1 = passImage(image)
        self.assertIsInstance(image1, Image)
        self.assertEqual(image1.dtype, image.dtype)
        self.assertEqual(image1.bbox, image.bbox)
        np.testing.assert_array_equal(image.array, array)
        cimage1 = passImageToConst(image)
        self.assertIsInstance(cimage1, Image)
        self.assertEqual(cimage1.dtype, image.dtype)
        self.assertEqual(cimage1.bbox, image.bbox)
        np.testing.assert_array_equal(cimage1.array, array)
        cimage2 = passConstImage(cimage1)
        self.assertIsInstance(cimage2, Image)
        self.assertEqual(cimage2.dtype, image.dtype)
        self.assertEqual(cimage2.bbox, image.bbox)
        np.testing.assert_array_equal(cimage2.array, array)
        with self.assertRaises(TypeError):
            passImage(cimage1)
        with self.assertRaises(TypeError):
            cimage1[point] = 0.0
        with self.assertRaises(ValueError):
            cimage1.array = 0.0
        image2 = image.copy()
        # modify the original image, and make sure all views are updated.
        image.array = 1.0
        np.testing.assert_array_equal(image.array, np.ones(image.array.shape, dtype=image.dtype))
        np.testing.assert_array_equal(image.array, image1.array)
        np.testing.assert_array_equal(image.array, cimage1.array)
        np.testing.assert_array_equal(image.array, cimage2.array)
        np.testing.assert_array_equal(subimage.array,
                                      image.array[array_subbox.y.slice, array_subbox.x.slice])
        np.testing.assert_array_equal(image2.array, array)  # copy, so not updated like the views

    def testFloatImage(self):
        box = IndexBox(min=(1, 2), max=(5, 4))
        image = Image(box, dtype=np.float32)
        array = np.random.randn(box.height, box.width).astype(image.dtype)
        self.checkImage(image, array, box)

    def testComplexFloatImage(self):
        box = IndexBox(min=(1, 2), max=(5, 4))
        image = Image(box, dtype=np.complex64)
        real = np.random.randn(box.height, box.width).astype(np.complex64)
        imag = np.random.randn(box.height, box.width).astype(np.complex64)
        array = real + 1j*imag
        self.assertEqual(array.dtype, np.dtype(np.complex64))
        self.checkImage(image, array, box)


if __name__ == "__main__":
    unittest.main()
