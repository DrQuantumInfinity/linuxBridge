export SYSROOT_AARCH64=/opt/raspbian/sysroot
cd ../../.. &&
./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-arm64-bridge-clang build"

DEST="matter@192.168.0.128"
deploy () {
  scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ./out/linux-arm64-bridge-clang/chip-bridge-app $DEST:~
}

if [ "$1" == "--deploy" ]; then
    deploy
fi