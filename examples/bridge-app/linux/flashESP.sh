
                      
source ./bin/activate  

RST_PIN=2
PROG_PIN=3
pinctrl set $PROG_PIN op dl
pinctrl set $RST_PIN op dl
sleep 0.1
pinctrl set $RST_PIN op dh

esptool.py -p /dev/ttyEspNow -b 460800 --before default_reset --after hard_reset --chip esp8266  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x10000 firmware.bin


pinctrl set $PROG_PIN op dh
pinctrl set $RST_PIN op dl
sleep 0.1
pinctrl set $RST_PIN op dh