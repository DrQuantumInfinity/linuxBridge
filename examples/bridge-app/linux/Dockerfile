FROM ubuntu:20.04

RUN wget https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-21.04-aarch64

RUN tar -xf ubuntu-21.04-aarch64 /opt/raspbian/sysroot

ENV SYSROOT_AARCH64 /opt/raspian/sysroot/

COPY 