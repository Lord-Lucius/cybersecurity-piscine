# Généré par dsh — édite .dsh-packages, pas ce fichier
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
        bear \
        build-essential \
        libpcap-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /work
