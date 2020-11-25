# Memory Game

A simple memory game for an arduino (Only tested on an UNO) with an [Adafruit RGB Shield](https://www.adafruit.com/product/714) attached.

This was my coursework for my Embedded Systems module at university and was my first time using C++.

I have used the [Adafruit RGB LCD Shield Library](https://github.com/adafruit/Adafruit-RGB-LCD-Shield-Library) to control the RGB Shield.

## Instructions

### Reset / start leaderboard 

(Leaderboard will not work correctly until this is done)

* If it is the first time launching the game on the device you will need to set the leaderboard. To do this you must:
1) Go to the 'Set length menu' 
2) Hold the Up button
3) Click the select button
* Now the leaderboard should be initialised and you can check this by pressing the left button to navigate to the leaderboard

### For normal use

* Use the up and down buttons to select the length of the first pattern
* Press right to go to the next menu
* Use the up and down buttons to select the amount seconds you will have to enter an arrow
* Observe and remember the pattern and wait for the pattern to disappear
* Press the arrows in order that they were displayed in the pattern
* As the game goes on the longer the length of the pattern will be and more types of buttons will be added to the pattern

If you enter an incorrect arrow the game will end, if your score was one of the top 10 recorded on the device then you will be prompted to enter a 3 digit long name

* Use the up and down keys to choose my initials and use the right and left keys to navigate the initial currently being changed
* Press the right button to save your username

### To view the leaderboard

* Go to the furthest left menu and use the up and down buttons to navigate the leaderboard
