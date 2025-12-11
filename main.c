#include <stdio.h>           
#include <stdlib.h>          
#include "rs232.h"           
#include "serial.h"          

#define bdrate 115200        // Define the baud rate for serial communication 

// Define a structure to hold stroke data for a single character
typedef struct {
    int   ascii;             // ASCII code for this character
    int   nMoves;            // Number of stroke points (coordinates) for this character
    float *X;                // Dynamically allocated array of X coordinates for each stroke point
    float *Y;                // Dynamically allocated array of Y coordinates for each stroke point
    int   *Z;                // Dynamically allocated array of pen states (0 = pen up, 1 = pen down)
} StrokeData;

// Function prototypes:
int TexttoWordArray(FILE *file, char *word_buffer, int maxLengthWord);
int WordArraytoASCII(const char *word, int *TextToAscii, int maxLengthASCII);
int ExtractStrokeData(const int *TextToAscii, int len, FILE *stroke_data, StrokeData *chars, int maxChars);
void ScaleandAdjustStrokeData(StrokeData *chars, int nChars, float FontSize, float *curX, float *curY, float maxWidth, float maxHeight);
void ConvertStrokestoGcode(StrokeData *chars, int nChars, char *buffer);
void FreeStrokeData(StrokeData *stroke);

// Function prototype: sends one G-code string in buffer to the robot
void SendCommands(char *buffer);

