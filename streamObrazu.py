import cv2
import threading
import time
import numpy as np


class CameraStream:
    def __init__(self, src=0, width=640, height=480):
        self.cap = cv2.VideoCapture(src)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

        self.frame = None
        self.stopped = False

        if self.cap.isOpened():
            self.thread = threading.Thread(target=self.update, args=())
            self.thread.daemon = True
            self.thread.start()
        else:
            print("Błąd: Nie można otworzyć kamery.")

    def update(self):
        while True:
            if self.stopped:
                break
            (grabbed, frame) = self.cap.read()
            if not grabbed:
                self.stop()
                break

            self.frame = frame
            time.sleep(0.001)

        self.cap.release()

    def read(self):
        return self.frame

    def stop(self):
        self.stopped = True

    def is_opened(self):
        return self.cap.isOpened()