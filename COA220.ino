#include <Wire.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

//define colours
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

boolean DEV_MODE = true; // this should be false when the product is not in development

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
uint8_t buttons;

class Button {  // a class for containing button methods
  public:
    int character;
    uint8_t button;
    String buttonName;

    Button (uint8_t b, String n) {
      this->button = b;
      this->buttonName = n;
    }
    Button() {

    }

    void setChar(int charValue, byte charByte[8]) {
      lcd.createChar(charValue, charByte);
      this->character = charValue;  // this will be the integer that the custom character will be stored in for the lcd library
    }

    boolean pressed() { // returns true if the button is pressed
      if (button & buttons) {
        return true; // if the button is pressed return true
      }
      else {
        return false; // if the button is not pressed return false
      }
    }
};

struct leaderboardPosition {
  char username[4];
  int score;
};

// Custom Char bytes for arrows

byte upArrow[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00000
};

byte downArrow[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
  B00000
};

byte rightArrow[] = {
  B00000,
  B00100,
  B00110,
  B11111,
  B11111,
  B00110,
  B00100,
  B00000
};

byte leftArrow[] = {
  B00000,
  B00100,
  B01100,
  B11111,
  B11111,
  B01100,
  B00100,
  B00000
};

byte underscore[] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

Button leftButton (BUTTON_LEFT, "left");
Button rightButton (BUTTON_RIGHT, "right");
Button upButton (BUTTON_UP, "up");
Button downButton (BUTTON_DOWN, "down");

Button* gameButtons[] = {&leftButton, &rightButton, &upButton, &downButton};  // array of pointers to
Button* seq[10]; // an array that points to different buttons in the gameButtons array

int startSeqLength = 2;  // the default value for the starting sequence length
int guessTime = 2; // how many seconds the user has to make a guess (if = 0 the timer is disabled)
int m = 2;  // how many different buttons can be used in the sequence
int D = 1000; // the amount of milliseconds the user has to remember the sequence
int score = 0;

int seqLength;  // how many moves the sequence is
int seqPosition = 0;  // this is the index that the player is currently trying to remember

const int leaderboardSize = 10;
leaderboardPosition leaderboard[leaderboardSize];

const char firstAlphabetLetter = 65; // Capital A as ASCII respresentation
const char lastAlphabetLetter = 90; // Capital Z as ASCII respresentation

char username[4] = "AAA";
int leaderboardCursor = 0;

int correctAnswers = 0; // stores the users current amount of correct answers, resets when the games is over

unsigned long guessStart; // this will store the millis of a guess start
unsigned long currentTime; // this will store the current value of millis each loop
uint8_t guess;  // the users current guess

int leaderboardViewPos = 0;  // min 0 max 8, stores where the user is currently viewing the leaderboard at

String state;

void setRandomPattern() {
  for (int i = 0; i < seqLength; i++) { // loop through the sequence to set each value
    int randomNumber = random(0, m); // doesnt include the m value
    Button* randomButton = gameButtons[randomNumber]; // a pointer to a random button from the gameButtons array
    seq[i] = randomButton;
  }
}

void showSequence() {
  lcd.clear();
  lcd.home();
  for (int i = 0; i < seqLength; i++) {
    lcd.write(seq[i]->character);
    if (DEV_MODE) {
      Serial.print(seq[i]->buttonName + " ");
    }
  }
  if (DEV_MODE) {
    Serial.println();
  }
  delay(D);
  lcd.clear();
}

boolean makeGuess() { // SOMETHING ABOUT THESE INDEXES BEING ONE TOO FAR TO THE RIGHT
  if (seq[seqPosition]->button & guess) { // if the guess is correct
    guess = 0;
    lcd.setCursor(seqPosition, 0);
    lcd.write(seq[seqPosition]->character);
    return true;
  }
  else {
    guess = 0;
    return false;
  }
}

void resetGame() {
  seqLength = startSeqLength;
  seqPosition = 0;
  m = 2;
}

