from PIL import Image, ImageTk
import tkinter as tk
from tkinter import messagebox
import cv2
import numpy as np
import json

## Helper
def check_cameras(count = 10, capture_api = cv2.CAP_ANY):
    devices = []
    for i in range(count):
        cap = cv2.VideoCapture(i, capture_api)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 270)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 480)
        ret, frame = cap.read()
        devices.append((i, ret, frame))

    # # Return valid devices
    # valid = [(i, ret, frame) for i, ret, frame in devices if ret == True]
    return devices

## Main Window
window = tk.Tk()
window.wm_title("Puyo Chain Detector Settings")

## Variables
capture_modes = ["None", "DirectShow"]
current_mode = tk.StringVar(window)
current_mode.set(capture_modes[0])

device_ids = []
device_ss = []
current_device = tk.IntVar(window)
current_device.set(2424)

np_img = np.ones((270, 480, 3), dtype=np.uint8) * 255
img = ImageTk.PhotoImage(Image.fromarray(np_img))

## Mode map
mode_map = {
    'None': cv2.CAP_ANY,
    'DirectShow': cv2.CAP_DSHOW
}

## Callbacks
def reset_devices():
    global device_ids
    global device_ss

    device_ids = []
    device_ss = []
    opt_device["menu"].delete(0, 'end')
    current_device.set(2424)

def call_check_cameras(*args):
    global device_ids
    global device_ss

    mode = current_mode.get()

    if mode == "None":
        print('No mode selected.')
        reset_devices()
    elif mode == "DirectShow":
        print('DirectShow Capture selected')
        camera_data = check_cameras(capture_api=cv2.CAP_DSHOW)
        device_ids = [i for i, ret, frame in camera_data]
        device_ss = [frame for i, ret, frame in camera_data]

        valid_ids = [i for i, ret, frame in camera_data if ret == True]

        # Reset device option menu
        opt_device["menu"].delete(0, 'end')
        for id in valid_ids:
            opt_device["menu"].add_command(label=id, command=tk._setit(current_device, id))
        print('Finished checking DirectShow devices.')

def call_show_screenshot(*args):
    idx = current_device.get()
    if idx != 2424:
        rgb_im = cv2.cvtColor(device_ss[idx], cv2.COLOR_BGR2RGB)
        rgb_im = ImageTk.PhotoImage(Image.fromarray(rgb_im))
        panel.configure(image=rgb_im)
        panel.image = rgb_im

def call_save_settings(*args):
    idx = current_device.get()
    mode = mode_map[current_mode.get()]

    if mode == "None":
        messagebox.showinfo("Error", "Invalid capture mode. Select one fromt he drop down boxes.")
    if idx == 2424:
        messagebox.showinfo("Error", "Invalid device id. Select one from the drop down boxes.")
    if idx == 2424 or mode == "None": return

    data = {
        'mode': mode,
        'device_id': idx
    }

    json.dump(data, open('settings.json', 'w'))
    messagebox.showinfo("Success", "Saved the settings to settings.json. Close this window and run the main program.")
    

## Bind callbacks
current_mode.trace("w", call_check_cameras)
current_device.trace("w", call_show_screenshot)

## Select Capture mode
txt_mode = tk.Label(text='Select Capture Mode')
txt_mode.pack()

opt_capture = tk.OptionMenu(window, current_mode, *capture_modes)
opt_capture.pack()

## Select Capture Device
txt_device = tk.Label(text='Select Video Capture Device')
txt_device.pack()

opt_device = tk.OptionMenu(window, current_device, None)
opt_device.pack()

## Select sample screenshot
txt_screen = tk.Label(text='Sample screenshot of selected device:')
txt_screen.pack()

panel = tk.Label(window, image=img)
panel.pack()

## Save settings
btn_save = tk.Button(text="Save settings")
btn_save.bind("<Button-1>", call_save_settings)
btn_save.pack()

## Initiate loop
window.mainloop()