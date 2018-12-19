FROM registry.gitlab.com/pantacor/pv-platforms/alpine-base:ARM32V6 as sysroot

RUN apk update && apk --force add git build-base util-linux binutils grep bash-completion findutils coreutils usbutils gcc abuild cmake g++ sqlite sqlite-dev libusb libusb-dev fftw fftw-dev raspberrypi-dev raspberrypi-libs libpng libpng-dev libvncserver-dev ffmpeg-dev python3 libstdc++ && pip3 install requests && rm -rf /var/cache/apk/*; echo 'http://dl-cdn.alpinelinux.org/alpine/edge/testing' >> /etc/apk/repositories; apk update && apk add --force i2c-tools-dev && rm -rf /var/cache/apk/*

FROM registry.gitlab.com/pantacor/platform-tools/docker-musl-cross-arm as crossbuilder

WORKDIR /work
RUN mkdir /work/stage; apt-get update; apt-get install cmake cmake-data -y; apt-get clean

COPY --from=sysroot / /sysroot-arm
COPY . src

RUN cd src; cmake --debug-output -DCMAKE_TOOLCHAIN_FILE=cmake-cross/musl-arm32v6; make; make install

FROM registry.gitlab.com/pantacor/pv-platforms/alpine-base:ARM32V6

RUN apk update && apk add libstdc++ libusb  && rm -rf /var/cache/apk/*

COPY --from=crossbuilder /work/stage /usr/local

