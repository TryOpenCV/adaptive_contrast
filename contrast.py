import cv2
import numpy as np

class Area:
    n1, n2 = 2, 2
    m1, m2 = 2, 2

def get_indices(i, j, width, height):
    i_min = i - Area.n1
    i_max = i + Area.n2
    j_min = j - Area.m1
    j_max = j + Area.m2

    if i_min < 0:
        i_min = 0
    if i_max > width:
        i_max = width
    if j_min < 0:
        j_min = 0
    if j_max > height:
        j_max = height

    return i_min, i_max, j_min, j_max

def contrast(z, zc):
    return abs(z-zc)/float(z+zc)

def contrast_gain(cz):
    return cz**0.8

def get_new_pixel(z, zc, c):
    if z < zc:
        return int(zc*(1-c)/float(1+c))
    else:
        return int(zc*(1+c)/float(1-c))


img = cv2.imread('lenna.bmp', 0)
width, height = img.shape
img_new = cv2.imread('lenna.bmp', 0)

for i in range(0, width):
    for j in range(0, height):
        i_min, i_max, j_min, j_max = get_indices(i, j, width, height)
        area = img[i_min:i_max, j_min:j_max]
        average_brightness = int(round(area.sum()/area.size))
        pixel_brightness = img[i,j]
        pixel_contrast = contrast(pixel_brightness, average_brightness)

        gained_contrast = contrast_gain(pixel_contrast)
        new_pixel = get_new_pixel(pixel_brightness, average_brightness, gained_contrast)
        img_new[i,j] = new_pixel


cv2.imwrite('lenna_adaptive.bmp', img_new)
cv2.imshow('image',img_new)
cv2.waitKey(0)
cv2.destroyAllWindows()
