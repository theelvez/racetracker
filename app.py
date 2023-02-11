# app.py

from flask import Flask, render_template, request
import sqlite3

app = Flask(__name__)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/upload_races", methods=["POST"])
def upload_races():
    if request.method == "POST":
        # get the data from the ICD
        data = request.get_json()

        # insert the data into the database
        conn = sqlite3.connect("speedtracker.db")
        c = conn.cursor()
        c.execute("INSERT INTO races (driver_id, car_id, start_time, finish_time, highest_speed) VALUES (?, ?, ?, ?, ?)", (data["driver_id"], data["car_id"], data["start_time"], data["finish_time"], data["highest_speed"]))
        conn.commit()
        conn.close()

        return "Success"

@app.route("/add_driver", methods=["POST"])
def add_driver():
    if request.method == "POST":
        # get the data from the ICD
        data = request.get_json()

        # insert the data into the database
        conn = sqlite3.connect("speedtracker.db")
        c = conn.cursor()
        c.execute("INSERT INTO drivers (name, car_make, car_model, car_year) VALUES (?, ?, ?, ?)", (data["name"], data["car_make"], data["car_model"], data["car_year"]))
        conn.commit()
        conn.close()

        return "Success"


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)
