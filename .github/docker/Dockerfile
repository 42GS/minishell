FROM debian:bookworm-slim

LABEL org.opencontainers.image.source="https://github.com/42GS/minishell"
LABEL org.opencontainers.image.description="Lightweight CI environment for 42GS/minishell (gcc, make, readline, valgrind, norminette)"

RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc \
        libc6-dev \
        libc6-dbg \
        make \
        libreadline-dev \
        valgrind \
        python3 \
        python3-pip \
        git \
        ca-certificates \
    && pip3 install --no-cache-dir --break-system-packages norminette \
    && apt-get purge -y --auto-remove python3-pip \
    && rm -rf /var/lib/apt/lists/* \
    && cc --version \
    && make --version \
    && bash --version \
    && norminette --version \
    && valgrind --error-exitcode=42 /bin/true
