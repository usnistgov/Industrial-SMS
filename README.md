# Industrial-SMS
Spectrum Monitoring serves as the eyes and ears of the spectrum management process helping spectrum managers to plan and use frequencies, avoid incompatible usage, and identify sources of harmful interference. The key objective for this research at the National Institute of Standards and Technology (NIST) is to develop a prototype for spectral analysis of both licensed and unlicensed radio frequency (RF) bands, allowing the monitoring of the industrial environment and the access of that information in a simple way for use at industrial facilities, including factories, warehouse and discrete manufacturing, assembly, robotics, oil and gas refineries, and water and wastewater treatment plants.

This prototype called Industrial Spectrum Monitoring System (ISMS) is an application based on GNU Radio software development toolkit that monitors the RF spectrum and reports metrics on discovered signals. These metrics include the frequency, bandwith, and power of discovered signals, with timestamps. 

The prototype is directed toward facilities which use wireless sensing where unexpected improper operation may create a safety risk or cause economic loss. The intended user can be varied, including the followings:
* System integrators who design or implement a spectrum monitoring system
* Plant operators who are trying to understand implications as they apply a spectrum monitoring system to help mitigate the impacts to business operations
* Information Technology (IT) and Operational Technology (OT) security officers interested in monitoring electromagnetic spectrum for intrusions and anomalies
* Device manufacturers developing products which will be deployed as part of a spectrum monitoring system

The low-cost hardware platform chosen for this prototype was HackRF One which has its own limitations, such as fairly poor sensitivity. Further steps may be taken by considering different low-cost hardware platforms, such as SDRplay and Airspy, which may be better fit for specific industrial spectrum monitoring applications. 

It is recommended getting started by watching video tutorials on [Software Defined Radio with HackRF](http://greatscottgadgets.com/sdr/) which will be introducing users to HackRF One and software defined radio (SDR) with the GNU Radio software development toolkit.  


## Installation

### Ubuntu
The ISMS has been tested on Ubuntu Linux 14.04 LTS. Installation instructions for Ubuntu can be found on [help.ubuntu.com/14.04/installation-guide](https://help.ubuntu.com/14.04/installation-guide/).

### GNU Radio
The installation instructions for GNU Radio software development toolkit providing digital signal processing blocks to design software-defined radios can be found on [Installing GNU Radio](https://github.com/mossmann/hackrf/wiki/Operating-System-Tips).

### HackRF One
The hardware specs for the low-cost platform HackRF One can be found on [HackRF One Specs](https://github.com/mossmann/hackrf/wiki/HackRF-One). 

The installation instructions for HackRF One can be found on [Installing HackRF Tools](https://github.com/mossmann/hackrf/wiki/Operating-System-Tips).

[Where to Buy](http://greatscottgadgets.com/wheretobuy/) will provide the user with information on how to get HackRF One and [This Video Tutorial](http://greatscottgadgets.com/sdr/5/) is especially essential to understand the firmware configuration of this specific hardware. 

## Running the ISMS
The software consists of two parts: radio scanner (gr-scan-monitor) and signal monitor (hackrf-monitor). Scanner is a modified version of gr-scan tool and signal monitor is an independent project. Both packages can be built using make command inside corresponding directory.

In order to run the system, the user needs to specify scanning parameters and run monitor that will display scan results. Scanner and monitor are independent. Scanner can be stopped and launched with different set of parameters without closing monitor.
The only required scan parameters are start and end frequencies. For instance, the following command should be used for 2.4 GHz ISM band :
```
./gr-scan -x 2400 -y 2500
```
Options are: 

```
-x <starting frequency in MHz>
-y <ending frequency in MHz>
```

Other available options include:

```
-a <N> - average over N FFT samples at each frequency, reasonable range is 20-2000
-r <N> - set sample rate of N Msamples/s (HackRF One is capable of up to 20 Msps)
-w <W> - set FFT width to W points
-z <S> - set frequency step of S MHz
-G <G> - set total gain of G dB (if greater than 0, this parameter overrides individual three gains)
-i <G> - set IF gain to G dB (for HackRF One, valid range is 0-40 dB with 8 dB steps)
-t <G> - set antenna gain to G dB (for HackRF One, valid values are 0 or 14 dB)
-g <G> - set baseband gain to G dB (for HackRF One, valid range is 0-62 dB with 2 dB steps)
-A <a> - turn AGC on/off (1 and 0 correspondingly), when turned on, AGC overrides IF and antenna gains
```
When scanner is launched, the user can run the monitor in another terminal with the following command:
```
./sdr_processor 
```
The monitor has no command line parameters. In graphical interface, there are two charts: full range chart with frequencies from 100 to 6000 MHz and zoom window with adjustable range. Zoom window is centered with mouse: when cursor is in bottom third of the screen, zoom window central frequency corresponds to full range frequency which is under mouse cursor.

Zoom window frequency range is adjusted with mouse scroll wheel. When left shift key is pressed, mouse wheel changes power scale of both charts. When left alt key is pressed, mouse wheel changes zero level of both charts.


### Sample Output

The followings are some sample screenshots during the continuous scan of the system for microwave oven and NIST Shops experiments: 

The ISMS monitor displaying microwave oven interfering signal at 2465 MHz along with WiFi signals:

![terminal output_1](https://user-images.githubusercontent.com/19610600/30132362-1cccd1d2-931e-11e7-9a19-d70d5f8b4d49.jpg)

Spectrum analyzer confirming microwave oven interfering signal displayed by the monitor:

![speca](https://user-images.githubusercontent.com/19610600/30132491-7c90b386-931e-11e7-85c2-2c8151ac362f.jpg)






# Principles of Operation
The SMS runs continously, measuring the power at samples in a small frequency range over a short period of time, and computing a fast Fourier transform (FFT) to identify frequency components present. 
(Detail the operation of the SMS, including what is configurable)
* start frequency
* stop frequency
* step size
* scan bandwidth)
* dwell time
* FFT size
## Calibration
(How to calibrate it, with indications to what command-line options or source code variables to set)
## Thresholding
(How thresholding is done to miniimze false alarms, and how it is configured)
## Adataptive Gain Control
(How AGC is done)

# Limitations and Future Work
(Here is where you would show how the SMS would figure as a node in a larger system; what other hardware could be used)
