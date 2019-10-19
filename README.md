# N2kLogReaderWriter for Teensy 3.6

Reads a logfile from internal SD-Card and writes NMEA2000 sentences to the bus.
Timestamps in log file are taken for real time simulation.

## Log fie format
similar to Seasmart $PCDIN sentences but with timestamp and PGN in decimals for better human reading.

Conversion into and from tN2k.msg in https://github.com/ronzeiller/N2kLogReaderWriter/tree/master/lib/SailmaxFormat/src

(Logged with a modified version of KBox V0)

timestamp, PGN, Source, Data, checksum

@1004,127245,02,00FFFF7F0AFEFFFF*2F


You will find a 2.4GB logfile of a Bavaria 41s during Round Palagruza Cannonball regatta in April 2018
starting in Biograd/Croatia, pre start, start at 2pm,.....

https://www.zeiller.eu/downloads/RPC2018.log

### Units on the bus:
Garmin GPSMap 721

Garmin GMI20

Garmin GWS 20

Actisense NGT-1

## Version History
20.10.2019
    NMEA2000 Library aktualisiert, Kommentare eingefügt.
    3 Branches
    master: normales einlesen in setup() {}
    test-with-loop: in loop()
    while-in-setup-version: eine der beiden ist mit Start/Stop und Springe zu Zeile nn

Achtung!!!
  Sobald in PlatformIO die lib NMEA2000_esp32 installiert wird gibt es build-Error
  soc/dport_reg.h not found
