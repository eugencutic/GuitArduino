# GuitArduino

  Description

  The game follows the guitar hero concept (ergo the name), dividing the matrix into 3 columns (each having a corresponding button) on which notes are constantly falling, and a marker, requring the player to hit the correct button when the note is on the marker. It follows the arcade style, lacking levels, while getting progressively more difficult. As time passes, notes are falling at a bigger speed rate, with a cap at ~200ms (i. e. one new note every ~200 ms). Also, the column for each new note is generated randomly.  
  
  Rules and score
  
  Score is updated only when input is read, therefore letting all notes pass will not affect the player stats. When a button is pressed the last note on the corresponding column is checked. Depending on its position stats can be updated as follows:
    i) score: +10 <-> note on the marker
    ii) score: +5 <-> note one line above or underneath the marker
    iii) score: -5 & lives: -1 <-> note anywhere else / no note on column
  The player starts on score 0 and 15 lives and the game comes to an end when they run out of lives. Overflowing aside, there is no cap on score, but it can also be negative.
  
  User interaction
  
  The matrix starts on a 2-piece animation which implies the press of a button to start.
  The game is started by pressing the middle button.
  Upon running out of lives, an X is shown on the matrix along with 3 descending chromatic notes to announce the end. Afterwards, the initial animation is shown again, and the user is expected to press the middle button again for another go.
  Feedback during playing:
    i) a note is played upon striking a note correctly (the game plays a melody if no notes are missed)
    ii) a disonant note is played when losing points
  
