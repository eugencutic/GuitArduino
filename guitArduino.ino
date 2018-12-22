
#include "LedControl.h" 
#include <SimpleList.h>
#include "pitches.h"
#include <LiquidCrystal.h>

#define BUTTON_LEFT_PIN A3
#define BUTTON_MIDDLE_PIN A4
#define BUTTON_RIGHT_PIN A5
#define PIEZO_PIN 8
#define V0_PIN 9 // PWN in loc de POTENTIOMETRU

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
LedControl lc = LedControl(12, 11, 10, 1); //DIN, CLK, LOAD, No. DRIVER
 
// pin 12 is connected to the MAX7219 pin 1
// pin 11 is connected to the CLK pin 13
// pin 10 is connected to LOAD pin 12
// 1 as we are only using 1 MAX7219

//Controls the visual representation of one Point
struct Point
{
  int currX = 0;
  int currY = 0;
  
  void moveX(int x)
  {
    currX = x;
  }
  void moveY(int y)
  {
    currY = y;
  }
  void show()
  {
    lc.setLed(0, currY, currX, true);
  }
  void hide()
  {
    lc.setLed(0, currY, currX, false);   
  }
  
};

//Controls the visual representation of one Note
struct Note
{
   Point left;
   Point right;

   int moveDown()
   {
      left.hide();
      right.hide();
      int valLeftY = left.currY + 1;
      int valRightY = right.currY + 1;
      if(valLeftY == 8 && valRightY == 8)
      {
        hide();
        return -1;
      }
        
      left.moveY(valLeftY);
      right.moveY(valRightY);
      
      left.show();
      right.show();
      return 0;
   }

   void hide()
   {
      left.hide();
      right.hide();
   }
   void create(int col)
   {
      left.moveX(col*3);
      left.moveY(0);
      left.show();
      
      right.moveX(col*3 + 1);
      right.moveY(0);
      right.show();
   }
};

//Struct to iterate through the melody's notes
struct Song
{
  int melody[16] = {
  NOTE_E4, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_D4, NOTE_FS4, NOTE_A4,
  NOTE_A4, NOTE_G4, NOTE_D5, NOTE_G4, NOTE_B4, NOTE_B3, NOTE_DS4,
  NOTE_A4, NOTE_A4
  };

  int index = 0;
  int melodySize = 16;

  void playNextNote()
  {
    tone(PIEZO_PIN, melody[index], 200);
    index = (index + 1) % melodySize;
  }
  
} song;


//Struct which controls notes movement, their sync with user input,
//as well as sound, using the Song struct
struct NotesManager
{
  SimpleList<Note> noteColumns[3];

  int buttonStates[3];
  int lastButtonStates[3] = {LOW, LOW, LOW};
  int lastDebounceTimes[3] = {0, 0, 0};
  int debounceDelay = 50;

  int score = 5;
  int lives = 16;

  void reset()
  {
    for (int i = 0; i < 3; i++)
    {
      while (!noteColumns[i].empty())
        noteColumns[i].pop_back();
    }
    score = 5;
    lives = 16;
  }

  void addNewNote(int col)
  {
      Note newNote;
      newNote.create(col);
      noteColumns[col].push_front(newNote);
      moveAllDown();    
  }

  void moveAllDown()
  {
      for(auto& col: noteColumns)
      {
          for(auto& note: col)
          {
              if(note.moveDown() != 0)
                col.pop_back();
          }
      }
  }

  void toggleNoteBar(bool on_off)
  {
      lc.setLed(0, 6, 2, on_off);
      lc.setLed(0, 6, 5, on_off);
  }

  //Reads button input and correlates it with note positions to update score
  //col: 0-2 value representing note columns, NOT actual matrix columns
  void readInputAndUpdateScore(int col)
  {
    int reading;
    bool toUpdate = false;
    switch (col)
    {
      case 0:
        {
          reading = digitalRead(BUTTON_LEFT_PIN);
          break;
        }
      case 1:
        {
          reading = digitalRead(BUTTON_MIDDLE_PIN);
          break;
        }
      case 2:
        {
          reading = digitalRead(BUTTON_RIGHT_PIN);
          break;
        }
    }

    if (reading != lastButtonStates[col])
    {
      lastDebounceTimes[col] = millis();
    }

    if ((millis() - lastDebounceTimes[col]) > debounceDelay)
    {
      if (reading != buttonStates[col])
      {
        buttonStates[col] = reading;

        if (buttonStates[col] == HIGH)
        {
          toUpdate = true;
        }
        if(buttonStates[col] == LOW)
        {
          toUpdate = false;
        }
          
      }
    }

    if (toUpdate)
      updateColumnScore(col);

    lastButtonStates[col] = reading;
  }

