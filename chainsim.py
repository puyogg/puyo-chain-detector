import numpy as np
from skimage import measure
from scipy import signal
import time
import math

# Game constants
CHAIN_POWER = np.array([0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672])
COLOR_BONUS = np.array([0, 3, 6, 12, 24])
GROUP_BONUS = np.array([0, 2, 3, 4, 5, 6, 7, 10])

# Constants for simulator
color_codes = np.array(['0', 'J', 'R', 'G', 'B', 'Y', 'P']) # Mask for converting field to code representation
fallable_nums = np.array([1, 2, 3, 4, 5, 6])
no_garbage = np.array([0, 0, 2, 3, 4, 5, 6]) # Mask for seeing the field without garbage
PUYO_TYPE = {
    'NONE': 0,
    'GARBAGE': 1,
    'RED': 2,
    'GREEN': 3,
    'BLUE': 4,
    'YELLOW': 5,
    'PURPLE': 6
}

def _apply_drop(field: np.ndarray):
    for i in range(6):
        col = field[:, i]
        not_zero = col[col != 0]
        if len(not_zero) > 0:
            field[:, i] = 0
            field[-len(not_zero):, i] = not_zero
    return field

def _calculate_garbage_pops(field, color_pop_mask):
    garbages = np.argwhere(np.isin(field, PUYO_TYPE['GARBAGE']))
    garbage_to_clear = []
    for i, pos in enumerate(garbages):
        y, x = pos
        if ((y > 1 and color_pop_mask[y - 1, x] == True) or (y < 12 and color_pop_mask[y + 1, x] == True) or (x > 0 and color_pop_mask[y, x - 1] == True) or (x < 6 and color_pop_mask[y, x + 1] == True)):
            garbage_to_clear.append(i)
    return garbages[garbage_to_clear]

def _analyze_pops(field: np.ndarray, puyo_to_pop=4):
    # Calculated connected components
    labels = measure.label(no_garbage[field], background=0, connectivity=1)

    # Groups that are big enough to pop
    groups, count = np.unique(labels[labels != 0], return_counts=True)
    pop_labels = groups[count >= puyo_to_pop]
    pop_counts = count[count >= puyo_to_pop]

    # Has pop? Boolean variable
    has_pops = pop_labels.shape[0] > 0

    # Get where the colors are popping
    color_pop_mask = np.isin(labels, pop_labels)

    # Number of colors popping
    unique = np.unique(field[color_pop_mask])
    color_count = unique.shape[0]

    return has_pops, pop_counts, color_count, color_pop_mask

def _calculate_step_score(step, pop_counts, color_count, puyo_to_pop):
    # Determine group bonus
    link_group_bonus = 0
    link_puyo_cleared = 0
    for count in pop_counts:
        link_puyo_cleared += count
        if count >= 7 + puyo_to_pop: link_group_bonus += GROUP_BONUS[-1]
        else: link_group_bonus += GROUP_BONUS[count - puyo_to_pop]

    # Number of different colors
    link_color_bonus = COLOR_BONUS[color_count - 1]

    if step >= len(CHAIN_POWER):
        link_chain_power = CHAIN_POWER[-1]
    else:
        link_chain_power = CHAIN_POWER[step - 1]

    link_total_bonuses = link_group_bonus + link_color_bonus + link_chain_power
    if link_total_bonuses < 1: link_total_bonuses = 1
    elif link_total_bonuses > 999: link_total_bonuses = 999

    return 10 * link_puyo_cleared * link_total_bonuses

def simulate_chain(field: np.ndarray, step=0, puyo_to_pop=4, score=0, target_point=70):
    # Check for drops
    field = _apply_drop(field)
    
    # Check for pops
    has_pops, pop_counts, color_count, color_pop_mask = _analyze_pops(field, puyo_to_pop)
    garbage_to_clear = _calculate_garbage_pops(field, color_pop_mask)

    if has_pops:
        current_step = step + 1 # Increase chain length
        step_score = _calculate_step_score(current_step, pop_counts, color_count, puyo_to_pop)
        total_score = score + step_score
        field[color_pop_mask] = 0
        field[garbage_to_clear[:, 0], garbage_to_clear[:, 1]] = 0
        return simulate_chain(field, step=current_step, puyo_to_pop=puyo_to_pop, score=total_score, target_point=target_point)
    else:
        damage = math.floor(score / target_point)
        return field, step, score, damage

if __name__ == '__main__':
    field = np.array([[0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [3, 0, 0, 0, 0, 0],
                  [0, 0, 5, 0, 0, 0],
                  [0, 1, 5, 0, 0, 0],
                  [0, 5, 5, 0, 0, 0],
                  [3, 5, 5, 1, 1, 6],
                  [3, 2, 4, 5, 6, 6],
                  [3, 3, 2, 4, 5, 5],
                  [2, 2, 4, 4, 5, 6]], dtype=np.int)

    since = time.time()
    for i in range(100):
        new_field, step, score, damage = simulate_chain(np.copy(field))
    print('Analysis time: ', (time.time() - since) * 60, 'frames')