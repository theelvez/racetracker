from flask import Flask, render_template, request
import qrcode
import sqlite3

app = Flask(__name__)

# Define a route to serve the driver registration form
@app.route("/")
def registration_form():
    return render_template("registration.html")

# Define a route to handle the form submission
@app.route("/register", methods=["POST"])
def register_driver():
    # Retrieve the form data
    name = request.form.get("name")
    car_make = request.form.get("car_make")
    car_model = request.form.get("car_model")

    # Generate a unique ID for the driver
    driver_id = generate_driver_id()

    # Add the driver to the database
    conn = sqlite3.connect("speedtracker.db")
    c = conn.cursor()
    c.execute("INSERT INTO drivers (id, name, car_make, car_model) VALUES (?, ?, ?, ?)", (driver_id, name, car_make, car_model))
    conn.commit()
    conn.close()

    # Generate the QR code for the driver ID
    qr = qrcode.QRCode(version=1, error_correction=qrcode.constants.ERROR_CORRECT_L, box_size=10, border=4)
    qr.add_data(driver_id)
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")

    # Serve the driver ID and QR code to the user
    return render_template("qr_code.html", driver_id=driver_id, qr_code=img)

def generate_driver_id():
    # Generate a random 8-character driver ID
    import random
    import string
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=8))

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
