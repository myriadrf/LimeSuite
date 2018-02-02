#!/bin/sh

cp 64-limesuite.rules /etc/udev/rules.d/
udevadm control --reload-rules
udevadm trigger
