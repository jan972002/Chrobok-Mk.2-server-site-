from flask import Flask, Response, render_template, request, jsonify
import obrobkaObrazu as OObraz
from streamObrazu import CameraStream
import time
import threading
import cv2
import numpy as np
import serial
import serial.tools.list_ports
from collections import deque

app = Flask(__name__)

SERIAL_PORT_NAME = 'COM5'
BAUD_RATE = 115200

serial_received_queue = deque(maxlen=20)

LOGIC_COMMAND_MAP = {
    "GripperDelay": "opoznienieChwytaka",
    "ServoDelay": "opoznienieRuchuServa"
}


class SerialController:
    def __init__(self, port, baud_rate):
        self.port = port
        self.baud_rate = baud_rate
        self.ser = None
        self.lock = threading.Lock()
        self.is_running = False
        self.read_thread = None
        self.connect()

    def connect(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
        try:
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=0.1)
            time.sleep(2)
            print(f"📡 Pomyślnie połączono z portem szeregowym: {self.port} @ {self.baud_rate}")
            self.start_reading()
        except serial.SerialException as e:
            print(f"Błąd połączenia z portem {self.port}: {e}")
            self.ser = None

    def start_reading(self):
        if self.ser and self.ser.is_open and not self.is_running:
            self.is_running = True
            self.read_thread = threading.Thread(target=self._read_serial_data, daemon=True)
            self.read_thread.start()
            print("Wątek nasłuchujący serial portu uruchomiony.")

    def _read_serial_data(self):
        while self.is_running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        timestamp = time.strftime("%H:%M:%S")
                        message = f"[{timestamp}] OTRZYMANO: {line}"
                        serial_received_queue.append(message)
                        print(message)
            except serial.SerialTimeoutException:
                pass
            except Exception as e:
                print(f"Błąd odczytu serial: {e}")
                self.is_running = False
                break
            time.sleep(0.01)

    def send_command(self, command):
        if not self.ser or not self.ser.is_open:
            print(f"⚠️ Nie można wysłać. Port szeregowy {self.port} jest nieotwarty lub niepołączony.")
            self.connect()
            if not self.ser or not self.ser.is_open:
                return False

        with self.lock:
            try:
                full_command = (command + '\n').encode('utf-8')
                self.ser.write(full_command)
                print(f"WYSLANO SERIAL: {command}")
                return True
            except Exception as e:
                print(f"Błąd zapisu na port {self.port}: {e}")
                self.ser.close()
                self.ser = None
                self.is_running = False
                return False

    def close(self):
        self.is_running = False
        if self.read_thread and self.read_thread.is_alive():
            self.read_thread.join(timeout=1)
        if self.ser and self.ser.is_open:
            self.ser.close()
            print(f"Port szeregowy {self.port} zamknięty.")


serial_control = SerialController(SERIAL_PORT_NAME, BAUD_RATE)


is_streaming = False
stream_lock = threading.Lock()
camera = CameraStream(src=0)

servo_values = {
    "Servo1": 500, "Servo2": 75, "Servo3": 30, "Servo4": 180, "Servo5": 90
}
logic_parameters = {
    "GripperDelay": 10, "ServoDelay": 10
}


def generate_frames():
    if not camera.is_opened():
        return

    while True:
        frame = camera.read()

        if frame is None:
            time.sleep(0.05)
            continue

        frame_bytes = b''

        with stream_lock:
            should_process = is_streaming

        if should_process:
            frame_bytes = OObraz.OpracowaneKlatki(frame)
        else:
            black_frame = np.zeros(frame.shape, dtype=np.uint8)
            if black_frame.size > 0:
                cv2.putText(black_frame, "Stream zatrzymany", (170, 240), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
            ret, buffer = cv2.imencode('.jpg', black_frame)
            frame_bytes = buffer.tobytes()
            time.sleep(0.5)

        if frame_bytes:
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')


@app.route('/update_servo', methods=['POST'])
def update_servo():
    data = request.get_json()
    servo_name = data.get('servo')
    value = data.get('value')

    if servo_name in servo_values and value is not None:
        try:
            value = int(value)
            servo_values[servo_name] = value

            servo_number = servo_name.replace("Servo", "")
            serial_command = f"M{servo_number}:{value}"  # Format: M#:kąt

            serial_control.send_command(serial_command)

            return jsonify({"status": "success", "servo": servo_name, "value": value}), 200
        except ValueError:
            return jsonify({"status": "error", "message": "Nieprawidłowa wartość"}), 400

    return jsonify({"status": "error", "message": "Nieznane serwo lub brak wartości"}), 400


@app.route('/update_logic_param', methods=['POST'])
def update_logic_param():
    data = request.get_json()
    param_name = data.get('setting')
    value = data.get('value')

    if param_name in logic_parameters and value is not None:
        try:
            value = int(value)
            logic_parameters[param_name] = value
            if param_name in LOGIC_COMMAND_MAP:
                command_prefix = LOGIC_COMMAND_MAP[param_name]
                serial_command = f"{command_prefix}:{value}"
                serial_control.send_command(serial_command)
            print(f"PARAMETR ZAKTUALIZOWANY: {param_name} na wartość: {value}")
            return jsonify({"status": "success", "param": param_name, "value": value}), 200
        except ValueError:
            return jsonify({"status": "error", "message": "Nieprawidłowa wartość"}), 400

    return jsonify({"status": "error", "message": "Nieznany parametr lub brak wartości"}), 400


@app.route('/set_initial_position', methods=['POST'])
def set_initial_position():
    print("WYWOŁANO: Ustawienie pozycji początkowej ramienia!")
    serial_control.send_command("ZerowaPozycja")
    initial_servo_values = {
        "Servo1": 500, "Servo2": 75, "Servo3": 30, "Servo4": 180, "Servo5": 90
    }
    global servo_values
    servo_values.update(initial_servo_values)
    return jsonify({
        "status": "success",
        "message": "Wysłano komendę ustawienia pozycji początkowej i zresetowano UI.",
        "initial_values": initial_servo_values
    }), 200

@app.route('/get_serial_log', methods=['GET'])
def get_serial_log():
    log_messages = list(serial_received_queue)
    serial_received_queue.clear()
    return jsonify({"log": log_messages}), 200


@app.route('/start_stream')
def start_stream():
    global is_streaming
    with stream_lock:
        is_streaming = True
    return "Streaming started", 200


@app.route('/stop_stream')
def stop_stream():
    global is_streaming
    with stream_lock:
        is_streaming = False
    return "Streaming stopped", 200


@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/')
def index():
    return render_template('strona_ramie.html')


if __name__ == '__main__':
    try:
        app.run(host='0.0.0.0', port=5000, threaded=True)
    except KeyboardInterrupt:
        pass
    finally:
        camera.stop()
        serial_control.close()