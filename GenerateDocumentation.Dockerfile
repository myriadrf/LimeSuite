# Build the same version of Doxygen as it is in Netlify
FROM gcc:13 AS doxygen-build
WORKDIR /doxygen

RUN apt update && \
    apt-get install -y --no-install-recommends \
        cmake \
        flex \
        bison \
    && \
    rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/doxygen/doxygen.git
WORKDIR /doxygen/doxygen
RUN git checkout Release_1_8_17

RUN cmake -G "Unix Makefiles" -B build . 
RUN make -C build --no-print-directory -j$(nproc) install

FROM python:3.8-bookworm AS build-stage
WORKDIR /documentation

RUN apt update && \
    apt-get install -y --no-install-recommends \
        cmake \
        libusb-1.0-0-dev \
    && \
    rm -rf /var/lib/apt/lists/*

COPY docs/requirements.txt docs/requirements.txt

RUN pip install -r docs/requirements.txt

COPY Desktop/ Desktop/
COPY external/ external/
COPY cmake/ cmake/
COPY amarisoft-plugin/ amarisoft-plugin/
COPY Changelog.txt Changelog.txt
COPY CMakeLists.txt CMakeLists.txt
COPY src/ src/

COPY docs/ docs/
COPY --from=doxygen-build /usr/local/bin/doxygen /usr/bin/doxygen
COPY --from=doxygen-build /usr/local/lib64/libstdc++.so /lib/x86_64-linux-gnu/libstdc++.so.6

WORKDIR /documentation/docs
RUN ./generate_docs.sh

FROM scratch AS export-stage
COPY --from=build-stage /documentation/docs/_build/html /
