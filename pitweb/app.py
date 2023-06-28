from flask import Flask
import sqlite3
from flask import render_template
from flask import request

app = Flask(__name__)

# Database Initialization
conn = sqlite3.connect('racing.db')
c = conn.cursor()

# Create a table for Race Data
c.execute('''CREATE TABLE IF NOT EXISTS race_data
             (id INTEGER PRIMARY KEY AUTOINCREMENT,
              time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
              lat FLOAT,
              lng FLOAT,
              groundspeed FLOAT)''')

# Save the changes and close the connection
conn.commit()
conn.close()


@app.route('/')
def home():
    conn = sqlite3.connect('racing.db')
    c = conn.cursor()

    # Retrieve the race data from the database
    c.execute("SELECT * FROM race_data")
    race_data = c.fetchall()

    # Close the connection
    conn.close()

    return render_template('index.html', race_data=race_data)

@app.route('/upload_data', methods=['POST'])
def upload_data():
    data = request.get_json()
    lat = data['lat']
    lng = data['lng']
    groundspeed = data['groundspeed']

    conn = sqlite3.connect('racing.db')
    c = conn.cursor()

    # Insert the data into the race_data table
    c.execute("INSERT INTO race_data (lat, lng, groundspeed) VALUES (?, ?, ?)", (lat, lng, groundspeed))

    # Save the changes and close the connection
    conn.commit()
    conn.close()

    return 'Data uploaded successfully'

# Run the Flask app
if __name__ == '__main__':
    app.run(host='0.0.0.0',port=5000)
