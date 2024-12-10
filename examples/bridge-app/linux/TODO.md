Test the light
    -hard code inject a serial message
    -may lead to moooore

why brightness/hsv not work

Test multicluster devices
    -like how does a RGB and temperature light at the same time work?

ESPnow class
    -provide an array of clusters to the device
    -bidirectional serialization from ESPNowMessage to cluster readers/writers
    -makes unique device types obsolete

Figure out mode clusters

linux bridge

Thermostat

other circuit board

persistent device detection


OTA
https://project-chip.github.io/connectedhomeip-doc/guides/esp32/ota.html

thread
https://openthread.io/codelabs/openthread-border-router#1

Mode
https://github.com/project-chip/connectedhomeip/blob/f09120f41f3d32678454db1915730f6f5730bb39/src/app/clusters/mode-base-server/README.md?plain=1#L7



nightlights for christmas

rgb pwm led
identify pins
app task to setup leds
3d printing/boxes

stretch goals
switch to change led brightness

temp sensor


******************************************Linux Bridge first-time Setup:
sudo apt-get install -y libmosquitto-dev
wget http://launchpadlibrarian.net/741613665/libssl1.1_1.1.1f-1ubuntu2.23_arm64.deb
sudo dpkg -i libssl1.1_1.1.1f-1ubuntu2.23_arm64.deb 

sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
sudo nano /etc/mosquitto/mosquitto.conf
//Add "listener 1883" and "allow_anonymous true" to the end
sudo systemctl restart mosquitto

apt install python3.10-venv
python3 -m venv ./espVenv/
cd espVenv/
source ./bin/activate

pip install esptool



***** ubuntu

**enable serial port
remove console=... from /boot/firmware/cmdline.txt

create file named /lib/systemd/system/esp-serial.service with one of these two in it
[Unit]
Description=My Sample Service
After=multi-user.target

[Service]
Type=idle
ExecStart=cmd

[Install]
WantedBy=multi-user.target

where cmd is one of:
sudo ln -s /dev/ttyS0 /dev/ttyEspNow

sudo ln -s /dev/ttyAMA0 /dev/ttyEspNow

sudo systemctl daemon-reload
sudo systemctl enable esp-serial.service

**pinctrl

sudo apt install g++

git clone https://github.com/raspberrypi/utils.git
cd utils
cmake .
make
sudo make install




BugFoots:
sometimes MQTT "get" updates don't show up on google. Happened once???