void levelUp() {
  seqPosition = 0;
  correctAnswers++;
  if (seqLength + 1 <= 10) { // ensure that the sequence length does not exceed 10
    seqLength++;
  }
  lcd.setBacklight(GREEN);
  lcd.clear();
  lcd.home();
  lcd.print("LEVEL UP");
  delay(D);
  lcd.setBacklight(WHITE);
  if (correctAnswers % 3 == 0 && (m + 1 <= 4)) { // every 3 correct answers another characer will be added to set of characters that can be used for the sequence
    m++;
  }
}

void updateLeaderboardView(){
  lcd.clear();
  for (int i = leaderboardViewPos; i < leaderboardViewPos + 2; i++){
    lcd.setCursor(0, i - leaderboardViewPos);
    String toPrint = "#"+String(i+1)+" ";
    toPrint += String(leaderboard[i].username) + ": ";
    toPrint += String(leaderboard[i].score) + " ";
    lcd.print(toPrint);
  }
  if (leaderboardViewPos != 0){
    lcd.setCursor(14,0);
    lcd.write(upButton.character);
  }
  if (leaderboardViewPos != 8){
    lcd.setCursor(14,1);
    lcd.write(downButton.character);
  }

  lcd.setCursor(15,0);
  lcd.write(rightButton.character);
}

void changeState(String newState) {
  if (state == newState) { // if the new state is the same as the old one then return straight away
    return;
  }

  else if (newState == "leaderboard_view"){
    state = newState;
    updateLeaderboardView();
  }

  else if(newState == "leaderboard_awaiting_viewchange_release"){
    state = newState;
  }
  
  else if (newState == "menu") {
    strcpy(username, "AAA"); // reset the username stored
    leaderboardCursor = 0;
    changeState("menu_set_length");
    correctAnswers = 0;  // reset the correct answers
    score = 0;
  }

  else if (newState == "menu_set_length") {
    leaderboardViewPos = 0; // reset the view for the leaderboard
    lcd.clear();
    lcd.home();
    lcd.write(leftButton.character);
    lcd.print("START LENGTH:");
    lcd.setCursor(15,0);
    lcd.write(rightButton.character);
    lcd.setCursor(7, 1);
    lcd.print(String(startSeqLength));
    state = newState;
    delay(300);  // if the user is holding the right button they will not have time to take their finger off the button to change the value in the next menu
  }

  else if (newState == "menu_set_guess_time") {
    lcd.clear();
    lcd.home();
    lcd.write(leftButton.character);
    lcd.print("TIMER:");
    lcd.setCursor(15,0);
    lcd.write(rightButton.character);
    lcd.setCursor(7, 1);
    lcd.print(String(guessTime));
    state = newState;
    delay(300); // if the user is holding the right button it would instantly send them to start the game
  }

  else if (newState == "menu_awaiting_length_release") {
    state = newState;
  }

  else if (newState == "menu_awaiting_guess_time_release") {
    state = newState;
  }

  else if (newState == "show_next_level") {
    state = newState;
  }

  else if (newState == "waiting_for_guess") {
    state = newState;
  }

  else if (newState == "waiting_for_button_release") {
    state = newState;
  }

  else if (newState == "leaderboard_username_entry"){
    lcd.clear();
    lcd.print(username);
    lcd.print(" HIGH SCORE");
    lcd.setCursor(15,0);
    lcd.write(rightButton.character);
    lcd.setCursor(leaderboardCursor, 1);
    lcd.write(4);
    state = newState;
  }

  else if (newState == "leaderboard_await_username_change_release"){
    state = newState;
  }

  else {
    reportUnknownState(newState);
  }
}

void reportUnknownState(String stateName) {
  Serial.println("Unknown state: " + stateName);
  lcd.clear();
  lcd.home();
  lcd.print("ERROR");
  lcd.setCursor(0, 1);
  lcd.print("Check Serial");
  exit(0);
}

