xcj_swarm/AtTiny2313
=========

Part of Xinchejian Swarm Robot project - contains ALL software and hardware for AtTiny2313 SwarmRobots.

As there are now many different types of SwarmRobots, moving all AtTiny2313 files here.

hardware/swarmIRShield			Shield for IR follow/avoid, object detection and communications
hardware/mainPCB			Main processor & motor controller


software/IR_Follow			IR follow has bugs not working very well
software/IR_Follow_AFRON		IR follow released for AFRON competition + has bug fix
software/IR_object_detection		IR object detection
software/SwitchShield_Avoid_Follow  	Wall avoid or follow using micro-switch shield (only on proto-board - no PCB done yet!)
/software/SwarmIRControl		SwarmRobot(s) respond to RC5 IR comamnds
					Build from command line 'make' - so you need a compiler setup!
/software/SwarmIRControllerArduino	Send RC5 commands to SwarmRobot(s) using an Arduino 
						(most remotes in China are NEC protocol!)
/software/Swarm_Random_Demo		SwarmRobot moves in random direction for random time (6 seconds max) 
						- no shield required!

... to do for software/SwarmIRControl
	- add non-rc5 ie any IR follow to SwarmIRControl
	- more doco on compiling & settting up compiler....
		(if you have Arduino installed - you should have it already, maybe need path..)

************************************************************
Changes made to move from previous mess to this structure:-

/swarmIR		-> AtTiny2313/hardware/swarmIRShield
/kicad			-> AtTiny2313/hardware/mainPCB

/source-Avoid-Follow	-> AtTiny2313/software/SwitchShield_Avoid_Follow 
/source-IR		-> AtTiny2313/software/IR_Follow_AFRON
/swarm_wall_follow_ir	-> AtTiny2313/software/IR_object_detection

All the Arduino code files also renamed so can open sketches in the folder!

************************************************************

