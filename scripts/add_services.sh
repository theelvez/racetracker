#!/bin/bash

# Check if script is being run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

# Create user if it does not exist
if ! id -u srv_runner >/dev/null 2>&1; then
    useradd -r -s /bin/false srv_runner
fi

# Create service file
cat > /etc/systemd/system/speedtracker.service << EOF
[Unit]
Description=Speedtracker Service
After=network.target

[Service]
User=srv_runner
WorkingDirectory=/home/pi/source/repos/speedtracker
ExecStart=/usr/bin/python /home/pi/source/repos/speedtracker/app.py
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd daemon
systemctl daemon-reload

# Enable service to start at boot time
systemctl enable speedtracker.service
