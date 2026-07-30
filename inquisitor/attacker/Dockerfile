FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    libpcap-dev \
    iproute2 \
    net-tools \
    iputils-ping \
    ftp \
    procps \
    && rm -rf /var/lib/apt/lists/*
