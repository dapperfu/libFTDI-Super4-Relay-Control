# Super4 C Interface

libftdi bit bang interface for [Super4 USB Relay](https://tctec.net/super4usbrelay.htm)

USB hub powered, modular relay control board.
Relays (4 x):   5 Amp 30VDC, 5Amp 250V AC

Uses [libftdi](https://www.intra2net.com/en/developer/libftdi/index.php) instead of the libftd2xx drivers from FTDI.

# Usage

    make
    ./super4 -l # List devices
    ./super4 -m 15 # All relays on.
    ./super4 -m 0 # All relays off.
    ./super4 -r # Read relay status.