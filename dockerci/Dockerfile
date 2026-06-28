FROM python:3.11-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    g++ \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir \
    platformio==6.1.18 \
    clang-format==21.1.0

RUN clang-format --version && pio --version