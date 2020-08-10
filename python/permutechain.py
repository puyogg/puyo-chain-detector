# %% Imports
import numpy as np
from chainsim import simulate_chain

# %%
no_garbage = np.array([0, 0, 2, 3, 4, 5, 6], dtype=np.int)

# %% Helper Functions
def check_for_drops(field: np.ndarray) -> bool:
    for x in range(field.shape[1]):
        for y in range(1, field.shape[0]):
            if field[y, x] == 0 and field[y - 1, x] > 0:
                return True
    return False

def remove_drops(field: np.ndarray) -> np.ndarray:
    for x in range(field.shape[1]):
        for y in reversed(range(field.shape[0] - 1)):
            if field[y, x] > 0 and field[y + 1, x] == 0:
                field[y, x] = 0
    return field

def get_surface_inds(field: np.ndarray) -> bool:
    x = np.arange(field.shape[1])
    y = field.shape[0] - np.count_nonzero(field, axis=0) - 1
    ind_list = np.array([x, y]).transpose()
    return ind_list[ind_list[:, 1] >= 1, :]

def get_colors_to_try(field: np.ndarray, surface_inds: np.ndarray):
    cols = []

    for x, y in surface_inds:
        to_try = []
        if x > 1 and field[y, x - 1] > 1:
            to_try.append(field[y, x - 1])
        if x < 5 and field[y, x + 1] > 1:
            to_try.append(field[y, x + 1])
        if y < 12 and field[y + 1, x] > 1:
            to_try.append(field[y + 1, x])
        cols.append(to_try)

    return cols

def _get_fields_to_try(field: np.ndarray, surface_inds: np.ndarray, colors_to_try):
    fields_to_try = []
    try_inds = []
    try_colors = []
    for x in range(len(colors_to_try)):
        y = surface_inds[x, 1]
        for c in colors_to_try[x]:
            try_inds.append((y, x))
            try_colors.append(c)
            new_field = np.copy(field)
            new_field[y, x] = c
            if y > 1:
                new_field[y - 1, x] = c
            fields_to_try.append(new_field)
    
    return fields_to_try, try_inds, try_colors

def get_fields_to_try(field):
    surface_inds = get_surface_inds(field)
    colors_to_try = get_colors_to_try(field, surface_inds)
    fields_to_try, try_inds, try_colors = _get_fields_to_try(field, surface_inds, colors_to_try)
    return fields_to_try, try_inds, try_colors

def get_all_chain_lengths(fields_to_try):
    result = [simulate_chain(field) for field in fields_to_try]
    return [step for field, step, score, damage in result]

# %% Set Field
field = np.array([[0, 0, 0, 0, 0, 1],
                [0, 0, 0, 0, 0, 1],
                [0, 0, 0, 0, 5, 1],
                [0, 0, 0, 0, 1, 1],
                [0, 0, 0, 0, 1, 1],
                [0, 0, 0, 0, 1, 1],
                [0, 0, 5, 0, 1, 1],
                [0, 1, 5, 0, 3, 1],
                [3, 5, 5, 0, 3, 1],
                [3, 5, 5, 1, 1, 6],
                [3, 2, 4, 5, 6, 6],
                [3, 3, 2, 4, 5, 5],
                [2, 2, 4, 4, 5, 6]], dtype=np.int)

field = np.array([[0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 0, 0],
                  [3, 0, 0, 0, 0, 0],
                  [4, 0, 0, 0, 0, 0],
                  [4, 0, 0, 0, 0, 1],
                  [2, 2, 0, 3, 1, 1],
                  [4, 4, 2, 1, 1, 6],
                  [3, 2, 4, 5, 6, 6],
                  [3, 3, 2, 4, 5, 5],
                  [2, 2, 4, 4, 5, 6]], dtype=np.int)

# %% Test has drop
has_drop = check_for_drops(field)

# %%
surface_inds = get_surface_inds(field)
print(surface_inds)

# %%
colors_to_try = get_colors_to_try(field, surface_inds)
print(colors_to_try)

# %%
fields_to_try = _get_fields_to_try(field, surface_inds, colors_to_try)
print(fields_to_try)

# %%
fields_to_try, try_inds, try_colors = get_fields_to_try(field)

# %%
lengths = get_all_chain_lengths(fields_to_try)
print(try_inds, try_colors, lengths)


# %%
