wget https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-21.04-aarch64
sudo mkdir -p /opt/raspbian/sysroot
sudo tar -xf ubuntu-21.04-aarch64 -C /opt/raspbian/sysroot
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip gn
export SYSROOT_AARCH64=/opt/raspbian/sysroot