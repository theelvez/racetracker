import json
import pymysql

from flask import Flask
from flask import request
from flask import render_template

app = Flask(__name__)

# Connect to MySQL database
db = pymysql.connect(host='localhost',
                     user='root',
                     password='password',
                     db='racetracker',
                     cursorclass=pymysql.cursors.DictCursor)

@app.route("/")
def home():
    # Retrieve race data from database
    with db.cursor() as cursor:
        cursor.execute('SELECT * FROM race_data')
        race_data = cursor.fetchall()

    # Create list of racers
    racers = race_data['racers']

    # Create list of race results
    results = []
    for racer in racers:
        if racer['hasFinished']:
            results.append((racer['name'],
                            racer['carModel'],
                            racer['carYear'],
                            racer['highestSpeed']))
        else:
            results.append((racer['name'],
                            racer['carModel'],
                            racer['carYear'],
                            'N/A'))

    # Render template with race data
    return render_template('result.html', 
                           race_name=race_data['name'],
                           results=results) 

@app.route("/result.html")
def result_template():
    return render_template('result.html')

if __name__ == '__main__':
    app.run(debug=True)