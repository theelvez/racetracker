#!/bin/bash

# This script provisions a Raspberry Pi as the IPD for the Speedtracker project

# Check if script is being run as root
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# Set static IP address for the IPD
cat > /etc/dhcpcd.conf << EOF
interface wlan0
static ip_address=10.0.0.1/24
nohook wpa_supplicant
EOF

# Install necessary packages
apt-get update
apt-get install -y hostapd dnsmasq python3-pip sqlite3

# Configure hostapd to create access point
cat > /etc/hostapd/hostapd.conf << EOF
interface=wlan0
driver=nl80211
ssid=speedtracker
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=password
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Enable hostapd and dnsmasq services
systemctl unmask hostapd
systemctl enable hostapd
systemctl enable dnsmasq

# Configure dnsmasq to handle DHCP
cat > /etc/dnsmasq.conf << EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.50,12h
EOF

# Restart services to apply changes
systemctl restart dnsmasq
systemctl restart hostapd

# Install required Python packages
pip3 install flask sqlalchemy

# Create SQLite3 database and tables
cat > /opt/ipd/ipd.sql << EOF
CREATE TABLE driver (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    car_make TEXT NOT NULL,
    car_model TEXT NOT NULL
);
CREATE TABLE race (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    start_lat REAL NOT NULL,
    start_lng REAL NOT NULL,
    finish_lat REAL NOT NULL,
    finish_lng REAL NOT NULL,
    top_speed REAL NOT NULL,
    FOREIGN KEY (driver_id) REFERENCES driver (id)
);
EOF
sqlite3 /opt/ipd/ipd.db < /opt/ipd/ipd.sql

# Create Python Flask app
cat > /opt/ipd/ipd.py << EOF
import os
from flask import Flask, render_template, request
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

app = Flask(__name__)
engine = create_engine('sqlite:////opt/ipd/ipd.db', echo=True)

class Driver(object):
    def __init__(self, name, car_make, car_model):
        self.name = name
        self.car_make = car_make
        self.car_model = car_model

    def __repr__(self):
        return f'<Driver(name={self.name}, car_make={self.car_make}, car_model={self.car_model})>'

class Race(object):
    def __init__(self, driver_id, start_lat, start_lng, finish_lat, finish_lng, top_speed):
        self.driver_id = driver_id
        self.start_lat = start_lat
        self.start_lng = start_lng
        self.finish_lat = finish_lat
        self.finish_lng = finish_lng
        self.top_speed = top_speed

    def __repr__(self):
        return f'<Race(driver_id={self.driver_id}, start_lat={self.start_lat}, start_lng={self
