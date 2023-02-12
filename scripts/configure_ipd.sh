# Install hostapd and dnsmasq
sudo apt-get update
sudo apt-get install hostapd dnsmasq -y

# Configure hostapd
sudo systemctl stop hostapd
sudo systemctl disable hostapd

interface=wlan0
driver=nl80211
ssid=speedtracker
hw_mode=g
channel=1
wmm_enabled=0

echo "interface wlan0
    static ip_address=10.0.0.1/24
    nohook wpa_supplicant
    ssid=speedtracker
    channel=1
    hw_mode=g
    auth_algs=1
    wmm_enabled=0
" | sudo tee /etc/dhcpcd.conf > /dev/null

# Configure dnsmasq
echo "interface=wlan0
    dhcp-range=10.0.0.2,10.0.0.40,255.255.255.0,24h
    domain=speedtracker.local
    address=/speedtracker.local/10.0.0.1
" | sudo tee /etc/dnsmasq.conf > /dev/null

# Enable routing
echo "net.ipv4.ip_forward=1" | sudo tee /etc/sysctl.conf > /dev/null
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

# Install Flask
sudo apt-get install python3-pip -y
pip3 install flask

# Configure the Flask app
echo "from flask import Flask, request

app = Flask(__name__)

@app.route('/speedtracker', methods=['POST'])
def speedtracker():
    data = request.json
    # Process the data as needed
    return 'OK'

if __name__ == '__main__':
    app.run(host='0.0.0.0')" | sudo tee /home/pi/speedtracker.py > /dev/null

# Start the Flask app
export FLASK_APP=/home/pi/speedtracker.py
flask run --host=0.0.0.0 &
