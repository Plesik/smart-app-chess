FROM ubuntu:25.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang-format \
    python3 \
    python3-venv \
    python3-pip \
    git \
    curl \
    pkg-config \
    ca-certificates \
    libssl-dev \
    libpq-dev \
    postgresql-server-dev-17 \
    zlib1g-dev \
    libzstd-dev \
    libjemalloc-dev \
    libev-dev \
    libcurl4-openssl-dev \
    libc-ares-dev \
    libnghttp2-dev \
    libldap2-dev \
    libyaml-cpp-dev \
    libfmt-dev \
    libcctz-dev \
    libre2-dev \
    libbenchmark-dev \
    libcrypto++-dev \
    libboost-context-dev \
    libboost-coroutine-dev \
    libboost-filesystem-dev \
    libboost-iostreams-dev \
    libboost-locale-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libboost-stacktrace-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-chrono-dev \
    libboost-atomic-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -B build-release -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSERVER_FEATURE_POSTGRESQL=ON

RUN cmake --build build-release -j$(nproc) --target backend


FROM ubuntu:25.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
ENV STOCKFISH_PATH=/usr/games/stockfish

RUN apt-get update && apt-get install -y \
    ca-certificates \
    libssl-dev \
    libpq5 \
    libzstd1 \
    libjemalloc2 \
    libev4 \
    libcurl4-openssl-dev \
    libc-ares-dev \
    libnghttp2-dev \
    libldap2-dev \
    libyaml-cpp-dev \
    libfmt-dev \
    libcctz-dev \
    libre2-dev \
    libbenchmark-dev \
    libcrypto++-dev \
    libboost-context-dev \
    libboost-coroutine-dev \
    libboost-filesystem-dev \
    libboost-iostreams-dev \
    libboost-locale-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libboost-stacktrace-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-chrono-dev \
    libboost-atomic-dev \
    stockfish \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/build-release/backend /app/backend
COPY configs /app/configs

EXPOSE 8080

CMD ["/app/backend", "--config", "/app/configs/static_config.yaml", "--config_vars", "/app/configs/config_vars.yaml"]