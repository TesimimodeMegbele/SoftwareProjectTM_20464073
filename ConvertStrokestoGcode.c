#include <stdio.h>           

// Data structure for character strokes
typedef struct {
    int   ascii;             // ASCII code of the character
    int   nMoves;            // Total number of stroke movements
    float *X;                // Array of scaled X coordinates
    float *Y;                // Array of scaled Y coordinates
    int   *Z;                // Array of pen states
} StrokeData;

// Function: sends formatted G-code string to robot (defined in main.c)
void SendCommands(char *buffer);

// Function: converts positioned stroke data into complete a G-code sequence for the robot to execute
// Processes every stroke point, generating pen up/down (S0/S1000) and movement (G0/G1) commands
// Inputs: chars array (scaled coordinates), nChars (character count), buffer (for sprintf formatting)
// No return value - sends commands immediately
void ConvertStrokestoGcode(StrokeData *chars, int nChars, char *buffer)
{
    int currentPenState = 0;     // Initialize assuming pen starts in UP position

    for (int charIdx = 0; charIdx < nChars; charIdx++)      // Iterate through every character in the word
    {
        for (int strokeIdx = 0; strokeIdx < chars[charIdx].nMoves; strokeIdx++)     // Check every individual stroke point within this character
        {
            float targetX = chars[charIdx].X[strokeIdx];     // Destination X position
            float targetY = chars[charIdx].Y[strokeIdx];     // Destination Y position 
            int   penState = chars[charIdx].Z[strokeIdx];    // Required pen state (0=up, 1=down)

            if (penState == 0)      //If pen up                           
            {
                if (currentPenState)
                {
                    sprintf(buffer, "S0\n");                 // S0 = pen up
                    SendCommands(buffer);                    // Transmit command to robot using function
                    currentPenState = 0;                     // Update internal state tracker
                }

                sprintf(buffer, "G0 X%.3f Y%.3f\n", targetX, targetY);  // G0 = linear move
                SendCommands(buffer);                        // Send positioning command
            }

            else        // Pen down state                                 
            {
                if (!currentPenState)
                {
                    sprintf(buffer, "S1000\n");              // S1000 = pen down
                    SendCommands(buffer);                    // Transmit to robot
                    currentPenState = 1;                     // Update internal state tracker
                }

                sprintf(buffer, "G1 X%.3f Y%.3f\n", targetX, targetY);  // G1 = linear move
                SendCommands(buffer);                        // Send drawing command
            }
        }
    }
    
    if (currentPenState)
    {
        sprintf(buffer, "S0\n");                         // Final pen up command ( for safety)
        SendCommands(buffer);                            // Transmit final safety command
    }
}

