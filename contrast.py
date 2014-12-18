import cv2
import numpy as np

class Config:
    n1, n2 = 3, 3
    m1, m2 = 2, 2
    n_min, n_max = 0.13, 0.77
    # n_min: 0.1 ... 0.3
    # n_max: 0.7 ... 0.9
    fixed_kns = 0.8
    kns_type = 'std' # 'fixed', 'std' or 'entropy'

    min_std = 0
    max_std = float('inf')

def get_min_max_std(img):
    width, height = img.shape
    for i in range(0, width):
        for j in range(0, height):
            i_min, i_max, j_min, j_max = get_indices(i, j, width, height)
            area = img[i_min:i_max, j_min:j_max]
            std = np.std(area)

            if std < Config.min_std:
                Config.min_std = std
            if std > Config.max_std:
                Config.max_std = std

def get_indices(i, j, width, height):
    i_min = i - Config.n1
    i_max = i + Config.n2
    j_min = j - Config.m1
    j_max = j + Config.m2

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

def get_kns(area):
    if Config.kns_type == 'fixed':
        return Config.fixed_kns
    if Config.kns_type == 'std':
        return (np.std(area) - Config.min_std) / float(Config.max_std - Config.min_std)

def get_adaptive_power(area):
    return Config.n_min+(Config.n_max-Config.n_min)*get_kns(area)

def contrast_gain_adaptive(cz, area):
    return cz**get_adaptive_power(area)

def get_new_pixel(z, zc, c):
    if z < zc:
        return int(zc*(1-c)/float(1+c))
    else:
        return int(zc*(1+c)/float(1-c))

img = cv2.imread('lenna.bmp', 0)
width, height = img.shape
img_new = cv2.imread('lenna.bmp', 0)

if Config.kns_type == 'std':
    get_min_max_std(img)
    print Config.min_std, Config.max_std

for i in range(0, width):
    for j in range(0, height):
        i_min, i_max, j_min, j_max = get_indices(i, j, width, height)
        area = img[i_min:i_max, j_min:j_max]
        average_brightness = int(round(area.sum()/area.size))
        pixel_brightness = img[i,j]
        pixel_contrast = contrast(pixel_brightness, average_brightness)

        gained_contrast = contrast_gain_adaptive(pixel_contrast, area)
        new_pixel = get_new_pixel(pixel_brightness, average_brightness, gained_contrast)
        img_new[i,j] = new_pixel
        print new_pixel

cv2.imwrite('lenna_adaptive.bmp', img_new)
cv2.imshow('image',img_new)
cv2.waitKey(0)
cv2.destroyAllWindows()
