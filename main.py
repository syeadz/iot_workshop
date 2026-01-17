from flask import Flask, request, jsonify, render_template_string
from datetime import datetime

app = Flask(__name__)

# Devices store
devices = {}

# Preset thresholds
PRESET_THRESHOLDS = [10, 30, 50, 70]

# --- API endpoint for ESP32 to update distance ---
@app.route('/api/update', methods=['POST'])
def update_device():
    data = request.json
    device_id = data.get('id')
    distance = data.get('distance')

    if device_id not in devices:
        devices[device_id] = {
            "distance": 0,
            "threshold": 50.0,  # default
            "time": None
        }

    devices[device_id]["distance"] = distance
    devices[device_id]["time"] = datetime.now()

    # Send current threshold back to ESP32
    return jsonify({
        "threshold": devices[device_id]["threshold"]
    })

# --- API endpoint to fetch all device data as JSON ---
@app.route('/api/devices')
def get_devices():
    formatted_devices = {}
    for dev_id, info in devices.items():
        formatted_devices[dev_id] = {
            "distance": info["distance"],
            "threshold": info["threshold"],
            "time": info["time"].strftime('%H:%M:%S') if info["time"] else None
        }
    return jsonify(formatted_devices)

# --- Set threshold via radio button ---
@app.route('/set_threshold/<device_id>', methods=['POST'])
def set_threshold(device_id):
    if device_id in devices:
        selected_threshold = request.form.get("threshold")
        if selected_threshold:
            devices[device_id]["threshold"] = float(selected_threshold)
    return '<script>window.location.href="/"</script>'

# --- Dashboard ---
@app.route('/')
def dashboard():
    html = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Ultrasonic IoT Dashboard</title>
    </head>
    <body>
        <h1>Ultrasonic IoT Dashboard</h1>
        <table border="1" cellpadding="10">
          <tr>
            <th>Device ID</th>
            <th>Distance (cm)</th>
            <th>Threshold (cm)</th>
            <th>Update Threshold</th>
            <th>Last Update</th>
          </tr>
          {% for dev_id, info in devices.items() %}
          <tr>
            <td>{{ dev_id }}</td>
            <td id="{{ dev_id }}_distance">{{ info.distance }}</td>
            <td id="{{ dev_id }}_threshold">{{ info.threshold }}</td>
            <td>
              <form action="/set_threshold/{{ dev_id }}" method="post">
                {% for val in preset_thresholds %}
                  <label>
                    <input type="radio" name="threshold" value="{{ val }}" {% if val == info.threshold %}checked{% endif %}>
                    {{ val }}
                  </label>
                {% endfor %}
                <button type="submit">Set</button>
              </form>
            </td>
            <td id="{{ dev_id }}_time">{{ info.time.strftime('%H:%M:%S') if info.time else 'Never' }}</td>
          </tr>
          {% endfor %}
        </table>

        <script>
        async function refreshData() {
            try {
                const response = await fetch('/api/devices');
                const data = await response.json();

                for (const dev_id in data) {
                    document.getElementById(dev_id + '_distance').textContent = data[dev_id].distance;
                    document.getElementById(dev_id + '_threshold').textContent = data[dev_id].threshold;
                    document.getElementById(dev_id + '_time').textContent = data[dev_id].time || 'Never';
                }
            } catch (e) {
                console.error("Error fetching device data:", e);
            }
        }

        // Refresh every 0.5 seconds (500 ms)
        setInterval(refreshData, 500);
        </script>
    </body>
    </html>
    """
    return render_template_string(html, devices=devices, preset_thresholds=PRESET_THRESHOLDS)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