void gameOver() {
  score = correctAnswers * startSeqLength;  // the larger the startSequence number the greater the score
  if (guessTime != 0) {
    score = (int) (score / (sqrt(guessTime)));  // divide the score by amount of time the user had available to guess then truncate it to keep int type (higher time gives a worse score)
  }
  else {
    score = 0;
  }
  lcd.clear();
  lcd.home();
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("SCORE: ");
  lcd.print(score);
  delay(D);
  lcd.setBacklight(WHITE);
  resetGame();  // reset all the values
  if (isHighScore(score)){
    changeState("leaderboard_username_entry");  // go back to the menu
  }
  else{
    changeState("menu");
  }
}

bool isHighScore(int score){
  if (score <= leaderboard[leaderboardSize-1].score){ // the users score is not high enough to be on the leaderboard
    return false;
  }
  else{
    return true;
  }
}

void addToLeaderboard(char username[4], int score){
  //username += '\0'; 
  leaderboardPosition newWinner;
  strcpy(newWinner.username, username);
  newWinner.score = score;

  leaderboardPosition newleaderboard[leaderboardSize];

  for (int i = 0; i < leaderboardSize; i++){  // This is approach is awful
    if (leaderboard[i].score > newWinner.score){
      newleaderboard[i] = leaderboard[i];
    }
    else if (newWinner.score > leaderboard[i].score){
      newleaderboard[i] = newWinner;
      for (int j = i+1; j < leaderboardSize; j++){
        newleaderboard[j] = leaderboard[j-1];
      }
      break;
    }
  }

  for(int i = 0; i < leaderboardSize; i++){ // find the position of the new leaderboard score
    leaderboard[i] = newleaderboard[i];
  } 
  
  updateleaderboardInEEPROM();
  
}

void updateleaderboardInEEPROM(){
  int eeAddress = 0;
  for (int i = 0; i < leaderboardSize; i++) {
    EEPROM.put(eeAddress, leaderboard[i]);
    eeAddress += sizeof(leaderboardPosition); // increase the address by the size of leaderboardPosition in bytes
  }
}

void resetLeaderboard() {
  int eeAddress = 0;
  for (int i = 0; i < leaderboardSize; i++) {
    EEPROM.put(eeAddress, leaderboardPosition{"NUL\0", 0});
    eeAddress += sizeof(leaderboardPosition); // increase the address by the size of leaderboardPosition in bytes
  }
  getLeaderboard();
}

void getLeaderboard() {
  int eeAddress = 0;
  for (int i = 0; i < sizeof(leaderboard) / sizeof(leaderboard[0]); i++)
  {
    EEPROM.get(eeAddress, leaderboard[i]);

    eeAddress += sizeof(leaderboardPosition);
  }
  if (DEV_MODE){
    Serial.println("----- Current Leaderboard ----- ");
  
    for (int i = 0; i < leaderboardSize; i++) {
      Serial.print("#" + String(i) +" ");
      Serial.print(String(leaderboard[i].username));
      Serial.print(" has a score of ");
      Serial.println(String(leaderboard[i].score) + " ");
    }
  
    Serial.println("--------------------------------");
  }
  
}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  
  getLeaderboard();  // fetch the leaderboard from the EEPROM
  
  //Create custom chars for the LCD Screen

  leftButton.setChar(0, leftArrow);
  rightButton.setChar(1, rightArrow);
  upButton.setChar(2, upArrow);
  downButton.setChar(3, downArrow);
  lcd.createChar(4, underscore);

  seqLength = startSeqLength;
  changeState("menu");
}

