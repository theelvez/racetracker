from flask import Flask, render_template, request
import sqlite3

app = Flask(__name__)

# Connect to the SQLite3 database
conn = sqlite3.connect('/path/to/your/database.db')
c = conn.cursor()

# Create the 'gps_coordinates' table in the database
c.execute('''CREATE TABLE IF NOT EXISTS gps_coordinates
             (id INTEGER PRIMARY KEY, location TEXT, latitude REAL, longitude REAL)''')

# Route for the web app homepage
@app.route('/')
def index():
    return render_template('index.html')

# Route for storing GPS coordinates for the start line
@app.route('/store_start_line', methods=['POST'])
def store_start_line():
    location = 'start line'
    latitude = request.form['latitude']
    longitude = request.form['longitude']
    c.execute("INSERT INTO gps_coordinates (location, latitude, longitude) VALUES (?, ?, ?)", (location, latitude, longitude))
    conn.commit()
    return 'Start line GPS coordinates stored in the database'

# Route for storing GPS coordinates for the finish line
@app.route('/store_finish_line', methods=['POST'])
def store_finish_line():
    location = 'finish line'
    latitude = request.form['latitude']
    longitude = request.form['longitude']
    c.execute("INSERT INTO gps_coordinates (location, latitude, longitude) VALUES (?, ?, ?)", (location, latitude, longitude))
    conn.commit()
    return 'Finish line GPS coordinates stored in the database'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
