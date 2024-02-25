wget https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-21.04-aarch64
sudo mkdir -p /opt/raspbian/sysroot
sudo tar -xf ubuntu-21.04-aarch64 -C /opt/raspbian/sysroot
export SYSROOT_AARCH64=/opt/raspbian/sysroot