void loop() {
  buttons = lcd.readButtons();
  currentTime = millis();
  if (state == "leaderboard_view"){
    if (downButton.pressed() && leaderboardViewPos < 8){
      leaderboardViewPos++;
      changeState("leaderboard_awaiting_viewchange_release");
    }
    if (upButton.pressed() && leaderboardViewPos > 0){
      leaderboardViewPos--;
      changeState("leaderboard_awaiting_viewchange_release");
    }
    if (rightButton.pressed()){
      changeState("menu");
    }
  }
  else if (state == "leaderboard_awaiting_viewchange_release"){
    if (buttons == 0){
      changeState("leaderboard_view");
    }
  }
  else if (state == "menu_set_length") {
    if (upButton.pressed() && startSeqLength < 5) {
      startSeqLength++;
      changeState("menu_awaiting_length_release");
    }
    else if (downButton.pressed() && startSeqLength > 1) {
      startSeqLength--;
      changeState("menu_awaiting_length_release");
    }
    else if (rightButton.pressed()) {
      seqLength = startSeqLength;
      changeState("menu_set_guess_time");
    }
    else if (leftButton.pressed()) {
      changeState("leaderboard_view");
    }
  }
  else if (state == "menu_awaiting_length_release") {
    if (upButton.pressed() && (buttons & BUTTON_SELECT)){
      resetLeaderboard();
    }
    if (buttons == 0) { // if the user has let go of the button
      lcd.setCursor(0, 1);
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print(String(startSeqLength));
      changeState("menu_set_length"); // change to the menu to change the timer length
    }
  }
  else if (state == "menu_set_guess_time") {
    if (upButton.pressed() && guessTime < 10) {
      guessTime++;
      changeState("menu_awaiting_guess_time_release");
    }
    else if (downButton.pressed() && guessTime > 0) {
      guessTime--;
      changeState("menu_awaiting_guess_time_release");
    }
    else if (rightButton.pressed()) {
      changeState("show_next_level"); // start the game
    }

    else if (leftButton.pressed()) {
      changeState("menu_set_length"); // go back to setting the start sequence length
    }
  }
  else if (state == "menu_awaiting_guess_time_release") {
    if (buttons == 0) { // if the user has let go of the button
      lcd.setCursor(0, 1);
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print(String(startSeqLength));
      changeState("menu_set_guess_time");
    }
  }
  else if (state == "show_next_level") {
    setRandomPattern();  // generate the first random patern
    showSequence();
    changeState("waiting_for_guess");
    guessStart = millis();  // start the timer
  }
  else if (state == "waiting_for_guess") {
    if (guessTime != 0) {
      lcd.setCursor(12, 1);
      lcd.print((( (float) guessStart - (float) currentTime) / 1000) + guessTime);
    }
    if (currentTime - guessStart >= ((guessTime) * 1000) && guessTime != 0) {
      gameOver();
    }
    if (buttons > 1) { // a button that isnt select has been pressed
      guess = buttons;
      changeState("waiting_for_button_release");
    }
  }
  else if (state == "waiting_for_button_release") {
    guessStart = millis();  // reset the timer
    if (buttons == 0) {
      if (makeGuess()) { // if the guess was correct
        seqPosition++;
        if (seqPosition > seqLength - 1) { // the user has entered all correctly, level up
          levelUp();
          changeState("show_next_level");
          return;
        }
      }
      else {
        gameOver();
        return;  // Stop the cycle to stop the state frmo being set to waiting for another guess
      }
      changeState("waiting_for_guess");
    }
  }

  else if (state == "leaderboard_username_entry"){
    if (downButton.pressed()){
      char newChar = username[leaderboardCursor]+1;
      if (newChar > lastAlphabetLetter){
        newChar = firstAlphabetLetter;
      }
      username[leaderboardCursor] =  newChar;
      changeState("leaderboard_await_username_change_release");
    }
    else if (upButton.pressed()){
      char newChar = username[leaderboardCursor]-1;
      if (newChar < firstAlphabetLetter){
        newChar = lastAlphabetLetter;
      }
      username[leaderboardCursor] =  newChar;
      changeState("leaderboard_await_username_change_release");
    }
    else if (rightButton.pressed() && leaderboardCursor < 2){
      leaderboardCursor++;
      changeState("leaderboard_await_username_change_release");
    }
    else if (rightButton.pressed() && leaderboardCursor == 2){
      addToLeaderboard(username, score);
      changeState("menu");
    }
    else if (leftButton.pressed() && leaderboardCursor > 0){
      leaderboardCursor--;
      changeState("leaderboard_await_username_change_release");
    }
  }
  else if (state == "leaderboard_await_username_change_release"){
    if (buttons == 0){
      changeState("leaderboard_username_entry");
    }
  }
  else {
    reportUnknownState(state);
  }
}
