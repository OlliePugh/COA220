#include <Wire.h>
#include <TimerOne.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

boolean DEV_MODE= true; // this should be false when the product is not in development

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
uint8_t buttons;

class Button {  // a class for containing button methods
  public:
    int character;  
    uint8_t button;
    String buttonName;
    
    Button (uint8_t b, String n){
      this->button = b;
      this->buttonName = n;
    }
    Button(){
      
    }

    void setChar(int charValue, byte charByte[8]){
      lcd.createChar(charValue, charByte);
      this->character = charValue;  // this will be the integer that the custom character will be stored in for the lcd library
    }
  
    boolean pressed(){  // returns true if the button is pressed
      if (button & buttons){
        return true; // if the button is pressed return true
      }
      else{
        return false; // if the button is not pressed return false
      }
    }
};

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

Button leftButton (BUTTON_LEFT, "left");
Button rightButton (BUTTON_RIGHT, "right");
Button upButton (BUTTON_UP, "up");
Button downButton (BUTTON_DOWN, "down");

Button* gameButtons[] = {&leftButton, &rightButton, &upButton, &downButton};  // array of pointers to 
Button* seq[10]; // an array that points to different buttons in the gameButtons array

int startSeqLength = 10;

int m = 2;  // how many different buttons can be used in the sequence
int seqLength;  // how many moves the sequence is
int seqPosition = 0;  // this is the index that the player is currently trying to remember
uint8_t guess;  // the users current guess
uint8_t lastCyclePress;

int showTime = 1000;
String state;

void setRandomPattern(){
  for (int i = 0; i < seqLength; i++){  // loop through the sequence to set each value
    int randomNumber = random(0, m); // doesnt include the m value
    Button* randomButton= gameButtons[randomNumber]; // a pointer to a random button from the gameButtons array
    seq[i] = randomButton;
  }
}

void showSequence(){
  lcd.clear();
  lcd.home();
  for (int i = 0; i < seqLength; i++){
    lcd.write(seq[i]->character);
    if (DEV_MODE){
      Serial.print(seq[i]->buttonName + " ");
    }
  }
  if (DEV_MODE){
    Serial.println();  
  }
  delay(showTime);
  lcd.clear();
}

boolean makeGuess(){  // SOMETHING ABOUT THESE INDEXES BEING ONE TOO FAR TO THE RIGHT
  if (seq[seqPosition]->button & guess){ // if the guess is correct
    guess = 0;
    lcd.setCursor(seqPosition, 0);
    lcd.write(seq[seqPosition]->character);
    return true;
  }
  else{
    guess = 0;
    return false;
  }
}

void resetGame(){
  seqLength = startSeqLength;
  seqPosition = 0;
  m = 2;
}

boolean levelUp(){
  seqPosition = 0;
  if (++seqLength > 10){
    return false;
  }
  else{
    lcd.setBacklight(GREEN);
    lcd.clear();
    lcd.home();
    lcd.print("LEVEL UP");
    delay(showTime);
    lcd.setBacklight(WHITE);
    if (seqLength % 3 == 0 && (m+1 <= 4)){
      m++;
    }
    return true;
  }
}

void gotoMenu(){
  state = "menu_set_length";
  lcd.clear();
  lcd.home();
  lcd.print("START LENGTH:");
  lcd.setCursor(0,1);
  lcd.print(String(startSeqLength));
}

void win(){
  lcd.clear();
  lcd.home();
  lcd.print("YOU WIN");
  for (int i = 0; i < 3; i++){ // cycle through the colours
    lcd.setBacklight(RED);
    delay(50);
    lcd.setBacklight(YELLOW);
    delay(50);
    lcd.setBacklight(GREEN);
    delay(50);
    lcd.setBacklight(TEAL);
    delay(50);
    lcd.setBacklight(BLUE);
    delay(50);
    lcd.setBacklight(VIOLET);
    delay(50);
    lcd.setBacklight(WHITE); 
  }
}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);

  //Create custom chars for the LCD Screen
  
  leftButton.setChar(0, leftArrow); 
  rightButton.setChar(1, rightArrow);
  upButton.setChar(2, upArrow);
  downButton.setChar(3, downArrow);

  seqLength = startSeqLength;
  gotoMenu();
  
}

void loop() {
  buttons = lcd.readButtons();
  if (state == "menu_set_length"){
    if (upButton.pressed() && startSeqLength < 5){
      startSeqLength++;
      state = "menu_awaiting_length_release";
    }
    else if (downButton.pressed() && startSeqLength > 1){
      startSeqLength--;
      state = "menu_awaiting_length_release";
    }
    else if (rightButton.pressed()){
      seqLength = startSeqLength;
      state = "show_next_level";
    }
  }
  else if (state == "menu_awaiting_length_release"){
    if (buttons == 0){  // if the user has let go of the button
      lcd.setCursor(0,1);
      lcd.print("  ");
      lcd.setCursor(0,1);
      lcd.print(String(startSeqLength));
      state = "menu_set_length"; 
    }
  }
  else if (state == "show_next_level"){
    setRandomPattern();  // generate the first random patern
    showSequence();
    state = "waiting_for_guess";
  }
  else if (state == "waiting_for_guess"){
    if (buttons >1){ // a button that isnt select has been pressed
      guess = buttons;
      state = "waiting_for_button_release";
    }
  } 
  else if (state == "waiting_for_button_release"){
    if (buttons == 0){
      if (makeGuess()){ // if the guess was correct
        seqPosition++;
        if (seqPosition > seqLength-1){ // the user has entered all correctly, level up
          if(levelUp()){
            state = "show_next_level"; 
            return;
          }
          else{
            win();
            resetGame();
            gotoMenu();
            return;
          }
        }
      }
      else {
        Serial.println("finish");
        int score = seqLength - startSeqLength;
        lcd.setBacklight(RED);
        lcd.clear();
        lcd.home();
        lcd.print("GAME OVER");
        lcd.setCursor(0,1);
        lcd.print("SCORE: ");
        lcd.print(score);
        delay(showTime);
        lcd.setBacklight(WHITE);
        resetGame();
        gotoMenu();
        return;  // Stop the cycle to stop the state frmo being set to waiting for another guess 
      }
      state = "waiting_for_guess";
    }
  }
  else{
    Serial.println("Unknown state: " + state);
    lcd.clear();
    lcd.home();
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("Check Serial");
    exit(0); 
  }
}
