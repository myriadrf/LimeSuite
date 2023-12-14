FROM ubuntu:22.04 AS build-stage

WORKDIR /LimeSuite2/source

COPY install_dependencies.sh install_dependencies.sh

RUN apt update && \
    apt-get install -y --no-install-recommends \
        dpkg-dev \
        build-essential \
        debhelper \
    && \
    ./install_dependencies.sh && \
    rm -rf /var/lib/apt/lists/*

COPY amarisoft-plugin/ amarisoft-plugin/
COPY cmake/ cmake/
COPY debian/ debian/
COPY Desktop/ Desktop/
COPY external/ external/
COPY udev-rules/ udev-rules/
COPY Changelog.txt Changelog.txt
COPY CMakeLists.txt CMakeLists.txt
COPY README.md README.md
COPY src/ src/

RUN dpkg-buildpackage --build=binary --no-sign -d

FROM scratch AS export-stage
COPY --from=build-stage /LimeSuite2/*.* /
