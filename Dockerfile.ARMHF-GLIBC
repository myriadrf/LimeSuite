FROM raspbian/stretch as sysroot

COPY rerelln.sh rerelln.sh

RUN apt-get update && apt-get install -y \
	git \
	build-essential \
	bash-completion \
	findutils \
	coreutils \
	usbutils \
	gcc \
	cmake \
	g++ \
	libsqlite3-dev \
	libusb-1.0-0-dev \
	libfftw3-dev \
	libpng-dev \
	python3 \
	python3-pip \
	libstdc++-6-dev \
	libi2c-dev \
	&& pip3 install requests \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists \
	&& sh rerelln.sh /usr/lib/arm-linux-gnueabihf/libusb-1.0.so


FROM registry.gitlab.com/pantacor/platform-tools/docker-glibc-cross-arm:master as crossbuilder

WORKDIR /work
RUN mkdir /work/stage; apt-get update; apt-get install cmake cmake-data -y; apt-get clean

COPY --from=sysroot / /sysroot-arm
COPY . src

RUN cd src && cmake -DCMAKE_TOOLCHAIN_FILE=cmake-cross/glibc-armhf \
	&& make \
	&& make install || true

FROM raspbian/stretch

RUN apt-get update; apt-get install libusb-1.0-0 libfftw3-3 && apt-get clean; rm -rf /var/lib/apt/lists

COPY --from=crossbuilder /work/stage /usr/local

RUN ldconfig

