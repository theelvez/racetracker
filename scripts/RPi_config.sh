
# Configure HostAPD
sudo cat > /etc/hostapd/hostapd.conf << EOF
interface=wlan0
driver=nl80211
ssid=speedtracker
hw_mode=g
channel=6
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
max_num_sta=10
wpa=2
wpa_passphrase=speedtracker123
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
rsn_pairwise=CCMP
dhcp_server=yes
EOF

# Configure DHCP server
sudo cat > /etc/dnsmasq.conf << EOF
interface=wlan0
denyinterfaces wlan0
dhcp-range=10.0.0.2,10.0.0.10,255.255.255.0,24h
EOF

# Install packages
sudo apt-get install -y hostapd dnsmasq

# Shutdown the services
sudo systemctl stop dnsmasq
sudo systemctl stop hostapd

# Assign a static IP address to this machine
echo "interface wlan0" >> /etc/dhcpcd.conf
echo "  static ip_address=10.0.0.1/24" >> /etc/dhcpcd.conf
echo "  nohook wpa_supplicant" >> /etc/dhcpcd.conf

# Now restart the dhcpcd daemon
sudo service dhcpcd restart

# Configure hostapd service
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

# Configure dnsmasq service
sudo systemctl enable dnsmasq
sudo systemctl start dnsmasq

echo "Raspberry Pi 4 configured as WiFi access point 'speedtracker'!"