  //Correlates the press of one button with the corresponding notes column
  //and updates score accordingly:
  //+10 on bar; +5 above and underneath it; -5 anywhere else 
  void updateColumnScore(int col)
  {
    
    Note lastNoteOnColumn = *(noteColumns[col].end() - 1);
    if (lastNoteOnColumn.left.currY == 5)
    {
      score += 5;
      song.playNextNote();
      lastNoteOnColumn.hide();
      noteColumns[col].pop_back();
    }
    else if (lastNoteOnColumn.left.currY == 6)
    {
      score += 10;
      song.playNextNote();
      lastNoteOnColumn.hide();
      noteColumns[col].pop_back();
    }
    else if (lastNoteOnColumn.left.currY == 7)
    {
      score += 5;
      song.playNextNote();
      lastNoteOnColumn.hide();
      noteColumns[col].pop_back();
    }
    else
    {
      tone(PIEZO_PIN, NOTE_F2, 100);
      score -= 5;
      lives --;
    }
  }
}noteManager;

void setupLcdInfo()
{
  lcd.setCursor(0, 0);
  lcd.print("0");
  lcd.setCursor(0, 1);
  lcd.print("15");
  
  lcd.setCursor(11, 0);
  lcd.print("SCORE");

  lcd.setCursor(11, 1);
  lcd.print("LIVES");

}

void setup()
{
  //Serial.begin(9600);
  
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  
  randomSeed(analogRead(0));
  
  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUTTON_MIDDLE_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  
  lcd.begin(16, 2);
  lcd.clear();
 
  pinMode(V0_PIN, OUTPUT); 
  analogWrite(V0_PIN, 90);

  setupLcdInfo();
}

//this will decide whether to show the idle animation or the game on the matrix
bool playing = false;

unsigned long timeTurn1 = 0;
unsigned long timeTurn2 = 0;
unsigned long timeAnimation1 = 0;
unsigned long timeAnimation2 = 0;

int turn = 1;
int turnPeriod = 200;
int animationPeriod = 500;
int difficultyPeriod = 10000;
unsigned long difficultyTime = 0;


unsigned long scoreTime = 0;
int scorePeriod = 200;

//NOTE: the process goes as follows: a new note is created, all notes are moved down a line followed by a waiting period,
//      all notes are moved down again followed by another waiting period and repeat. 2 spaces are necessary between notes,
//      so that the game is playable, as well as the waiting period so that it gives the impression notes are falling 
//      sequentially, not 2 spaces at a time
void loop()
{
  //every scorePeriod ms stats are displayed on the LCD
  if(millis() > scoreTime + scorePeriod)
  {
    scoreTime = millis();
    showScoreAndLives();
  }
  
  if (playing)
  {
    //turn 1 creates a new note and pushes all down
    if (millis() > timeTurn1 + turnPeriod && turn == 1)
    {
      timeTurn1 = millis();
      
      //note creation:
      int col = random(0, 3);
      noteManager.addNewNote(col);
      turn = 2;
     
      //input is read and notes are moved down:
      nextTurn();
    }
   
    //wait for turnPeriod ms while aldo reading input
    while (millis() < timeTurn2 + turnPeriod)
    {
      noteManager.readInputAndUpdateScore(0);
      noteManager.readInputAndUpdateScore(1);
      noteManager.readInputAndUpdateScore(2);
    }
   
    //check if after reading input the player has lost:
    if (noteManager.lives <= 0)
    {
      //show game end notification
      youLose();
     
      playing = false;
     
      //reset stats displayed
      setupLcdInfo();
    }
    //if player didn't lose
    else
    {
      //turn 2 reads input and pushes notes down without creating a new one
      if (millis() > timeTurn2 + turnPeriod && turn == 2)
      {
        timeTurn2 = millis();
        turn = 1;
        nextTurn();
      }
      
      //check for lives again, as notes moved and input was read 
      if (noteManager.lives <= 0)
      {
        youLose();
        playing = false;
        setupLcdInfo();
      }
    }
    
    //wait for turnPeriod ms
    while (millis() < timeTurn2 + turnPeriod)
    {
      noteManager.readInputAndUpdateScore(0);
      noteManager.readInputAndUpdateScore(1);
      noteManager.readInputAndUpdateScore(2);
    }

    //every difficultyPeriod ms turnPeriod decreases, so notes will move faster, and the game will get harder
    if(millis() > difficultyTime + difficultyPeriod)
    {
      difficultyTime = millis();
      turnPeriod -= 10;
      
      //cap notes speed so the game doesn't become impossible
      if (turnPeriod < 100)
        turnPeriod = 100;
    }
  }
  else
  {
    //if the game hasn't been started, show an animation on the amtrix
    waitForStart();
  }
  
}

