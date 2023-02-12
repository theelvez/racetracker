#!/bin/bash

# Install required packages
sudo apt-get update
sudo apt-get install -y hostapd dnsmasq sqlite3 python3-flask

# Stop running services
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq

# Configure wlan0 for static IP
sudo cat > /etc/network/interfaces.d/wlan0 <<EOF
auto wlan0
iface wlan0 inet static
    address 10.0.0.1
    netmask 255.255.255.0
EOF

# Configure hostapd
sudo cat > /etc/hostapd/hostapd.conf <<EOF
interface=wlan0
ssid=speedtracker
hw_mode=g
channel=7
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=mypassword
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Configure dnsmasq
sudo cat > /etc/dnsmasq.conf <<EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.50,12h
EOF

# Restart services
sudo systemctl restart hostapd
sudo systemctl restart dnsmasq

# Create the SQLite3 database and tables
sudo sqlite3 speedtracker.db <<EOF
CREATE TABLE drivers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, car_make TEXT, car_model TEXT, qr_code TEXT);
CREATE TABLE races (id INTEGER PRIMARY KEY AUTOINCREMENT, driver_id INTEGER, start_time TEXT, start_latitude REAL, start_longitude REAL, finish_latitude REAL, finish_longitude REAL, highest_speed REAL);
EOF

# Create the Flask web app
sudo mkdir /var/www
sudo cat > /var/www/racehub.py <<EOF
from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80)
EOF

# Create the HTML template for the website
sudo mkdir /var/www/templates
sudo cat > /var/www/templates/index.html <<EOF
<!doctype html>
<html>
<head>
    <title>Race Hub</title>
</head>
<body>
    <h1>Race Hub</h1>
    <h2>Current Race</h2>
    <table>
        <thead>
            <tr>
                <th>Driver Name</th>
                <th>Car Make</th>
                <th>Car Model</th>
                <th>Highest Speed</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>John Doe</td>
                <td>Ford</td>
                <td>Mustang</td>
                <td>150 mph</td>
            </tr>
        </tbody>
    </table>
    <h2>Next 5 Drivers</h2>
    <ol>
        <li>Jane Doe</li>
        <li>Bob Smith</li>
        <li>Alice Johnson</li>
        <li>Tom Green</li>
        <li>Susan Lee</li>
    </ol>
    <h2>Previous Races</h2>
    <table>
        <thead>
            <tr>
                <th>Driver Name</th>
                
                
                
# 4. Install the required Python packages for Flask and SQLite3
sudo apt-get update
sudo apt-get install python3 python3-pip sqlite3
sudo pip3 install flask

# 5. Create a directory to hold the Flask application and database
sudo mkdir /var/www/speedtracker
cd /var/www/speedtracker

# 6. Create a new Flask application and SQLite3 database
sudo touch app.py
sudo touch speedtracker.db
sudo chmod 777 speedtracker.db

# 7. Add the following code to app.py
# (Please note that this is just a basic Flask application, you will need to add your own custom routes and functions)
cat <<EOT >> app.py
from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return 'Welcome to the Speedtracker Race Hub!'

if __name__ == '__main__':
    app.run(host='0.0.0.0')
EOT

# 8. Initialize the database with the required tables
sqlite3 speedtracker.db < create_tables.sql

# 9. Set up a systemd service for the Flask application
sudo touch /etc/systemd/system/speedtracker.service
cat <<EOT >> /etc/systemd/system/speedtracker.service
[Unit]
Description=Speedtracker Flask App
After=network.target

[Service]
User=pi
WorkingDirectory=/var/www/speedtracker
ExecStart=/usr/bin/python3 app.py
Restart=always

[Install]
WantedBy=multi-user.target
EOT

# 10. Start the service and enable it to start on boot
sudo systemctl daemon-reload
sudo systemctl start speedtracker
sudo systemctl enable speedtracker

# 11. Set up the IPD to be an access point for the speedtracker network
sudo cp /etc/dhcpcd.conf /etc/dhcpcd.conf.bak
sudo touch /etc/hostapd/hostapd.conf
sudo chmod 777 /etc/hostapd/hostapd.conf

# 12. Add the following code to /etc/dhcpcd.conf
cat <<EOT >> /etc/dhcpcd.conf
interface wlan0
    static ip_address=10.0.0.1/24
    nohook wpa_supplicant
EOT

# 13. Add the following code to /etc/hostapd/hostapd.conf
cat <<EOT >> /etc/hostapd/hostapd.conf
interface=wlan0
ssid=speedtracker
hw_mode=g
channel=6
ieee80211n=1
wmm_enabled=1
ht_capab=[HT40][SHORT-GI-20][DSSS_CCK-40]
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=MySuperSecretPassword
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOT

# 14. Restart the hostapd service
sudo systemctl restart hostapd

# 15. Reboot the IPD to apply all changes
sudo reboot
