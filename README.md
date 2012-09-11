xcj_swarm
=========

Xinchejian Swarm Robot project

Introduction:

Build a swarm of robots with autonomus behavior to achieve their goals by interacting with each other using IR communication.

Requirements:

Each basic robot contains the following:
	
	- 1x microcontroller Attiny2313
	- 1x motor controller L293D
	- 2x gear motors
	- 1x LiPo battery 3.7V
	Sensor shield

A custom made PCB will be used. This PCB will be mounted into a 3D printed casing which contains the gear motors and the wheels as well the IR sensor array.

Goal is that the maximum price for the basic robot should not exceed 5US$ (without shield)


Each robot will have a basic programming and basic hardware platform. 
The basic programming is:

1. Self protection: 
	- check your battery status
	- find the charging station when battery runs low
	- avoid obstacles and other robots
	- search for power (sunlight etc.)
2. Community (swarm) needs:
	- search for building material
	- search for dead robots
	- bring dead robots to the charging station
	- help dead robots with a charger robot
	- tell others where to find power (sunlight)
	- tell others where is an obstacle or danger

Folder structure:

	/kicad 		AtTin2313 - kicad schematics and PCB design files
	/source		AtTin2313 - source code

	/ATMEGA328	version based on 
	/UNO		version built onto and existing Arduino UNO.

	Note there is a LARGE amount of direct compatability between the code for ALL versions. Differences to note are:
		- different IO pins - see "Arduino, AtTiny, L293 and header pin mappings.ods"
		- size of memory limits size of code
		- AtTiny versions have fewer IO commands available. See http://wiki.xinchejian.com/wiki/Programming-AFRON


More at the XinCheJian website wiki.xinchejian.com 
	- the original Swarmrobot pages at http://wiki.xinchejian.com/wiki/Swarm_robots
	- collated summary and AFRON competition entry
		 - URL not yet published, but after 15/9/2012 you will find it via 
				http://wiki.xinchejian.com/wiki/Category:AFRON_$10_Robot_Competition


