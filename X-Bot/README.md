简介：
   X-Bot是一款群体机器人（Swarm Robot），让这样一群小巧的机器人来实现蚁群可以完成的任务，这就是群体机器人。它由来自新车间（xinchejian.com）创客空间成员组建的一个小组创建。该机器人是基于红外线传感而制作的，它的特点是低成本、易于实践和多功能。该机器人初级版在2012年9月 “AFRON $10 robot challenge” 上获得常规组的第二名。
   该项目有Lutz提议建立，于2012年4月启动，其目的是让成员通过实践可以学到多样的技能，并促进人之间的交流。该目的通过构建个体功能很简单，但通过它们之间的合作可以完成复杂任务的小机器人来实现，这就是Swarm Robot。
该项目在进行中，核心成员为：
   Lutz Michaelis： 德国 （发起人）
   Spencer F：澳大利亚
   Edward Jiang： 中国
   Sunny Sun ：中国
   Leo Yan ： 中国

   在这里归档是LeoYan在其基础上的进一步发展的中级版本，采用了ATMEGA328，拥有更大的创作空间，通过您的加入将会使其功能更完善，也将形成一个圈子，一起交流，一起make stuff。

版权声明：
    所有文档和硬件设计采用：CC BY-NC 3.0 CN ， see <http://creativecommons.net.cn/licenses>
    所有软件代码采用      ：GPL3.0 ，          see <http://www.gnu.org/licenses/>

# English

## Programming

* Checkout and compile the log
 * Set target to Arduino Uno
 * symlink source to SwarmRobot (required by Arduino IDE projects)
* Charge the X-Bot fully
* Configure your X-Bot ID in Swbot.Config.h
 * If you want to start _START_REMOTE_CONTROL
 * Select the behavior you want

```
  #define ROBOT_TYPE_ANT  0x10 (food hunter)
  #define ROBOT_TYPE_ANT_LINEUP 0x11 (line up to and follow the higher id)
  #define ROBOT_TYPE_FOOD 0x21 (turns your X-Bot into a edible source of food)
```
 * Verify the program (compilation step)
* Connect the USB TTL Programmer v2 to your USB
 * Align VCC to VCC
* Upload, should end up with Arduino 1.0.1:

```
avrdude done.  Thank you.
```

## Details

## Behavior changes

In Action.cpp, behavior is defined in RuleTbl as a state machine.

Below, an example using the food hunter behavior:

This represents the current state and the action to do (internal state):

    {INTERIORINFO, TASK_STAGE, TS_LOOKING,  0, 0, 0,  true},

The action is activated under each state by ANYINFO:

    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false },


(last boolean is used to create a linked-list of actions to be done following a state)

The first 3 parameters define the conditional for an action... In the case of ANYINFO, there's no conditional so parameters are zero:

    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},


If the type of message is FOOD, the action is to set the internal variable target to true and keep target id

    {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_FOOD,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},

If the TARGET variable is set, then change the state to TS_FOUND:

    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

