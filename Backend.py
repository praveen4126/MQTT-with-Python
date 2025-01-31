from flask import Flask, render_template, jsonify
import paho.mqtt.client as mqtt
import threading
import time

# MQTT Broker Details
MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883
WATER_LEVEL_TOPIC = "water_level"
LED_CONTROL_TOPIC = "green_led_control"

# Water tank simulation
water_level = 50  # Initial water level (percentage)
led_status = "OFF"

app = Flask(__name__)

# MQTT Client
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe(WATER_LEVEL_TOPIC)

def on_message(client, userdata, msg):
    global water_level, led_status

    if msg.topic == WATER_LEVEL_TOPIC:
        try:
            water_level = float(msg.payload.decode())  # Convert received level
        except ValueError:
            print("Invalid water level received")

# Connect MQTT
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

def mqtt_loop():
    client.loop_forever()

# Start MQTT in a separate thread
mqtt_thread = threading.Thread(target=mqtt_loop)
mqtt_thread.start()

# Flask Routes
@app.route('/')
def index():
    return render_template("dashboard.html")

@app.route('/status', methods=['GET'])
def status():
    return jsonify({"water_level": water_level, "led_status": led_status})

@app.route('/control_led/<state>', methods=['POST'])
def control_led(state):
    global led_status
    if state.upper() in ["ON", "OFF"]:
        led_status = state.upper()
        client.publish(LED_CONTROL_TOPIC, led_status)
        return jsonify({"success": True, "led_status": led_status})
    return jsonify({"success": False, "error": "Invalid state"})

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0")