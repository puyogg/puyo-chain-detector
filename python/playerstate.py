# %% Imports
import numpy as np
from roidetection import get_screen_rects, get_similar_roi
from permutechain import get_fields_to_try, get_all_chain_lengths, remove_drops
from scraper import crop_im, crop_centered_field, get_field_chain
from windowmotion import NextMovementTracker
from scorex import ScoreXTracker
import cv2

# %%
class PlayerState():
    def __init__(self, player, field_roi, next_roi, score_roi):
        self.player = player
        self.field_roi = field_roi
        self.next_roi = next_roi
        self.score_roi = score_roi

        # Chain lengths and colors to draw
        self.try_inds = []
        self.try_colors = []
        self.chain_lengths = []

        # Score X Tracker
        # When an X is detected in the score region, that implies a pop
        # is occurring. The Chain Detector doesn't need to run during pops.
        self.x_tracker = ScoreXTracker(score_roi)

        # Next Window Tracker
        # When next movement returns true, PlayerState will use
        # cooldown and max_cooldown to avoid analyzing it again for a while
        self.next_tracker = NextMovementTracker(player, next_roi)
        self.cooldown = 0
        self.max_cooldown = 2 # Frames
        self.pop_cooldown = 1

    def tick(self):
        self.cooldown = max(self.cooldown - 1, 0)

    def reset(self):
        self.cooldown = self.pop_cooldown
        self.try_inds = []
        self.try_colors = []
        self.chain_lengths = []

    def update(self, frame: np.ndarray, reset: bool, new_field_roi: np.ndarray):
        '''
        Arguments:
            - frame (np.ndarray): RGB image of the full game screen
        '''
        # Decrement/Increment any timers
        self.tick()

        # Process earlier check in the main loop for whether the screen faded to black
        if reset:
            self.reset()
            return self.try_inds, self.try_colors, self.chain_lengths

        # Check if Puyos are currently popping (examine the score roi)
        is_popping = self.x_tracker.has_x(frame)
        if is_popping:
            self.reset()
            return self.try_inds, self.try_colors, self.chain_lengths

        # Check if the next piece is coming out.
        next_moved = self.next_tracker.is_moving(frame)

        # If the next piece is coming out, run the analyzer
        # if next_moved and self.cooldown == 0:
        if next_moved:
            # print(self.player, 'next moved', next_moved)
            self.cooldown = self.max_cooldown

            # Update try_inds, try_colors, chain_lengths
            self.analyze_field(frame, new_field_roi)
            # print(self.try_inds, self.try_colors, self.chain_lengths)
        
        return self.try_inds, self.try_colors, self.chain_lengths 
        
    def analyze_field(self, frame: np.ndarray, new_field_roi: np.ndarray):
        field_im = crop_im(frame, new_field_roi)
        field_chain = get_field_chain(field_im)
        
        # Remove any floating Puyos
        field_chain = remove_drops(field_chain)
        fields_to_try, try_inds, try_colors = get_fields_to_try(field_chain)
        lengths = get_all_chain_lengths(fields_to_try)

        self.try_inds = try_inds
        self.try_colors = try_colors
        self.chain_lengths = lengths