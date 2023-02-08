#You: write a script to configure a raspberry pi 4 so that it exposes a WiFi access point named “racetracker”.

#!/bin/bash

# Install packages
sudo apt-get install -y hostapd dnsmasq

# Configure HostAPD
sudo cat > /etc/hostapd/hostapd.conf << EOF
interface=wlan0
driver=nl80211
ssid=racetracker
hw_mode=g
channel=6
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=Racetracker
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
rsn_pairwise=CCMP
EOF

# Configure DHCP server
sudo cat > /etc/dnsmasq.conf << EOF
interface=wlan0
dhcp-range=10.0.0.2,10.0.0.20,255.255.255.0,24h
EOF

# Enable IP forwarding
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

# Configure iptables
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT

# Save iptables
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

# Configure hostapd service
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

# Configure dnsmasq service
sudo systemctl enable dnsmasq
sudo systemctl start dnsmasq

echo "Raspberry Pi 4 configured as WiFi access point 'racetracker'!"
