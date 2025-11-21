import cv2
from ultralytics import YOLO
import torch
import numpy as np


print(f"CUDA dostępna: {torch.cuda.is_available()}")
print(f"Nazwa urządzenia: {torch.cuda.get_device_name(0)}")

polskie_nazwy_klas = {
    0: 'OSOBA', 1: 'ROWER', 2: 'SAMOCHOD', 3: 'MOTOCYKL', 4: 'SAMOLOT',
    5: 'AUTOBUS', 6: 'POCIAG', 7: 'CIEZAROWKA', 8: 'LODZ', 9: 'SWIATLA_DROGOWE',
    10: 'HYDRANT', 11: 'ZNAK_STOPU', 12: 'PARKOMETR', 13: 'LAWKA', 14: 'PTAK',
    15: 'KOT', 16: 'PIES', 17: 'KON', 18: 'OWCA', 19: 'KROWA',
    20: 'SLON', 21: 'NIEDZWIEDZ', 22: 'ZEBRA', 23: 'ZYRAFA', 24: 'PLECAK',
    25: 'PARASOL', 26: 'TOREBKA', 27: 'KRAWAT', 28: 'WALIZKA', 29: 'FRISBEE',
    30: 'NARTY', 31: 'SNOWBOARD', 32: 'PILKA_SPORTOWA', 33: 'LATAWIEC', 34: 'KIJ_BEJSBOLOWY',
    35: 'REKAWICA_BEJSBOLOWA', 36: 'DESKOROLKA', 37: 'DESKA_SURFINGOWA', 38: 'RAKIETA_TENISOWA', 39: 'BUTELKA',
    40: 'KIELISZEK_WINA', 41: 'KUBEK', 42: 'WIDELEC', 43: 'NOZ', 44: 'LYZKA',
    45: 'MISKA', 46: 'BANAN', 47: 'JABLKO', 48: 'KANAPKA', 49: 'POMARANCZA',
    50: 'BROKUL', 51: 'MARCHEWKA', 52: 'HOT_DOG', 53: 'PIZZA', 54: 'PACZEK',
    55: 'CIASTO', 56: 'KRZESLO', 57: 'KANAPA', 58: 'ROSLINA_DONICZKOWA', 59: 'LOZKO',
    60: 'STOL_JADALNIANY', 61: 'TOALETA', 62: 'TELEWIZOR', 63: 'LAPTOP', 64: 'MYSZ_KOMPUTEROWA',
    65: 'PILOT', 66: 'KLAWIATURA', 67: 'TELEFON_KOMORKOWY', 68: 'MIKROFALOWKA', 69: 'PIEKARNIK',
    70: 'TOSTER', 71: 'ZLEW', 72: 'LODOWKA', 73: 'KSIAZKA', 74: 'ZEGAR',
    75: 'WAZON', 76: 'NOZYCZKI', 77: 'PLUSZOWY_MIS', 78: 'SUSZARKA_DO_WLOSOW', 79: 'SZCZOTECZKA_DO_ZEBOW'
}
akcelerator = '0'
nazwaOkna = 'YOLO detekcja'
model = YOLO('yolov8s.pt')

#print(model.names)
#{0: 'person', 1: 'bicycle', 2: 'car', 3: 'motorcycle', 4: 'airplane', 5: 'bus', 6: 'train', 7: 'truck', 8: 'boat', 9: 'traffic light', 10: 'fire hydrant', 11: 'stop sign', 12: 'parking meter', 13: 'bench', 14: 'bird', 15: 'cat', 16: 'dog', 17: 'horse', 18: 'sheep', 19: 'cow', 20: 'elephant', 21: 'bear', 22: 'zebra', 23: 'giraffe', 24: 'backpack', 25: 'umbrella', 26: 'handbag', 27: 'tie', 28: 'suitcase', 29: 'frisbee', 30: 'skis', 31: 'snowboard', 32: 'sports ball', 33: 'kite', 34: 'baseball bat', 35: 'baseball glove', 36: 'skateboard', 37: 'surfboard', 38: 'tennis racket', 39: 'bottle', 40: 'wine glass', 41: 'cup', 42: 'fork', 43: 'knife', 44: 'spoon', 45: 'bowl', 46: 'banana', 47: 'apple', 48: 'sandwich', 49: 'orange', 50: 'broccoli', 51: 'carrot', 52: 'hot dog', 53: 'pizza', 54: 'donut', 55: 'cake', 56: 'chair', 57: 'couch', 58: 'potted plant', 59: 'bed', 60: 'dining table', 61: 'toilet', 62: 'tv', 63: 'laptop', 64: 'mouse', 65: 'remote', 66: 'keyboard', 67: 'cell phone', 68: 'microwave', 69: 'oven', 70: 'toaster', 71: 'sink', 72: 'refrigerator', 73: 'book', 74: 'clock', 75: 'vase', 76: 'scissors', 77: 'teddy bear', 78: 'hair drier', 79: 'toothbrush'}

#target_class_ids = [0]
#model.names.update(polskie_nazwy_klas)


def OpracowaneKlatki(frame):
    frame = cv2.flip(frame, 1)
    results = model(frame, conf=0.5, device=akcelerator) #classes=target_class_ids

    if results:
        r = results[0]
        annotated_frame = r.plot()
        if 'inference' in r.speed:
            fps_value = 1000 / r.speed['inference']
            fps_text = f"FPS: {fps_value:.2f}"
        else:
            fps_text = "FPS: N/A"

        cv2.putText(annotated_frame, fps_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        ret, buffer = cv2.imencode('.jpg', annotated_frame)

        return buffer.tobytes()

    return frame


if __name__ == "__main__":

    nazwaOkna = 'Detekcja YOLO'
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        print("Nie mozna otworzyc kamery!")
    else:
        while True:
            ret, frame = cap.read()

            if not ret:
                break

            processed_bytes = OpracowaneKlatki(frame)

            if processed_bytes:
                nparr = np.frombuffer(processed_bytes, np.uint8)
                processed_frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                cv2.imshow(nazwaOkna, processed_frame)

            if cv2.waitKey(1) & 0xFF == 27:
                break
            if cv2.getWindowProperty(nazwaOkna, cv2.WND_PROP_VISIBLE) < 1:
                break

        cap.release()
        cv2.destroyAllWindows()