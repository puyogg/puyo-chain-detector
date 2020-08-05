# %%
import numpy as np
from PIL import Image
# import matplotlib.pyplot as plt
from chainsim import color_codes

# %%
def alpha_blend(bg: np.ndarray, top: np.ndarray):
    '''
    https://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending
    '''
    # # Fix alpha=0 in top layer.
    # top[:, :, :3][top[:, :, 3] == 0] = 0
    
    bg = bg.astype(np.float32) / 255.0
    top = top.astype(np.float32) / 255.0
    a = top[:, :, 3]
    a = a[:, :, np.newaxis] * np.array([1, 1, 1])
    top_rgb = top[:, :, :3]
    # return np.array((top_rgb * a + bg[:, :, :3] * (1 - a)) * 255, dtype=np.uint8)
    return ((top_rgb * a + bg[:, :, :3] * (1 - a)) * 255).astype(np.uint8)

green_screen = Image.open('green-screen/green_bg.png')
green_screen = np.array(green_screen)
screen = np.copy(green_screen)

# color_codes = np.array(['0', 'J', 'R', 'G', 'B', 'Y', 'P'])
cursors = {
    'R': np.array(Image.open('green-screen/red_cursor.png').resize((64, 60)), dtype=np.uint8),
    'G': np.array(Image.open('green-screen/green_cursor.png').resize((64, 60)), dtype=np.uint8),
    'B': np.array(Image.open('green-screen/blue_cursor.png').resize((64, 60)), dtype=np.uint8),
    'Y': np.array(Image.open('green-screen/yellow_cursor.png').resize((64, 60)), dtype=np.uint8),
    'P': np.array(Image.open('green-screen/purple_cursor.png').resize((64, 60)), dtype=np.uint8)
}

numbers = {
    2: np.array(Image.open('green-screen/2.png').resize((64, 60)), dtype=np.uint8),
    3: np.array(Image.open('green-screen/3.png').resize((64, 60)), dtype=np.uint8),
    4: np.array(Image.open('green-screen/4.png').resize((64, 60)), dtype=np.uint8),
    5: np.array(Image.open('green-screen/5.png').resize((64, 60)), dtype=np.uint8),
    6: np.array(Image.open('green-screen/6.png').resize((64, 60)), dtype=np.uint8),
    7: np.array(Image.open('green-screen/7.png').resize((64, 60)), dtype=np.uint8),
    8: np.array(Image.open('green-screen/8.png').resize((64, 60)), dtype=np.uint8),
    9: np.array(Image.open('green-screen/9.png').resize((64, 60)), dtype=np.uint8),
    10: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    11: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    12: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    13: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    14: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    15: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    16: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    17: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    18: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8),
    19: np.array(Image.open('green-screen/omg.png').resize((64, 60)), dtype=np.uint8)
}

cursor_num = {}
green_square = np.zeros((60, 64, 3))
green_square[:, :, 1] = 255
for color, cursor_im in cursors.items():
    for number, number_im in numbers.items():
        cell = alpha_blend(green_square, cursor_im)
        cell = alpha_blend(cell, number_im)
        cursor_num[(color, number)] = cell

# %%
im = Image.open('green-screen/2.png')
im = np.array(im)

# %%
def update_greenscreen(screen: np.ndarray, field_roi: np.ndarray, try_inds, try_colors, chain_lengths):
    orig_width, orig_height = field_roi[2], field_roi[3]

    target_height = 30 * 12
    target_width = 32 * 6

    diff_x = orig_width - target_width
    diff_y = orig_height - target_height

    centered_roi = np.array([field_roi[0] + diff_x // 2, field_roi[1] + diff_y // 2, 32 * 6, 30 * 12])
    
    scaled_roi = centered_roi * 2
    scaled_roi[1] = scaled_roi[1] - 60 # Height adjustment for 13th row

    field_maxes = np.zeros((13, 6), dtype=int)
    cursor_colors = np.zeros((13, 6), dtype=int)

    for i in range(len(try_inds)):
        row, col = try_inds[i]
        color = try_colors[i]
        length = chain_lengths[i]

        if length >= 2 and length > field_maxes[row, col]:
            field_maxes[row, col] = length
            cursor_colors[row, col] = color
    
    corner_x, corner_y = scaled_roi[0], scaled_roi[1]
    for row in range(1, 13):
        for col in range(6):
            if field_maxes[row, col] > 0:
                x0, x1 = corner_x + 64 * col, corner_x + 64 * (col + 1)
                y0, y1 = corner_y + 60 * row, corner_y + 60 * (row + 1)

                letter = color_codes[cursor_colors[row, col]]
                number = field_maxes[row, col]
                cell = cursor_num[(letter, number)]

                screen[y0:y1, x0:x1] = cell

def get_greenscreen(field_rois: np.ndarray, p1_data, p2_data):
    screen[:] = green_screen[:]

    try_inds, try_colors, chain_length = p1_data
    update_greenscreen(screen, field_rois[0], try_inds, try_colors, chain_length)

    try_inds, try_colors, chain_length = p2_data
    update_greenscreen(screen, field_rois[1], try_inds, try_colors, chain_length)

    return screen