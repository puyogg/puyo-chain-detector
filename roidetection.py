import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
import cv2
from typing import Tuple, List

def pairwise_dist(x: np.ndarray) -> np.ndarray:
    """
    Calculate pairwise distances between data points.
    The main diagonal of the output is set to infinity so that
    running np.min(x) will ignore the distance to itself, which would
    always be trivially zero.
    """
    diff = x[np.newaxis, : , :] - x[:, np.newaxis, :]
    norm = np.linalg.norm(diff, 2, -1)
    dist = norm.transpose()
    np.fill_diagonal(dist, np.infty)
    return dist

def filter_field_rects(rects: np.ndarray, aspect_ratio=0.5333, threshold=0.1):
    """
    Extract a pair of ROIs that have a Width/Height ratio of 0.5333, like
    a normal game field. If a pair isn't found, the function returns None
    """
    # Sort field_rois by largest area
    areas = rects[:, 2] * rects[:, 3]
    rects = rects[np.argsort(-areas)]
    
    widths = rects[:, 2]
    heights = rects[:, 3] + 1e-8 # Avoid divide by 0 errors
    aspect_ratios = widths / heights

    # Calculate percent difference to expected aspect ratio
    field_ar_pdiff = (aspect_ratios - aspect_ratio) / ((aspect_ratios + aspect_ratio) / 2)

    # Get the rects for contours that have <5% difference
    game_fields = rects[field_ar_pdiff < threshold]

    # Get only the 2 that have the smallest distance between their y-pos, width, and height
    dists = pairwise_dist(game_fields[:, 1:4])
    paired_inds = np.array(np.unravel_index(np.argmin(dists), dists.shape))
    game_field_rois = game_fields[paired_inds]
    
    # Sort it so Player 1 (lowest X value) is first
    game_field_rois = game_field_rois[np.argsort(game_field_rois[:, 0]), :]

    return game_field_rois

def get_field_rois(im: np.ndarray, closing_kernel=(7, 7), return_im=False):
    # Grayscale
    im_gray = np.dot(im[...,:3], [0.2989, 0.5870, 0.1140])
    im_gray = im_gray.astype(np.uint8)

    # Contrast Stretching
    xp = [0, 64, 128, 192, 255]
    fp = [0, 32, 64, 240, 255]
    x = np.arange(256)
    table = np.interp(x, xp, fp).astype('uint8')
    new_im = table[im_gray]
    new_im = cv2.GaussianBlur(new_im, (5, 5), 0)

    # Get a mask for dark regions
    mask = np.zeros_like(new_im)
    mask[new_im < 40] = 255

    # Try to merge nearby contours
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, closing_kernel)
    closed = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    # Get the contours
    contours, hierarchy = cv2.findContours(closed, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
    # Only keep contours that outline white regions
    hierarchy = hierarchy.reshape(-1, 4)
    top_inds = np.argwhere(hierarchy[:, 3] == -1).reshape(-1)
    contours = [contours[i] for i in top_inds]
    hierarchy = hierarchy[top_inds]

    # Find bounding rects for each contour
    rects = np.array([cv2.boundingRect(contour) for contour in contours])
    rect_contour = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in rects]
    im_copy = np.copy(im[..., :3])
    cv2.drawContours(im_copy, rect_contour, -1, (255, 0, 0), 8)

    # Filter ROIs that should belong to player fields
    fields = filter_field_rects(rects)

    if return_im:
        rect_contour = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in fields]
        im_copy = np.copy(im[..., :3])
        cv2.drawContours(im_copy, rect_contour, -1, (255, 0, 0), 8)
        return fields, im_copy
    
    return fields

def get_score_rois(field_rois: np.ndarray) -> np.ndarray:
    score_rois = np.copy(field_rois)
    score_rois[:, 1] += field_rois[:, 3]
    score_rois[:, 3] = field_rois[:, 3] // 11 # Height adjustment
    score_rois[1, 0] = field_rois[1, 0] - field_rois[1, 2] * 0.02
    score_rois[:, 2] = field_rois[:, 2] * 1.02 # Width adjustment
    
    return scores

