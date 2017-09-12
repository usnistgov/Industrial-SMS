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
The software consists of two parts: radio scanner (gr-scan) and signal monitor (radio-scan-monitor). Scanner is a modified version of gr-scan tool and signal monitor is an independent project. Both packages can be built using make command inside corresponding directory.

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

The followings are some sample screenshots during the continuous scan of the system for microwave oven and the NIST Shops experiments. These experiments were conducted using the command line options below:

```
./gr-scan -x 2400 -y 2500 -r 20 -w 2048 -a 200 -z 0.1 -A 1 -G 0 -g 0 -i 0 -t 0
```
and

```
./gr-scan -x 600 -y 1100 -r 20 -w 2048 -a 200 -z 0.1 -A 1 -G 0 -g 0 -i 0 -t 0
```
Monitor and detector use the same units (dBm), but monitor shows peak amplitude (just like spectrum analyzer), while detector integrates signal over the entire bandwidth (as a result, more power) so it's only natural that numbers are different. For narrow peaks, monitor is displaying couple dB higher level than detector. For wideband, monitor is displaying much lower level than detector. For instance, it is reasonable if monitor seems like 10-20 dB less than detector during WiFi scanning. 

The ISMS monitor while microwave oven turned off:



The ISMS monitor displaying microwave oven interfering signal at 2465 MHz along with other signals at 2.4 GHz band:



Spectrum analyzer confirming microwave oven interfering signal displayed by the monitor:

![speca](https://user-images.githubusercontent.com/19610600/30132491-7c90b386-931e-11e7-85c2-2c8151ac362f.jpg)

The ISMS monitor displaying cellular signal during the NIST shops experiment:

![0831171112a](https://user-images.githubusercontent.com/19610600/30171879-1ff7ee66-93c1-11e7-893d-516bb112c189.jpg)




# Principles of Operation
The ISMS runs continuously, measuring the power at samples in a small frequency range over a short period of time, and computing a fast Fourier transform (FFT) to identify frequency components present. Some of the important command line configuration options are provided for the user below: 

* *start frequency*: The user can enter any starting frequency in MHz between 100 MHz and 6000 MHz                              
* *stop frequency*: The user can enter any ending frequency in MHz between 100 MHz and 6000 MHz                              
* *step size*: The user can enter frequency step size in MHz which was configured as 0.1 MHz for our experiments                     
* *scan bandwidth*: The user can enter 8 MS/s, 10 MS/s, 12.5 MS/s, 16 MS/s and 20 MS/s since using a sampling rate of less than 8 MHz is not recommended because of the fact that the MAX5864 (ADC/DAC chip) is not specified to operate at less than 8 MHz. Maximum option of 20 MS/s was utilized for our experiments                                                                                            
* *dwell time*: The user can configure this via -a option which is the average over N samples at each frequency step. The value 200 was configured as -a option which resulted 20 milliseconds for every 0.1 MHz frequency step in our experiments                        
* *FFT size*: The user can enter FFT points which was configured as 2048 for our experiments

## Calibration

The calibration experiment of the HackRF One device was conducted by fixing the center frequency and then injecting different levels of power (-80 dBm to -20 dBm by 10 dB increments) using vector signal generator. Because of the fact that HackRF One is not a linear SDR peripheral, there was a difference between injected and measured levels so there needed to be a coefficient introduced to compensate for difference between these two levels which can be located under gr-scan folder in scanner_sink.hpp at line 179.

## Thresholding
At each given frequency, the detector looks over its central window and calculates average power inside this window. The detector looks at signal levels in the left and right windows and average power in these windows are calculated as well which are treated as background level. Afterwards, the detector compares signal level in the center window with the signal levels in the left and right windows to figure out how much average power goes above background level. After comparison, If the signal level in the center lower than the signal levels in the left or right windows, this is treated as there is no signal detected. On the other hand, if the signal level in the center higher than the signal levels in both left and right windows, this is treated as there is signal detected. The higher the difference between the center window and background level, the more likely it is the center point as long as we have more equal sides.
After that, we seek for such point in our frequency range where this value is maximum and make an assumption that this point where there is the peak of detected signal is the signal center. Then, we start at this peak point and move left and right over frequencies looking at signal power. Once we see signal drops less than halfway between peak and noise level meaning that signal power goes below 0.7 of average value over the range of detected signal, we conclude that these points are BW edges. Then, we integrate signal power between these points.


## Adaptive Gain Control (AGC)
First, average spectral power in current FFT window (excluding edge points) and then average of all points above average (in case if most points are noise) are calculated. If this average is too low, we increase gain for one step (8 dB) and if it too high, we decrease gain for one step (8 dB). Then, we lock gain change for the next 1000 FFT samples. The reason for us to lock it is to prevent oscillations. We turn on RF gain first (and correspondingly turn it off last) and adjust IF gain only when RF gain is turned on.


# Future Work
It is vital to maintain situational awareness of the spectrum environment to ensure full-spectrum superiority. The ISMS operating on the basis of distributed RF spectrum sensor units would provide imperative data for frequency situational awareness by working together in a coordinated way to detect and confirm anomalies in the spectrum, measure the complete range of frequencies, and capture weak signals.

Since spectrum usage can differ by both time and location within an area, more measurements would be needed from different locations by deploying a network of monitoring sensor units which can autonomously execute their tasks to provide statistically valid data. These remotely deployed sensor units would be connected over transmission control protocol/internet protocol (TCP/IP) network, including cable, fiber, and cellular. TCP would be used for administrative tasks, such as configuration and monitoring of individual sensor units, that require reliability and compatibility.



