import numpy as np
import matplotlib.pyplot as plt
import itertools
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

def get_intersect(a: np.ndarray, b: np.ndarray) -> Tuple[float, float]:
    """ 
    https://stackoverflow.com/questions/3252194/numpy-and-line-intersections
    Returns the point of intersection of the lines passing through a2,a1 and b2,b1.
    Args:
        a (np.ndarray): Two sets of [x y], the points that make the first line
        b (np.ndarray): Two sets of [x y], the points that make the second line
    Returns:
        point (Tuple(x, y)): Tuple of floats marking the intersection of a and b.
    """
    a1, a2 = a
    b1, b2 = b
    s = np.vstack([a1, a2, b1, b2])
    h = np.hstack((s, np.ones((4, 1))))
    l1 = np.cross(h[0], h[1])
    l2 = np.cross(h[2], h[3])
    x, y, z = np.cross(l1, l2)

def get_line_points(lines: np.ndarray) -> np.ndarray:
    """
    Convert OpenCV's rho/theta format for HoughLines to two (x, y) pairs of points
    that can be plotted.
    Args:
        lines (np.ndarray): Array of two sets of pairs.
    Returns:
        line_points (List[[pt1, pt2]])
    """
    line_points = []
    for i in range(0, len(lines)):
        rho = lines[i][0][0]
        theta = lines[i][0][1]
        a = np.cos(theta)
        b = np.sin(theta)
        x0 = a * rho
        y0 = b * rho
        pt1 = (int(x0 + 1000*(-b)), int(y0 + 1000*(a)))
        pt2 = (int(x0 - 1000*(-b)), int(y0 - 1000*(a)))
        line_points.append([pt1, pt2])
    return line_points

def screen_rois(im: np.ndarray) -> List[Tuple[int, int, int, int]]:
    """
    Get the main regions of interest (ROI) for a Puyo match.
    If the ROIS can't be found, the function will return None.
    Args:
        im (np.ndarray): A color image. It can be 3 or 4 channel.
    Returns:
        rois (List[cv2 Rects]): A list of (x, y, w, h) tuples that map
            to the important regions of the game screen.
    """
    # 1. Convert to grayscale
    im_gray = np.dot(im[...,:3], [0.2989, 0.5870, 0.1140])

    # 2. Apply Canny Edge detector
    edges = cv2.Canny(im, 0, 30)

    # 3. Apply Hough Lines Transform
    lines = cv2.HoughLines(edges, 1, np.pi / 18, 300, None, 0, 0)
    if a is None: return

    # Convert cv2's rho/theta output to paired points
    line_points = get_line_points(lines)
    # Get intersections
    line_pairs = [comb for comb in itertools.combinations(line_points, 2)]
    intersections = []
    for line0, line1 in line_pairs:
        intersect = get_intersect(line0, line1)
        if intersect[0] != np.inf and intersect[1] != np.inf:
            intersections.append(intersect)
    intersections = np.array(intersections)

    # 4. Get bounding box for intersection points
    x0, y0 = np.amin(intersections, axis=0)
    x1, y1 = np.amax(intersections, axis=0)
    x0, y0, x1, y1 = np.int0((x0, y0, x1, y1))
    intersect_mask = np.zeros_like(edges)
    intersect_mask[y0:y1, x0:x1] = 255

    # 5. Add lines to the intersect mask
    for pt1, pt2 in line_points:
        cv2.line(intersect_mask, pt1, pt2, 0, 2, cv2.LINE_AA)
    
    # 6. Apply binary opening to remove thin regions
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (15, 15))
    opening = cv2.morphologyEx(middle_mask, cv2.MORPH_OPEN, kernel)

    # 7. Find contours
    contours, _ = cv2.findContours(opening, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    rects = np.array([cv2.boundingRect(contour) for contour in contours], dtype=np.int)

    return rects

def player_fields(rects: List[Tuple[int, int, int, int]], aspect_ratio=0.5333, threshold=0.05):
    """
    Extract a pair of ROIs that have a Width/Height ratio of 0.5333, like
    a normal game field. If a pair isn't found, the function returns None
    """
    widths = rects[:, 2]
    heights = rects[:, 3] + 1e-8 # Avoid divide by 0 errors
    aspect_ratios = widths / heights

    # Calculate percent difference to expected aspect ratio
    field_ar_pdiff = (aspect_ratios - field_ar) / ((aspect_ratios + field_ar) / 2)

    # Get the rects for contours that have <5% difference
    game_fields = rects[field_ar_pdiff < threshold]

    # Get only the 2 that have the smallest distance between their y-pos, width, and height
    dists = pairwise_dist(game_fields[:, 1:4])
    paired_inds = np.array(np.unravel_index(np.argmin(dists), dists.shape))
    game_field_rois = game_fields[paired_inds]
    
    # Sort it so Player 1 (lowest X value) is first
    game_field_rois = game_field_rois[np.argsort(game_field_rois[:, 0]), :]

    return game_field_rois