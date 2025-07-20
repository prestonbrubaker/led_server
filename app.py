from flask import Flask, jsonify, request, render_template
import json
import os

app = Flask(__name__)

# File to store the JSON data
json_file_path = 'data.json'

# List of valid modes (matched to ESP8266 code, including 'top_species_glow')
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
    'top_species_glow'  # Retained to match existing functionality
]

# Function to read the JSON data
def read_json():
    if not os.path.exists(json_file_path):
        default_data = {"mode": "off", "top_species_colors": []}
        write_json(default_data)
        return default_data
    
    try:
        with open(json_file_path, 'r') as f:
            data = json.load(f)
            if 'mode' not in data or data['mode'] not in VALID_MODES:
                data['mode'] = 'off'
            if 'top_species_colors' not in data:
                data['top_species_colors'] = []
            write_json(data)
            return data
    except json.JSONDecodeError:
        print("Error decoding JSON data. Recreating with default values.")
        default_data = {"mode": "off", "top_species_colors": []}
        write_json(default_data)
        return default_data

# Function to write the JSON data
def write_json(data):
    with open(json_file_path, 'w') as f:
        json.dump(data, f, indent=2)

# Route to serve the current mode as plain text
@app.route('/mode', methods=['GET'])
def get_mode():
    data = read_json()
    return data['mode']

# Route to serve the top species colors
@app.route('/top_species', methods=['GET'])
def get_top_species():
    data = read_json()
    return jsonify({'colors': data['top_species_colors']})

# Route to update the mode or top species colors
@app.route('/update', methods=['POST'])
def update_data():
    data = read_json()
    request_data = request.get_json()
    
    if not request_data:
        return jsonify({'error': 'No data provided'}), 400

    # Update mode if provided
    if 'mode' in request_data:
        new_mode = request_data['mode']
        if new_mode in VALID_MODES:
            data['mode'] = new_mode
        else:
            return jsonify({'error': f'Invalid mode. Choose from {VALID_MODES}.'}), 400

    # Update top species colors if provided
    if 'top_species_colors' in request_data:
        colors = request_data['top_species_colors']
        # Validate colors format
        if isinstance(colors, list) and len(colors) <= 5:
            for color in colors:
                if not (isinstance(color, dict) and all(key in color for key in ['r', 'g', 'b']) and
                        all(isinstance(color[key], int) and 0 <= color[key] <= 255 for key in ['r', 'g', 'b'])):
                    return jsonify({'error': 'Invalid color format. Must be list of up to 5 {r, g, b} objects with values 0-255.'}), 400
            data['top_species_colors'] = colors

    write_json(data)
    return jsonify({'message': 'Data updated successfully!', 'data': data}), 200

# Route to render the HTML page
@app.route('/')
def index():
    data = read_json()
    return render_template('index.html', mode=data['mode'], valid_modes=VALID_MODES)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
