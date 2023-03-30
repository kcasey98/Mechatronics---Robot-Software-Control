# Mechatronics---Robot-Software-Control

Project Overview (ME 210 Final Project)
Link to our website: https://me210lilbopeep.weebly.com

Problem statement:
Design an autonomous robot (software,hardware and electrical design) that picks a ping pong ball in a pickup zone and drops it off on the other side of the course. The robot must pick up and drop off as many balls as possible in two minutes. However, there will be anothe robot on the course and whoever gets more balls in the final zone wins. Your robot must be autonomous, but also capable of avoiding the other robot.

Description of our project:
This project was a culmination of not only mechatronics lessons learned from the ME210 class, but from past skills and knowledge in design, teamwork, and prototyping. Team members in this group succeeded because we had a great combination of technical skills, collaboration, and love for the project. All of us thoroughly enjoyed designing and manufacturing our robot and are proud of what we accomplished. Given more time, there is no doubt we would have improved on our current iteration, but team “Lil Bo Peep’s” robot is a shepherd unlike anything the world has ever seen! 

Control code (C++):
For our robot’s software, we drew inspiration from many of the examples and prior labs completed in this class, specifically Lab 0. We structured our code by outlining multiple different states and pairing said states with a global events function. The loop would iterate over a set of switch functions that change the movement direction and task state of our robot, while also checking for new global events that triggered. Global event triggers included our bumper sensors, various timers (loading timer during the loading process, backup timer for the reverse move after unloading, a stall timer during a cycle reset, and the game timer), and two switches which either determined the team color or a reset switch. These global events were paired with other path specific conditions which when satisfied switched the states of our robot. Movement states included the four cardinal directions as well as four diagonal directions. We also had task states to help identify the stage of the robot including dropoff, findloading, and gameover states. Additional parts of our code included functions to drive the motors in specific directions, code to read the state of the switches, and code to move the servo during the dropoff phase. The code is shared in the link below.
