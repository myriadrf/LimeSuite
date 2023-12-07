FROM python:3.8-bookworm AS build-stage
WORKDIR /documentation

RUN apt update && \
    apt-get install -y --no-install-recommends \
        libwxbase3.2-1 \
        libwxgtk3.2-1 \
        libwxgtk3.2-dev \
        wx-common \
        wx3.2-headers \
        cmake \
        libusb-1.0-0-dev \
        doxygen \
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

RUN cmake -B build
RUN make -C build --no-print-directory -j$(nproc) doc
RUN breathe-apidoc --generate class --members --force --output-dir docs/apidoc build/xml/

WORKDIR /documentation/docs

RUN make -j$(nproc) html

FROM scratch AS export-stage
COPY --from=build-stage /documentation/docs/_build/html /
