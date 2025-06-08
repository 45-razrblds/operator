FROM --platform=linux/amd64 ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    gcc-multilib \
    grub-pc-bin \
    grub-common \
    grub2-common \
    xorriso \
    nasm \
    mtools \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace