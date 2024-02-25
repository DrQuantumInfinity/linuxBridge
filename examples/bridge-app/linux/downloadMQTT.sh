cd ./mqtt
mkdir garbage -p
cd garbage
wget http://ports.ubuntu.com/ubuntu-ports/pool/universe/m/mosquitto/libmosquitto-dev_2.0.11-1ubuntu1.1_arm64.deb
wget http://ports.ubuntu.com/ubuntu-ports/pool/universe/m/mosquitto/libmosquitto1_2.0.11-1ubuntu1.1_arm64.deb


ar x libmosquitto-dev_2.0.11-1ubuntu1.1_arm64.deb
tar xvf data.tar.zst
ar x libmosquitto1_2.0.11-1ubuntu1.1_arm64.deb
tar xvf data.tar.zst
cp -r ./usr/lib ../lib
cp -r ./usr/include ../include