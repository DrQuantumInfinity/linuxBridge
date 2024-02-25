export SYSROOT_AARCH64=/opt/raspbian/sysroot
cd ../../.. &&
./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-arm64-bridge-clang build"