FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    python3 \
    python3-venv \
    wget \
    gnupg \
    lsb-release \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/platformio

RUN /opt/platformio/bin/pip install platformio==6.1.18

ENV PATH="/opt/platformio/bin:$PATH"

RUN pio --version

# LLVM / clang-format
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 21 && \
    apt-get update && \
    apt-get install -y clang-format-21 && \
    rm -rf /var/lib/apt/lists/*

RUN clang-format-21 --version
RUN pio --version