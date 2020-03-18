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

int startSeqLength = 2;

int m = 2;  // how many different buttons can be used in the sequence
int seqLength;  // how many moves the sequence is
int seqPosition = 0;  // this is the index that the player is currently trying to remember
uint8_t guess;  // the users current guess

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
  Serial.println("showing new sequence");
  lcd.clear();
  lcd.home();
  for (int i = 0; i < seqLength; i++){
    lcd.write(seq[i]->character);
  }
  delay(showTime);
  lcd.clear();
}

boolean makeGuess(){
  if (seq[seqPosition]->button & guess){ // if the guess is the same as the element that the user is trying to guess
    guess = 0;
    return true;
  }
  else{
    guess = 0;
    return false;
  }
}

void resetGame(){
  int score = seqLength - 2;
  lcd.setBacklight(RED);
  lcd.clear();
  lcd.home();
  lcd.print("GAME OVER");
  lcd.setCursor(0,1);
  lcd.print("SCORE: ");
  lcd.print(score);
  delay(showTime);
  lcd.setBacklight(WHITE);
  seqLength = startSeqLength;
  seqPosition = 0;
  m = 2;
}

boolean levelUp(){
  lcd.setBacklight(GREEN);
  lcd.clear();
  lcd.home();
  lcd.print("LEVEL UP");
  delay(showTime);
  lcd.setBacklight(WHITE);
  seqLength++;
  if (seqLength % 3 == 0 && (m+1 <= 4)){
    m++;
  }
  seqPosition = 0;
  if (seqLength > 10){
    lcd.clear();
    lcd.home();
    lcd.print("YOU WIN");
    return false;
  }
  else{
    return true;
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

  state = "show_next_level";
  seqLength = startSeqLength;
}

void loop() {
  buttons = lcd.readButtons();
  if (state == "show_next_level"){
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
        seqPosition += 1;
        if (seqPosition > seqLength-1){ // the user has entered all correctly, level up
          if(levelUp()){
            state = "show_next_level"; 
            return;
          }
          else{
            state = "completed_game";
            return;
          }
        }
      }
      else{
        resetGame();
        state = "show_next_level";
        return;  // N
      }
      state = "waiting_for_guess";
    }
  }
  else{
    Serial.print("state: " + state);
  }
}
