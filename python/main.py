# %% Imports
from chainsim import simulate_chain
from roidetection import get_next_rois, get_score_rois, draw_rois, get_field_rects, get_similar_roi
from greenscreen import get_greenscreen
from playerstate import PlayerState
import numpy as np
import cv2
import time
from collections import deque
import json

settings = json.load(open('settings.json'))
mode = settings['mode']
device_id = settings['device_id']

cap = cv2.VideoCapture(device_id, mode)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 540)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 960)

# Set defaults for field ROIs
field_rois = np.array([[136,  79, 200, 361], [624,  79, 199, 361]])
next_rois = get_next_rois(field_rois)
score_rois = get_score_rois(field_rois)

# Initialize state trackers
player_1 = PlayerState(1, field_rois[0], next_rois[0], score_rois[0])
player_2 = PlayerState(2, field_rois[1], next_rois[1], score_rois[1])

def update(i, frame, reset, roi):
    if i == 0:
        return player_1.update(frame, reset, roi)
    else:
        return player_2.update(frame, reset, roi)

prev = time.time()
confirmation = 50
fps_queue = deque()
tick = 0
# field_find_queue = deque()
# chain_inf_queue = deque()
# green_scr_queue = deque()
# bookkeep_queue = deque()

while True:
    if confirmation > 0:
        confirmation -= 1

    tick = (tick + 1) % 120
    if tick == 119:
        list_fps = list(fps_queue)
        avg_fps = np.mean(list_fps)
        std_fps = np.std(list_fps)
        print('Detection Speed (frames): ' + str(round(avg_fps, 2)) + ' +/- ' + str(round(std_fps, 2)))

    if len(fps_queue) > 30:
        fps_queue.popleft()
        # field_find_queue.popleft()
    
    time_elapsed = (time.time() - prev) * 60
    prev = time.time()
    fps_queue.append(time_elapsed)
    
    ret, frame = cap.read()
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    reset = [False, False]
    rois = [None, None]
    p1_field_roi = None
    p2_field_roi = None

    # Confirm the positions of the field rois
    rects = get_field_rects(frame)

    if len(rects) == 0:
        reset = [True, True]
    else:
        # Find Player 1 Field
        p1_field_roi, p1_field_sim = get_similar_roi(field_rois[0], rects)
        if p1_field_sim < 0.99:
            reset[0] = True
        else:
            rois[0] = p1_field_roi
        
        # Find Player 2 Field
        p2_field_roi, p2_field_sim = get_similar_roi(field_rois[1], rects)
        if p2_field_sim < 0.99:
            reset[1] = True
        else:
            rois[1] = p2_field_roi

    p1_data, p2_data = [update(i, frame, reset[i], rois[i]) for i in range(2)]

    green_screen = get_greenscreen(field_rois, p1_data, p2_data)

    if confirmation > 0:
        cv2.imshow('Puyo Chain Detector', cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))
    else:
        cv2.imshow('Puyo Chain Detector', cv2.cvtColor(green_screen, cv2.COLOR_RGB2BGR))

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    elif cv2.getWindowProperty('Puyo Chain Detector', cv2.WND_PROP_VISIBLE) < 1:
        break

cap.release()
cv2.destroyAllWindows()

# %%

