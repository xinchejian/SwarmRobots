EESchema Schematic File Version 2  date Thu 09 Aug 2012 10:43:04 CST
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:tsop
EELAYER 43  0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "9 aug 2012"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L R R?
U 1 1 502322BF
P 2300 3850
F 0 "R?" V 2380 3850 50  0000 C CNN
F 1 "R" V 2300 3850 50  0000 C CNN
	1    2300 3850
	0    1    1    0   
$EndComp
$Comp
L LED D?
U 1 1 502322CE
P 2750 3850
F 0 "D?" H 2750 3950 50  0000 C CNN
F 1 "LED" H 2750 3750 50  0000 C CNN
	1    2750 3850
	1    0    0    -1  
$EndComp
$Comp
L TSOP1838 ???
U 1 1 502322DD
P 1750 4050
F 0 "???" H 1750 4050 60  0000 C CNN
F 1 "TSOP1838" H 1750 4050 60  0000 C CNN
	1    1750 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 3850 3100 3850
Wire Wire Line
	3100 3850 3100 4050
Wire Wire Line
	3100 4050 2050 4050
Text Notes 1250 3500 0    60   ~ 0
Resistor & LED provide optional visual output indication
Text Notes 1100 4550 0    60   ~ 0
Four repeats of circtui for front Left, Right, Center & Rear.
$EndSCHEMATC
