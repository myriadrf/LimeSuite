# Build .deb file for Debian 11, Ubuntu 20.04, Ubuntu 22.04
FROM ubuntu:20.04 AS build-stage

WORKDIR /LimeSuite2/source

COPY install_dependencies.sh install_dependencies.sh

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && \
    apt-get install -y --no-install-recommends \
        dpkg-dev \
        debhelper \
    && \
    ./install_dependencies.sh -y && \
    rm -rf /var/lib/apt/lists/*

COPY amarisoft-plugin/ amarisoft-plugin/
COPY cmake/ cmake/
COPY debian/ debian/
COPY debian/control_soapy0.7 debian/control
COPY Desktop/ Desktop/
COPY external/ external/
COPY udev-rules/ udev-rules/
COPY SoapyLMS7/ SoapyLMS7/
COPY Changelog.txt Changelog.txt
COPY CMakeLists.txt CMakeLists.txt
COPY README.md README.md
COPY src/ src/

# Workaround to compile SoapySDR using C++17,
# as in SoapySDR's 0.7 CMake file there exists a line
# set(CMAKE_CXX_STANDARD 11)
# which forces the SoapySDR compilation to use C++11 no matter what.
# This issue is fixed in SoapySDR 0.8.
# As far as I can see this is not causing any issues with linking to the SoapySDR library.
RUN sed -i "s/set(CMAKE_CXX_STANDARD 11)/set(CMAKE_CXX_STANDARD 17)/g" /usr/share/cmake/SoapySDR/SoapySDRConfig.cmake && \
    dpkg-buildpackage --build=binary --no-sign -d

FROM scratch AS export-stage
COPY --from=build-stage /LimeSuite2/*.* /
