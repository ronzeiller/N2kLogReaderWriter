# N2kLogReaderWriter for Teensy 3.6

Reads a logfile from internal SD-Card and writes NMEA2000 sentences to the bus.
Timestamps in log file are taken for real time simulation.

## Log fie format
similar to Seasmart $PCDIN sentences but with timestamp, PGN in decimals for better human reading

timestamp, PGN, Source, Data, checksum

@1004,127245,02,00FFFF7F0AFEFFFF*2F