//function to print to lcd display
void showScoreAndLives()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(noteManager.score);
  lcd.setCursor(0, 1);
  lcd.print(noteManager.lives);
  
  lcd.setCursor(11, 0);
  lcd.print("SCORE");

  lcd.setCursor(11, 1);
  lcd.print("LIVES");
}

//function to display an X on the matrix and to play 3 descending chromatic notes
void youLose()
{
  lc.clearDisplay(0);
  for(int i = 0; i < 8; i++)
  {
    lc.setLed(0, i, i, true);
    lc.setLed(0, i, 7 - i, true);
  }
 
  tone(PIEZO_PIN, NOTE_E4, 300);
  delay(333);
  tone(PIEZO_PIN, NOTE_DS4, 300);
  delay(333);
  tone(PIEZO_PIN, NOTE_D4, 300);
  delay(333);
 
  lc.clearDisplay(0);
}

//reads input and moves all notes down one line
void nextTurn()
{
    
  noteManager.readInputAndUpdateScore(0);
  noteManager.readInputAndUpdateScore(1);
  noteManager.readInputAndUpdateScore(2);
  noteManager.moveAllDown();
  
}

//reads middle button, and gets game ready to play if the button was pressed
void getStartInput()
{
  int reading;

  reading = digitalRead(BUTTON_MIDDLE_PIN);
  if(reading == HIGH)
  {
    lc.clearDisplay(0);
    playing = true;
    timeTurn1 = millis();
    timeTurn2 = millis();
   
    //reset turnPeriod so the game starts at normal difficulty
    turnPeriod = 200;
   
    noteManager.toggleNoteBar(true);
    noteManager.reset();
    return;
  }
}


void waitForStart()
{
  //shows the bigger square on the matrix every animationPeriod ms
  if (millis() > timeAnimation1 + animationPeriod)
  {
    timeAnimation1 = millis();
    
    lc.clearDisplay(0);
    
    for(int i = 2; i <= 5; i++)
    {
      lc.setLed(0, 2, i, true);
      lc.setLed(0, 5, i, true);
    }
    for(int i = 2; i <= 5; i++)
    {
      lc.setLed(0, i, 2, true);
      lc.setLed(0, i, 5, true);
    }
  }
  
  //leaves the bigger square on the matrix for animationPeriod ms while reading input
  while (millis() < timeAnimation1 + animationPeriod)
  {
    //getStartInput() will change the bool variable 'playing' instead of returning, to avoid memory overhead
    if(!playing)
      getStartInput();
    else
      return;
  }
  
  //get the time when the smaller square is displayed
  timeAnimation2 = millis();
  
  //display the smaller square
  lc.clearDisplay(0);

  lc.setLed(0, 3, 3, true);
  lc.setLed(0, 3, 4, true);
  lc.setLed(0, 4, 3, true);
  lc.setLed(0, 4, 4, true);
  
  //repeat the process, but this time showing the smaller square on the matrix
  while (millis() < timeAnimation2 + animationPeriod)
  {
    if(!playing)
      getStartInput();
    else
      return;
  }
}
