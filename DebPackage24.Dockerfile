# Build .deb file for Debian 12, Ubuntu 23.10, Ubuntu 24.04
FROM debian:bookworm AS build-stage

WORKDIR /LimeSuite2/source

COPY install_dependencies.sh install_dependencies.sh

RUN apt update && \
    apt-get install -y --no-install-recommends \
        debhelper \
        dh-python \
        dpkg-dev \
        gnuradio-dev \
    && \
    ./install_dependencies.sh -y && \
    rm -rf /var/lib/apt/lists/*

COPY cmake/ cmake/
COPY debian/ debian/
COPY external/ external/
COPY udev-rules/ udev-rules/
COPY plugins/ plugins/
COPY Changelog.txt Changelog.txt
COPY CMakeLists.txt CMakeLists.txt
COPY README.md README.md
COPY src/ src/

RUN patch debian/control < debian/control.patch
RUN patch debian/rules < debian/rules.patch
RUN dpkg-buildpackage --build=binary --no-sign

FROM scratch AS export-stage
COPY --from=build-stage /LimeSuite2/*.* /
