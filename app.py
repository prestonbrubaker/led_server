from flask import Flask, render_template, jsonify, request
import json
import os

app = Flask(__name__)

# File to store the JSON data
json_file_path = 'data.json'

# Function to read the number from the JSON file
def read_json():
    if not os.path.exists(json_file_path):
        # If the file doesn't exist, create it with the default value
        write_json({"number": 1})
        return {"number": 1}
    
    try:
        with open(json_file_path, 'r') as f:
            data = json.load(f)
            # Ensure 'number' key exists in the data
            if 'number' not in data:
                data['number'] = 1  # Default to 1 if key is missing
                write_json(data)  # Update the file with default data
            return data
    except json.JSONDecodeError:
        # Handle case where the JSON is invalid
        print("Error decoding JSON data. Recreating the file with default values.")
        write_json({"number": 1})
        return {"number": 1}

# Function to write the number to the JSON file
def write_json(data):
    with open(json_file_path, 'w') as f:
        json.dump(data, f)

# Route to serve the current JSON data
@app.route('/data', methods=['GET'])
def get_data():
    data = read_json()
    return jsonify(data)

# Route to update the number in the JSON file
@app.route('/update', methods=['POST'])
def update_data():
    new_number = request.json.get('number')
    if new_number in [1, 2, 3]:
        data = {'number': new_number}
        write_json(data)
        return jsonify({'message': 'Number updated successfully!'}), 200
    else:
        return jsonify({'error': 'Invalid number. Choose from 1, 2, or 3.'}), 400

# Route to render the HTML page
@app.route('/')
def index():
    data = read_json()
    return render_template('index.html', number=data['number'])

if __name__ == '__main__':
    # Run the Flask server on HTTP (No SSL)
    app.run(debug=True, host='0.0.0.0', port=5000)  # This is HTTP, no SSL specified
