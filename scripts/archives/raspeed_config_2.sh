#!/bin/bash

# Check if script is being run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

# Install required packages
apt-get update
apt-get -y install hostapd dnsmasq python3-pip

# Configure static IP address
cat > /etc/dhcpcd.conf << EOF
interface wlan0
static ip_address=10.0.0.1/24
nohook wpa_supplicant
EOF

# Configure hostapd
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
wpa_passphrase=speedtracker
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Enable hostapd and dnsmasq services
systemctl unmask hostapd
systemctl enable hostapd
systemctl enable dnsmasq

# Configure dnsmasq
cat > /etc/dnsmasq.conf << EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.50,255.255.255.0,24h
EOF

# Restart networking services
systemctl restart dhcpcd
systemctl restart dnsmasq

# Install required Python packages
pip3 install Flask SQLAlchemy

# Create SQLite3 database and tables
cat > /opt/ipd/ipd.py << EOF
from sqlalchemy import create_engine, Column, Integer, String, Float
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()

engine = create_engine('sqlite:////opt/ipd/ipd.db', echo=True)

class Driver(Base):
    __tablename__ = 'drivers'

    id = Column(Integer, primary_key=True)
    name = Column(String)
    car_make = Column(String)
    car_model = Column(String)

class Race(Base):
    __tablename__ = 'races'

    id = Column(Integer, primary_key=True)
    driver_id = Column(Integer)
    start_lat = Column(Float)
    start_lng = Column(Float)
    finish_lat = Column(Float)
    finish_lng = Column(Float)
    top_speed = Column(Float)

Base.metadata.create_all(engine)
EOF

# Configure Flask web app
cat > /opt/ipd/app.py << EOF
from flask import Flask, render_template, request, redirect, url_for
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from ipd import Base, Driver, Race

app = Flask(__name__)

engine = create_engine('sqlite:////opt/ipd/ipd.db')
Base.metadata.bind = engine

DBSession = sessionmaker(bind=engine)
session = DBSession()

@app.route('/')
def index():
    drivers = session.query(Driver).all()
    races = session.query(Race).all()
    return render_template('index.html', drivers=drivers, races=races)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
EOF

# Create Flask app template
mkdir /opt/ipd/templates
cat > /opt/ipd/templates/index.html << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Race Hub</title>
    <style>
        body {
            background-color: black
