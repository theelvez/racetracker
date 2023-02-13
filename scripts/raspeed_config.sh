#!/bin/bash

# This script provisions a Raspberry Pi as the IPD for the Speedtracker project

# Check if script is being run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
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


# Configure hostapd to create access point
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
wpa_passphrase=sp33dtrack3r
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# Unmask hostapd service and enable on boot
systemctl unmask hostapd
systemctl enable hostapd
systemctl enable dnsmasq

# Configure dnsmasq to handle DHCP
cat > /etc/dnsmasq.conf << EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.50,12h
EOF

# Enable dnsmasq service on boot
systemctl enable dnsmasq
systemctl restart hostapd
systemctl restart dnsmasq

# Create SQLite3 database
cat > /opt/ipd/create_db.py << EOF
from sqlalchemy import create_engine, Column, Integer, String, Float
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base

# Install required Python packages
pip3 install flask sqlalchemy

# Create SQLite3 database and tables
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
sqlite3 /opt/ipd/ipd.db < /opt/ipd/ipd.sql

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

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        driver = Driver(name=request.form['name'], car_make=request.form['car_make'], car_model=request.form['car_model'])
        session.add(driver)
        session.commit()
        driver_id = driver.id
        return redirect(url_for('qr_code', driver_id=driver_id))

    return render_template('register.html')

@app.route('/qr_code/<int:driver_id>')
def qr_code(driver_id):
    driver = session.query(Driver).filter_by(id=driver_id).first()
    url = f'https://www.speedtracker.com/driver/{driver_id}'
    qr = qrcode.QRCode(version=1, box_size=10, border=5)
    qr.add_data(url)
    qr.make(fit=True)
    img = qr.make_image(fill_color='black', back_color='white')
    img.save(f'/opt/ipd/qr_codes/{driver_id}.png')
    return render_template('qr_code.html', driver=driver, qr_code=f'/static/qr_codes/{driver_id}.png')

@app.route('/map/start')
def start_map():
    return render_template('start_map.html')

@app.route('/map/finish')
def finish_map():
    return render_template('finish_map.html')

@app.route('/map/save_start', methods=['POST'])
def save_start():
    lat = request.form['lat']
    lng = request.form['lng']
    race = session.query(Race).order_by(Race.id.desc()).first()
    race.start_lat = lat
    race.start_lng = lng
    session.commit()
    return redirect(url_for('index'))

@app.route('/map/save_finish', methods=['POST'])
def save_finish():
    lat = request.form['lat']
    lng = request.form['lng']
    race = session.query(Race).order_by(Race.id.desc()).first()
    race.finish_lat = lat
    race.finish_lng = lng
    session.commit()
    return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
