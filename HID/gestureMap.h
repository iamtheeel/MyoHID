/*
 * Joshua Mehlman and James Swartz
 *
 * ENGR 845 Spring 2024
 * Term Project: HID
 *
 */

 /*
  *  Gesture mapping, settings, and commands
  */

// ******    Settings      ******
uint8_t CMD_MouseSpeedX = 5;
uint8_t CMD_MouseSpeedY = 5;



// ******   Commands       ******
// Mouse Move  
// Basics
#define mouse_xNon_yNon 0
#define mouse_xPos_yNon 1
#define mouse_xNeg_yNon 2
#define mouse_xNon_yPos 3
#define mouse_xNon_yNeg 4
// Diags
#define mouse_xPos_yPos 5
#define mouse_xNeg_yNeg 6
#define mouse_xNeg_yPos 7
#define mouse_xPos_yNeg 8
// Btn
#define mouse_clickL    11
#define mouse_clickR    12
#define mouse_clickM    14
// scroll wheel



/* Charictors
 * ASCII values
 * TAB = 9
 * LF = 10
 * CR = 13
 * dec 32 - 127
 */


// Initial state gestures
uint8_t gestMap[] = {mouse_xNon_yNon,                       // Stop = 1
                   mouse_xPos_yNon, mouse_xNeg_yNon,        // Mouse left = 2 right =3
                   mouse_xNon_yPos, mouse_xNon_yNeg,        // Mouse up down
                   mouse_clickL, mouse_clickR, mouse_clickM // Mouse Clicks
                  };


// Key press
