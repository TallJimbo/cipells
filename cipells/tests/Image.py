import unittest
import numpy as np

from cipells import Image, IndexBox, Index2


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

    def testFloatImage(self):
        box = IndexBox(min=(1, 2), max=(5, 4))
        image = Image(box, dtype=np.float32)
        array = np.random.randn(box.height, box.width).astype(image.dtype)
        self.checkImage(image, array, box)


if __name__ == "__main__":
    unittest.main()