int main(void)                        
{
    FILE *user_text = fopen("InputText.txt", "r");   // Open the user input text file in read mode
    if (user_text == NULL)                           // Check if the file failed to open
    {
        printf("Could not open InputText.txt\n");    // Print error message if file not found or inaccessible
        return 1;                                    // Exit program with error status code 1
    }

    FILE *stroke_data = fopen("SingleStrokeFont.txt", "r"); // Open the font stroke data file in read mode
    if (stroke_data == NULL)                                 // Check if the font file failed to open
    {
        printf("Could not open SingleStrokeFont.txt\n");     // Print error message
        fclose(user_text);                                   // Close the user text file before exiting
        return 1;                                            // Exit program with error status code 1
    }

    float FontSize;                                          // Variable to store user-selected font height in mm
    printf("Enter font height in mm (4-10): ");              // Prompt the user for a font height between 4 and 10 mm
    if (scanf("%f", &FontSize) != 1)                         // Read the input
    {
        printf("Invalid font height input.\n");              // Print error if scanf fails to read a float
        fclose(user_text);                                   // Close user text file
        fclose(stroke_data);                                 // Close stroke data file
        return 1;                                            // Exit with error status code 1
    }
    if (FontSize < 4.0f)  FontSize = 4.0f;                   // Lower Limit for font height to minimum of 4 mm
    if (FontSize > 10.0f) FontSize = 10.0f;                  // Upper Limit for font height to maximum of 10 mm

    char word[64];                                           // Buffer to hold a single word read from the text file
    int  TextToAscii[64];                                    // Array to hold ASCII codes for characters in the word
    int  word_count = 0;                                     // Counter to track how many words have been processed 
    
    float curX = 0.0f;                                       // Current X position (in mm) for placing the next character or word
    float curY = 0.0f;                                       // Current baseline Y position (in mm) for text
    float letterSpacing = FontSize * 0.15f;                  // Letter spacing (15% of font size)
    float wordSpacing   = FontSize * 0.8f;                   // Word spacing (80%) to be larger than letter spacing
    char buffer[100];                                        // Character buffer used to format and send G-code strings

    printf("Letter spacing: %.1fmm | Word spacing: %.1fmm\n", // Print computed spacing values for code checking (not necessary)
           letterSpacing, wordSpacing);

    if (CanRS232PortBeOpened() == -1)                        // Attempt to open the serial COM port 
    {
        printf("Unable to open COM port\n");                 // Print error if COM port cannot be opened
        fclose(user_text);                                   // Close user text file
        fclose(stroke_data);                                 // Close stroke data file
        exit(0);                                             // Exit the program immediately
    }

    printf("\nAbout to wake up the robot\n");                // Inform the user that the wake-up sequence is starting
    sprintf(buffer, "\n");                                   // Put a newline character into the buffer (wake-up signal)
    PrintBuffer(&buffer[0]);                                 // Send the newline over serial using provided function
    Sleep(100);                                              // Wait 100 ms to allow the robot to process the wake-up signal
    WaitForDollar();                                         // Block until a '$' character is received from the robot
    printf("\nThe robot is now ready to draw\n");            // Inform user that robot is ready to receive G-code

    sprintf(buffer, "G1 X0 Y0 F1000\n");                     // Prepare G-code to move to (0,0) with feedrate 1000
    SendCommands(buffer);                                    // Send this G-code line to the robot

    sprintf(buffer, "M3\n");                                 // Prepare G-code M3 (pen enable command)
    SendCommands(buffer);                                    // Send the M3 command to the robot

    sprintf(buffer, "S0\n");                                 // Prepare G-code S0 (set pen to pen up position)
    SendCommands(buffer);                                    // Send the S0 command to the robot

    while (TexttoWordArray(user_text, word, sizeof(word)))  // Loop while a new word is read from the input text file
    {
        word_count++;                                       // Increment the word counter for each successfully read word
        if (word[0] == '\0')                                // Check if the word buffer is empty 
        {
            printf("Error: Word buffer is empty. Check if text file has words\n"); // Print error message if buffer is empty
            break;                                          // Exit the word-processing loop
        }

        StrokeData chars[64];                               // Array of StrokeData structures to hold strokes for up to 64 characters in this word

        int len = WordArraytoASCII(word, TextToAscii, 64);  // Convert the current word into ASCII codes, store count in len

        int valid = 1;                                      // Indicate if ASCII conversion matches original characters
        for (int j = 0; j < len; j++)                       // Loop over each character index in the word
        {
            if ((char)TextToAscii[j] != word[j])            // Compare original character with converted ASCII back to char
            {
                valid = 0;                                  // Mark as invalid if there is a mismatch
                break;                                      // Stop checking further characters
            }
        }
        if (!valid)                                         // If ASCII conversion was invalid
        {
            printf("ASCII conversion failed for: %s\n", word); // Print error with the problematic word
            break;                                          // Exit the processing loop
        }

        int nChars = ExtractStrokeData(TextToAscii, len, stroke_data, chars, 64); // Load stroke data for each ASCII code into chars array
        if (nChars < 0)                                     // Check if stroke loading failed
        {
            printf("Stroke data missing for: %s\n", word);  // Inform user that stroke data was not found 
            break;                                          // Exit the processing loop
        }

        ScaleandAdjustStrokeData(chars, nChars, FontSize, &curX, &curY, 100.0f, 50.0f); // Scale and position strokes within 100x50 mm area

        ConvertStrokestoGcode(chars, nChars, buffer);                   // Convert positioned stroke data into G-code and send to robot

        curX += wordSpacing;                                // After finishing this word, advance X position by adding wordspacing value

        if (curX > 95.0f)                                   // If current X exceeds 95 mm (getting close to end of allowed space)
        {
            printf("Word wrap at X=%.1fmm\n", curX);        // Print message indicating text wrapping will happen
            curX = 0.0f;                                    // Reset X position to the left
            curY -= (FontSize + 5.0f);                      // Move Y down by font height plus 5 mm line spacing
        }

        for (int i = 0; i < nChars; i++)                    // Loop over each StrokeData structure in chars array
        {
            FreeStrokeData(&chars[i]);                      // Free dynamically allocated memory (X, Y, Z arrays) for each character
        }
    }

    sprintf(buffer, "S0\n");                                // Final S0 command to ensure pen is up at the end
    SendCommands(buffer);                                   // Send the final S0 command to the robot

    printf("\nDrew %d words | Final position: X=%.1f Y=%.1f\n", // Print summary of drawing operation (not necessary just for clarity)
           word_count, curX, curY);

    fclose(user_text);                                      // Close the input text file
    fclose(stroke_data);                                    // Close the font data file

    CloseRS232Port();                                       // Close the serial COM port
    printf("Com port closed\n");                            // Confirm to the user that the COM port has been closed
    return 0;                                               // Return 0 to indicate successful program termination
}

// Function to send one G-code command to the robot
void SendCommands(char *buffer)
{
    PrintBuffer(&buffer[0]);                                // Use provided PrintBuffer to send the string over serial
    WaitForReply();                                         // Block until the robot acknowledges the command
    Sleep(100);                                             // Wait for 100 ms to give robot time before next command
}
