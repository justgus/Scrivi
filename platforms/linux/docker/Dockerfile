# Scrivi Linux app — build + headless-GUI (Xvfb + VNC) image.
#
# EP-020 Step-0 pin (2026-07-13): Ubuntu 24.04 LTS + Qt 6.4 (from apt).
#
# Build context is the REPO ROOT (so the whole CMake tree — ScriviCore +
# platforms/linux — is available). From the repo root:
#
#   docker build -f platforms/linux/docker/Dockerfile -t scrivi-linux .
#   docker run --rm -p 5900:5900 scrivi-linux
#
# Then connect a VNC client on the Mac to localhost:5900 (e.g. Finder →
# Go → Connect to Server → vnc://localhost:5900). See run-vnc.sh for details.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Toolchain + Qt 6.4 (Quick/QML) + a headless X stack (Xvfb + x11-utils for
# xdpyinfo, used by run-vnc.sh to wait for the display) and a VNC server (x11vnc)
# so the GUI is viewable from the host. git/curl are pulled for CMake
# FetchContent (nlohmann/json) at configure time.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        ca-certificates \
        qt6-base-dev \
        qt6-declarative-dev \
        qml6-module-qtquick \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-templates \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-window \
        qml6-module-qtqml-workerscript \
        qml6-module-qtqml-models \
        libgl1-mesa-dev \
        xvfb \
        x11-utils \
        x11vnc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# Configure + build only the Linux app (and its ScriviCore dependency). Tests
# off — the ScriviCore ctest suite runs in its own CI job, not here.
RUN cmake -S . -B build -G Ninja \
        -DSCRIVI_BUILD_LINUX=ON \
        -DSCRIVI_BUILD_TESTS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

COPY platforms/linux/docker/run-vnc.sh /usr/local/bin/run-vnc.sh
RUN chmod +x /usr/local/bin/run-vnc.sh

EXPOSE 5900
CMD ["/usr/local/bin/run-vnc.sh"]
