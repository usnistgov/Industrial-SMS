gr-scan
=======

Fast radio spectrum scanner based on SDR and GnuRadio.

Tune to frequencies in specified range and tri fing peak in frequency power spectrum.
Print frequency and bandwidth of found peaks.

### Example

    $ sudo ./gr-scan -x 458 -y 462 -o toto.csv
    [...]
    00:00:01: Finished scanning 457.000000 MHz - 459.000000 MHz
    00:00:02: Finished scanning 457.500000 MHz - 459.500000 MHz
    00:00:03: Finished scanning 458.000000 MHz - 460.000000 MHz
    00:00:04: Finished scanning 458.500000 MHz - 460.500000 MHz
    00:00:05: Finished scanning 459.000000 MHz - 461.000000 MHz
    [+] 00:00:05: Found signal: at 460.796500 MHz of width 21.000000 kHz, peak power -74.304337 dB (difference 4.142212 dB)
    00:00:06: Finished scanning 459.500000 MHz - 461.500000 MHz
    00:00:07: Finished scanning 460.000000 MHz - 462.000000 MHz
    00:00:08: Finished scanning 460.500000 MHz - 462.500000 MHz
    00:00:09: Finished scanning 461.000000 MHz - 463.000000 MHz
    
    $ cat toto.csv 
    time,frequency_mhz,width_khz,peak,diff
    20151222_032034,460.796509,21.000000,-74.304337,4.142212
