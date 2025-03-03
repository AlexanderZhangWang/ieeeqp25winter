from flask import Flask, request, jsonify
from flask_cors import CORS  # Import Flask-CORS
import sqlite3
import os

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

DATABASE = 'sensor_data.db'

def init_db():
    if not os.path.exists(DATABASE):
        conn = sqlite3.connect(DATABASE)
        c = conn.cursor()
        c.execute('''
            CREATE TABLE sensor_data (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                analog_value REAL,
                voltage REAL,
                tds_value REAL,
                turbidity INTEGER,
                gps_lat TEXT,
                gps_long TEXT,
                gps_date TEXT,
                gps_time TEXT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        conn.commit()
        conn.close()
        print("Database initialized and table created.")

def insert_data(analog_value, voltage, tds_value, turbidity, gps_lat, gps_long, gps_date, gps_time):
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()
    c.execute('''
        INSERT INTO sensor_data (analog_value, voltage, tds_value, turbidity, gps_lat, gps_long, gps_date, gps_time)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    ''', (analog_value, voltage, tds_value, turbidity, gps_lat, gps_long, gps_date, gps_time))
    conn.commit()
    conn.close()
    print("Data inserted into the database.")

@app.route('/store_data', methods=['POST'])
def store_data_route():
    analog_value = request.form.get('analogValue')
    voltage = request.form.get('voltage')
    tds_value = request.form.get('tdsValue')
    turbidity = request.form.get('turbidity')
    gps_data = request.form.get('gpsData')

    print("Received Data:")
    print(f"Analog Value: {analog_value}")
    print(f"Voltage: {voltage}")
    print(f"TDS Value: {tds_value}")
    print(f"Turbidity: {turbidity}")
    print(f"GPS Data: {gps_data}")

    try:
        analog_value = float(analog_value)
    except (ValueError, TypeError):
        analog_value = None
    try:
        voltage = float(voltage)
    except (ValueError, TypeError):
        voltage = None
    try:
        tds_value = float(tds_value)
    except (ValueError, TypeError):
        tds_value = None
    try:
        turbidity = int(turbidity)
    except (ValueError, TypeError):
        turbidity = None

    # Initialize GPS variables as None (to be stored as NULL in the DB)
    gps_lat = None
    gps_long = None
    gps_date = None
    gps_time = None

    # Only parse if gps_data is provided and not just empty whitespace
    if gps_data and gps_data.strip() != "":
        parts = gps_data.split(',')
        for part in parts:
            if ':' in part:
                key, value = part.split(':', 1)
                key = key.strip().lower()
                value = value.strip()
                if key == 'lat':
                    gps_lat = value
                elif key == 'long':
                    gps_long = value
                elif key == 'date':
                    gps_date = value
                elif key == 'time':
                    gps_time = value

    # Insert the data (None values will be stored as NULL in SQLite)
    insert_data(analog_value, voltage, tds_value, turbidity, gps_lat, gps_long, gps_date, gps_time)
    return jsonify({'status': 'success', 'message': 'Data received and stored!'}), 200


@app.route('/get_data', methods=['GET'])
def get_data():
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()
    c.execute('SELECT gps_lat, gps_long, analog_value, voltage, tds_value, turbidity, gps_date, gps_time FROM sensor_data')
    rows = c.fetchall()
    conn.close()
    data = []
    for row in rows:
        data.append({
            "gps_lat": row[0],
            "gps_long": row[1],
            "analog_value": row[2],
            "voltage": row[3],
            "tds_value": row[4],
            "turbidity": row[5],
            "gps_date": row[6],
            "gps_time": row[7]
        })
    return jsonify(data)

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000)
