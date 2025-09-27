from flask import Flask, jsonify, request, render_template
import json
import os

app = Flask(__name__)

# File to store the JSON data
json_file_path = 'data.json'

# List of valid modes (matched to ESP8266 code)
VALID_MODES = [
    'off',
    'rainbow-flow',
    'constant-red',
    'proletariat-crackle',
    'soma-haze',
    'loonie-freefall',
    'bokanovsky-burst',
    'total-perspective-vortex',
    'golgafrincham-drift',
    'bistromathics-surge',
    'groks-dissolution',
    'newspeak-shrink',
    'nolite-te-bastardes',
    'infinite-improbability-drive',
    'big-brother-glare',
    'replicant-retirement',
    'water-brother-bond',
    'hypnopaedia-hum',
    'vogon-poetry-pulse',
    'thought-police-flash',
    'electric-sheep-dream',
    'QRNG',
    'sd-client'
]

# Function to read the mode from the JSON file
def read_json():
    if not os.path.exists(json_file_path):
        write_json({"mode": "off"})
        return {"mode": "off"}
    
    try:
        with open(json_file_path, 'r') as f:
            data = json.load(f)
            if 'mode' not in data or data['mode'] not in VALID_MODES:
                data['mode'] = 'off'
                write_json(data)
            return data
    except json.JSONDecodeError:
        print("Error decoding JSON data. Recreating with default mode.")
        write_json({"mode": "off"})
        return {"mode": "off"}

# Function to write the mode to the JSON file
def write_json(data):
    with open(json_file_path, 'w') as f:
        json.dump(data, f)

# Route to serve the current mode as plain text
@app.route('/mode', methods=['GET'])
def get_mode():
    data = read_json()
    return data['mode']

# Route to update the mode in the JSON file
@app.route('/update', methods=['POST'])
def update_data():
    new_mode = request.json.get('mode')
    if new_mode in VALID_MODES:
        write_json({'mode': new_mode})
        return jsonify({'message': 'Mode updated successfully!'}), 200
    else:
        return jsonify({'error': f'Invalid mode. Choose from {VALID_MODES}.'}), 400

# Route to render the HTML page
@app.route('/')
def index():
    data = read_json()
    return render_template('index.html', mode=data['mode'], valid_modes=VALID_MODES)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)