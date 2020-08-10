import cv2
import numpy as np
from roidetection import get_score_rois
from scraper import crop_im
from PIL import Image
import time

field_rois = np.array([[136,  79, 200, 361], [624,  79, 199, 361]])
score_rois = get_score_rois(field_rois)

crop_x = np.array(Image.open('score-x-template.png').convert('L'), dtype=np.uint8)

class ScoreXTracker():
    def __init__(self, roi, bw_threshold = 200, match_threshold = 0.8):
        self.roi = roi
        self.bw_threshold = bw_threshold
        self.match_threshold = match_threshold
    
    def has_x(self, frame):
        score = crop_im(frame, self.roi)
        
        gray = cv2.cvtColor(score, cv2.COLOR_RGB2GRAY)

        thresh = np.zeros((score.shape[0], score.shape[1]), dtype=np.uint8)
        thresh[gray > 200] = 255

        res = cv2.matchTemplate(thresh, crop_x, cv2.TM_CCOEFF_NORMED)
        loc = np.where(res >= self.match_threshold)

        return len(loc[0]) > 0

if __name__ == '__main__':
    cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 540)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 960)
    ret, frame = cap.read()
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    prev = 0
    prev_frame = frame

    p1_score = ScoreXTracker(score_rois[0])
    
    while True:
        time_elapsed = time.time() - prev
        if time_elapsed <= 1./60:
            continue

        ret, frame = cap.read()
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # Next moved?
        has_x = p1_score.has_x(frame)

        if has_x:
            cv2.imshow('Puyo Chain Detector', cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))
            prev_frame = frame
        else:
            cv2.imshow('Puyo Chain Detector', cv2.cvtColor(prev_frame, cv2.COLOR_RGB2BGR))

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        elif cv2.getWindowProperty('Puyo Chain Detector', cv2.WND_PROP_VISIBLE) < 1:
            break