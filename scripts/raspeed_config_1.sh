#!/bin/bash

# Check if script is being run as super user
if [[ $(id -u) -ne 0 ]]; then
    echo "Script must be run as super user. Exiting."
    exit 1
fi

# Install required packages
apt-get update
apt-get install -y hostapd dnsmasq python3 python3-pip python3-sqlalchemy python3-flask sqlite3

# Stop hostapd and dnsmasq services
systemctl stop hostapd
systemctl stop dnsmasq

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
channel=6
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=password123
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Unmask hostapd service and enable on boot
systemctl unmask hostapd
systemctl enable hostapd

# Configure dnsmasq
cat > /etc/dnsmasq.conf << EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.50,12h
EOF

# Enable dnsmasq service on boot
systemctl enable dnsmasq

# Create SQLite3 database
cat > /opt/ipd/create_db.py << EOF
from sqlalchemy import create_engine, Column, Integer, String, Float
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()

class Driver(Base):
    __tablename__ = 'drivers'

    id = Column(Integer, primary_key=True)
    name = Column(String(50))
    car_make = Column(String(50))
    car_model = Column(String(50))

class Race(Base):
    __tablename__ = 'races'

    id = Column(Integer, primary_key=True)
    driver_id = Column(Integer)
    race_time = Column(String(50))
    highest_speed = Column(Float)

engine = create_engine('sqlite:////opt/ipd/ipd.db', echo=True)
Base.metadata.create_all(bind=engine)

Session = sessionmaker(bind=engine)
session = Session()
EOF

# Start SQLite3 web app on boot
cat > /etc/systemd/system/ipd-webapp.service << EOF
[Unit]
Description=IPD Flask Web App
After=network.target

[Service]
User=pi
WorkingDirectory=/opt/ipd
ExecStart=/usr/bin/python3 /opt/ipd/webapp.py
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# Create webapp script
cat > /opt/ipd/webapp.py << EOF
from flask import Flask, render_template
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

app = Flask(__name__)
app.config['SECRET_KEY'] = 'password123'
engine = create_engine('sqlite:////opt/ipd/ipd.db', echo=True)

class Driver(object):
    def __init__(self, name, car_make, car_model):
        self.name = name
        self.car_make = car_make
        self
