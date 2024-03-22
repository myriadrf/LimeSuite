# Build .deb file for Debian 11, Ubuntu 20.04, Ubuntu 22.04
FROM ubuntu:20.04 AS build-stage

WORKDIR /LimeSuite2/source

COPY install_dependencies.sh install_dependencies.sh

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && \
    apt-get install -y --no-install-recommends \
        debhelper \
        dh-python \
        dpkg-dev \
    && \
    ./install_dependencies.sh -y && \
    rm -rf /var/lib/apt/lists/*

COPY cmake/ cmake/
COPY debian/ debian/
COPY debian/control_soapy0.7 debian/control
COPY external/ external/
COPY udev-rules/ udev-rules/
COPY plugins/ plugins/
COPY Changelog.txt Changelog.txt
COPY CMakeLists.txt CMakeLists.txt
COPY README.md README.md
COPY src/ src/

RUN dpkg-buildpackage --build=binary --no-sign -d

FROM scratch AS export-stage
COPY --from=build-stage /LimeSuite2/*.* /
