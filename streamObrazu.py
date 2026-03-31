import cv2
import threading
import time

class CameraStream:
    # Default resolution values as class constants for easy modification
    DEFAULT_WIDTH = 640
    DEFAULT_HEIGHT = 480
    
    def __init__(self, src=0, width=DEFAULT_WIDTH, height=DEFAULT_HEIGHT):
        self.cap = cv2.VideoCapture(src)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

        self.frame = None
        self.stopped = False
        self.lock = threading.Lock()  # Thread-safe frame access

        if self.cap.isOpened():
            # Daemon thread automatically terminates with main program
            self.thread = threading.Thread(target=self.update, daemon=True)
            self.thread.start()
        else:
            print("Błąd: Nie można otworzyć kamery.")

    def update(self):
        # Simplified loop - removed unnecessary redundant checks
        while not self.stopped:
            grabbed, frame = self.cap.read()
            if not grabbed:
                self.stop()
                break
            
            # Thread-safe frame storage
            with self.lock:
                self.frame = frame
            time.sleep(0.001)

        self.cap.release()

    def read(self):
        # Thread-safe read operation
        with self.lock:
            return self.frame

    def stop(self):
        self.stopped = True

    def is_opened(self):
        return self.cap.isOpened() and not self.stopped  # Check both open AND running
