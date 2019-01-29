#!/bin/sh
RESET_GPIO=18
SR2_RESET_GPIO=32
if [ -f /sys/class/gpio/gpio$RESET_GPIO/direction ]; then
  echo "GPIO$RESET_GPIO as ZigBee Reset"
else
  echo "$RESET_GPIO" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio$RESET_GPIO/direction
fi

if [ -f /sys/class/gpio/gpio$SR2_RESET_GPIO/direction ]; then
  echo "GPIO$SR2_RESET_GPIO as ZigBee Reset"
else
  echo "$SR2_RESET_GPIO" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio$SR2_RESET_GPIO/direction
fi

if [ -f /sys/class/gpio/gpio49/direction ]; then
  echo "GPIO49 inited"
else
  echo "49" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio49/direction
fi

echo "0" > /sys/class/gpio/gpio49/value
sleep 1
echo "0" > /sys/class/gpio/gpio49/value
# reset CC2630
echo "1" > /sys/class/gpio/gpio$RESET_GPIO/value
echo "1" > /sys/class/gpio/gpio$SR2_RESET_GPIO/value
sleep 1
echo "0" > /sys/class/gpio/gpio$RESET_GPIO/value
echo "0" > /sys/class/gpio/gpio$SR2_RESET_GPIO/value
sleep 1
# reset done
echo "1" > /sys/class/gpio/gpio$RESET_GPIO/value
echo "1" > /sys/class/gpio/gpio$SR2_RESET_GPIO/value
echo "0" > /sys/class/gpio/gpio49/value
#picocom -b 115200 -r -l /dev/ttyMSM1
