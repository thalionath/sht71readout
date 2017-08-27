# Sensirion SHT7x readout

C++ program to readout of [Sensirion SHT71](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/pintype-digital-humidity-sensors/) or SHT75 humidity and temperature sensor with GPIO bitbanging over sysfs. Oh my!

Writes data in [infulxdb](https://www.influxdata.com/) line format to stdio, errors to stderr.

Note: SHT7x will reach end-of-life and should not be used for new projects.

## Build

    # build and install to /usr/local/bin/sht71readout
    sudo make install

## Uninstallation

    sudo make uninstall

## Sensor Pinout

 * CLK  = GPIO 0
 * DATA = GPIO 1

## Usage

Output:

    sudo ./sht71readout
    sht71,sensor=0 status=0,t=27.090000,rh_linear=51.576065,rh_true=51.859135

`rh_true` is the temperature compensated value of `rh_linear`. Refer to the datasheet for details.

Log data every minute to influxdb via UDP protocol:

    # /etc/crontab
    * * * * * root bash -c "sht71readout > /dev/udp/127.0.0.1/8189 2> /var/log/sht71readout.log"