def get_next_rois(field_rois: np.ndarray) -> np.ndarray:
    window_rois = np.copy(field_rois)

    # Player 1 Next Window
    window_rois[0, 0] += field_rois[0, 2] * 1.08 # Shift right
    window_rois[0, 1] -= field_rois[0, 3] * 0.02 # Shift up
    window_rois[0, 2] *= 0.35 # Shrink horizontal
    window_rois[0, 3] *= 0.40 # Shrink vertical

    # Player 2 Next Window
    window_rois[1, 0] -= field_rois[0, 2] * 0.08 + field_rois[1, 2] * 0.35 # Shift left
    window_rois[1, 1] -= field_rois[0, 3] * 0.02 # Shift up
    window_rois[1, 2] *= 0.35 # Shrink horizontal
    window_rois[1, 3] *= 0.40 # Shrink vertical

    return window_rois

def get_name_rois(field_rois: np.ndarray) -> np.ndarray:
    name_rois = np.copy(field_rois)

    name_rois[:, 1] = field_rois[:, 1] + field_rois[:, 3] * 1.11 # Shift down
    name_rois[:, 3] = field_rois[:, 3] * 0.05 # Height adjustment
    name_rois[:, 0] = field_rois[:, 0] - field_rois[:, 2] * 0.13
    name_rois[:, 2] = field_rois[:, 2] * 1.25 # Width adjustment

    return name_rois

def get_win_roi(field_rois: np.ndarray) -> np.ndarray:
    win_roi = np.copy(field_rois[0]).reshape(1, -1)

    win_roi[0, 0] = field_rois[0, 0] + field_rois[0, 2] * 1.25
    win_roi[0, 1] = field_rois[0, 1] + field_rois[0, 3] * 1.11
    win_roi[0, 2] = field_rois[0, 2] * 0.96
    win_roi[0, 3] = field_rois[0, 3] * 0.09

    return win_roi

def get_icon_rois(field_rois: np.ndarray) -> np.ndarray:
    icon_rois = np.copy(field_rois)

    icon_rois[0, 0] = field_rois[0, 0] - field_rois[0, 2] * 0.37
    icon_rois[0, 1] = field_rois[0, 1] + field_rois[0, 3] * 1.07
    icon_rois[0, 2] = field_rois[0, 2] * 0.24
    icon_rois[0, 3] = field_rois[0, 3] * 0.13

    icon_rois[1, 0] = field_rois[1, 0] + field_rois[0, 2] * 1.39 - field_rois[1, 2] * 0.24
    icon_rois[1, 1] = field_rois[1, 1] + field_rois[0, 3] * 1.07
    icon_rois[1, 2] = field_rois[1, 2] * 0.24
    icon_rois[1, 3] = field_rois[1, 3] * 0.13

    return icon_rois

def get_background_mask(im, field_rois, score_rois, next_rois, name_rois, win_roi, icon_rois):
    mask = np.zeros(im.shape[0:2], dtype=np.uint8)

    field_contours = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in field_rois]
    score_contours = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in score_rois]
    next_contours  = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in next_rois]
    name_contours  = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in name_rois]
    win_contour    = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in win_roi]
    icon_contours  = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in icon_rois]

    cv2.drawContours(mask, field_contours, -1, 255, -1)
    cv2.drawContours(mask, score_contours, -1, 255, -1)
    cv2.drawContours(mask, next_contours, -1, 255, -1)
    cv2.drawContours(mask, name_contours, -1, 255, -1)
    cv2.drawContours(mask, win_contour, -1, 255, -1)
    cv2.drawContours(mask, icon_contours, -1, 255, -1)

    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (11,11))
    mask = cv2.dilate(mask, kernel, iterations = 5)
    mask = 255 - mask
    return mask

def draw_rois(im, roi_sets):
    im_copy = np.copy(im[..., :3])

    for roi_set in roi_sets:
        contours = [np.array([[[x, y]], [[x, y + h]], [[x + w, y + h]], [[x + w, y]]]) for x, y, w, h in roi_set]
        cv2.drawContours(im_copy, contours, -1, (255, 0, 0), 5)
    
    return im_copy

if __name__ == '__main__':
    im = Image.open('test-images/amitie_witch3.jpg').convert('RGB')
    im = im.resize((im.width // 2, im.height // 2))
    im = np.array(im)
    fields, im_field = get_field_rois(im, return_im=True)
    plt.imshow(im_field)
    plt.show()