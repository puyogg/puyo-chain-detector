import cv2
from collections import deque
import numpy as np
from roidetection import get_next_rois
from scraper import crop_im
import time
# import matplotlib.pyplot as plt

field_rois = np.array([[136,  79, 200, 361], [624,  79, 199, 361]])
next_rois = get_next_rois(field_rois)

class NextMovementTracker():
    def __init__(self, player, roi, queue_size=2, bw_threshold = 180, move_threshold=0):
        self.top_queue = deque()
        self.bot_queue = deque()
        self.roi = roi
        self.queue_size = queue_size
        self.move_threshold = move_threshold
        self.bw_threshold = bw_threshold

        self.top_roi = np.copy(roi)
        self.bot_roi = np.copy(roi)

        if player == 1:
            self.top_roi[2] = roi[2] * 0.5
            self.top_roi[3] = roi[3] * 0.1
            self.top_roi[0] = roi[0] + roi[2] * 0.08
            self.top_roi[1] = roi[1] + roi[3] * 0.05

            self.bot_roi[2] = roi[2] * 0.5
            self.bot_roi[3] = roi[3] * 0.1
            self.bot_roi[0] = roi[0] + roi[2] * 0.35
            self.bot_roi[1] = roi[1] + roi[3] * 0.52

        elif player == 2:
            self.top_roi[2] = roi[2] * 0.5
            self.top_roi[3] = roi[3] * 0.1
            self.top_roi[0] = roi[0] + roi[2] * 0.92 - roi[2] * 0.5
            self.top_roi[1] = roi[1] + roi[3] * 0.05

            self.bot_roi[2] = roi[2] * 0.5
            self.bot_roi[3] = roi[3] * 0.1
            self.bot_roi[0] = roi[0] + roi[2] * 0.05
            self.bot_roi[1] = roi[1] + roi[3] * 0.52
    
    def is_moving(self, frame):
        gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        gray[gray < self.bw_threshold] = 0
        gray[gray >= self.bw_threshold] = 255

        top_third = crop_im(gray, self.top_roi)
        bottom_third = crop_im(gray, self.bot_roi)
        
        self.top_queue.append(top_third)
        self.bot_queue.append(bottom_third)

        if len(self.top_queue) > self.queue_size:
            self.top_queue.popleft()
            self.bot_queue.popleft()
        
        # top_images = list(self.top_queue)
        # top_images = [image.reshape((image.shape[0], image.shape[1], 1)) for image in top_images]
        # all_images = np.concatenate(top_images, axis=2)
        # median_top = np.median(all_images, axis=2).astype(np.uint8)

        # bot_images = list(self.bot_queue)
        # bot_images = [image.reshape((image.shape[0], image.shape[1], 1)) for image in bot_images]
        # all_images = np.concatenate(bot_images, axis=2)
        # median_bot = np.median(all_images, axis=2).astype(np.uint8)

        # diff_top = top_third - median_top
        # diff_bot = bottom_third - median_bot
        top_images = list(self.top_queue)
        top_image = top_images[0]
        diff_top = np.abs(top_third - top_image)

        bot_images = list(self.bot_queue)
        bot_image = bot_images[0]
        diff_bot = np.abs(bottom_third - bot_image)

        top_is_moving = len(diff_top[diff_top > 0]) > self.move_threshold
        bot_is_moving = len(diff_bot[diff_bot > 0]) > self.move_threshold

        return top_is_moving and bot_is_moving


# if __name__ == '__main__':
#     cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)
#     cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 540)
#     cap.set(cv2.CAP_PROP_FRAME_WIDTH, 960)
#     ret, frame = cap.read()
#     frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

#     queue = deque()
#     queue.append(frame)


#     p1_next = NextMovementTracker(1, next_rois[0])

#     prev = 0
#     prev_frame = frame
#     while True:
#         time_elapsed = time.time() - prev
#         # print(time_elapsed)
#         if time_elapsed <= 1./60:
#             continue
#         prev = time.time()

#         ret, frame = cap.read()
#         frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

#         # Next moved?
#         p1_next_moved = p1_next.is_moving(frame)

#         if p1_next_moved:
#             cv2.imshow('Puyo Chain Detector', cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))
#             prev_frame = frame
#         else:
#             cv2.imshow('Puyo Chain Detector', cv2.cvtColor(prev_frame, cv2.COLOR_RGB2BGR))

#         if cv2.waitKey(1) & 0xFF == ord('q'):
#             break
#         elif cv2.getWindowProperty('Puyo Chain Detector', cv2.WND_PROP_VISIBLE) < 1:
#             break