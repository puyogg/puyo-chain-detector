import numpy as np
import joblib
from skimage.util.shape import view_as_blocks
import sklearn
import cv2

clf = joblib.load('neuralnet-50-50.joblib')
zeros = np.zeros((1, 6))

def crop_im(im: np.ndarray, roi):
    x0, x1 = roi[0], roi[0] + roi[2]
    y0, y1 = roi[1], roi[1] + roi[3]
    return im[y0:y1, x0:x1]

### Functions for getting the game field ###
def crop_centered_field(field_im):
    orig_height, orig_width = field_im.shape[:2]

    target_height = 30 * 12
    target_width = 32 * 6

    diff_x = orig_width - target_width
    diff_y = orig_height - target_height

    x0 = max(0, diff_x // 2)
    y0 = max(0, diff_y // 2)
    x1 = min(x0 + target_width, orig_width)
    y1 = min(y0 + target_height, orig_height)

    cropped = field_im[y0:y1, x0:x1]
    if cropped.shape[0] != target_height or cropped.shape[1] != target_width:
        cropped = cv2.resize(cropped, (target_width, target_height), interpolation=cv2.INTER_LINEAR)

    return cropped

def get_cell_data(field_im: np.ndarray, block_shape=(30, 32, 3)):
    # print(field_im.shape)
    return view_as_blocks(field_im, block_shape=block_shape).reshape((12 * 6, - 1))

def analyze_cell_data(cell_data: np.ndarray):
    preds = clf.predict(cell_data).reshape((12, 6))
    return np.concatenate([zeros, preds], axis=0)

def get_field_chain(field_im):
    centered_field = crop_centered_field(field_im)
    cell_data = get_cell_data(centered_field)
    field_chain = analyze_cell_data(cell_data)
    field_chain = field_chain.astype(np.int)
    return field_chain
