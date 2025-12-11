#include <stdio.h>           
#include <stdlib.h>         

// Structure to hold all stroke data required to draw one character
typedef struct {
    int   ascii;             // ASCII code value identifying which character this represents
    int   nMoves;            // Total number of stroke points needed to draw this character
    float *X;                // Pointer to dynamically allocated array of X coordinates for each stroke point
    float *Y;                // Pointer to dynamically allocated array of Y coordinates for each stroke point  
    int   *Z;                // Pointer to dynamically allocated array of pen states (pen up or down)
} StrokeData;

// Helper function: loads stroke data for exactly one character from font file
// Searches entire font file for matching ASCII header, reads all associated strokes
// Inputs: ASCII value to find, open font file pointer, destination StrokeData structure
// Returns: Number of strokes loaded when successful, -1 for failure

int LoadStrokeForChar(int asciiValue, FILE *fontFile, StrokeData *charData)
{
    // Local temporary structure to hold one stroke point (X,Y and Z) read from file
    struct {
        int X;               // Temporary X coordinate read from font file
        int Y;               // Temporary Y coordinate read from font file  
        int Z;               // Temporary pen state (0/1) read from font file
    } strokePoint;
    
    int moveIndex;           // Reads individual stroke points

    rewind(fontFile);        // Moves file position to start of file

    // Scan through entire font file looking for character header lines
    // fscanf returns number of successfully read items
    while (fscanf(fontFile, "%d %d %d", &strokePoint.X, &strokePoint.Y, &strokePoint.Z) == 3)
    {
        if (strokePoint.X == 999)                        // 999 marks a charachter
        {
            int foundAscii = strokePoint.Y;              // Extract ASCII code
            int moveCount = strokePoint.Z;               // Extract number of strokes

            if (foundAscii == asciiValue)               // Check if ASCII values match
            {
                charData->ascii = asciiValue;            // Store ASCII code in destination structure
                charData->nMoves = moveCount;            // Store stroke count in destination structure

                // Allocates dynamic memory for all stroke coordinate arrays
                charData->X = malloc((size_t)moveCount * sizeof(float));  // Array for X coordinates
                charData->Y = malloc((size_t)moveCount * sizeof(float));  // Array for Y coordinates
                charData->Z = malloc((size_t)moveCount * sizeof(int));    // Array for pen states

                // Check if any memory allocation failed
                if (!charData->X || !charData->Y || !charData->Z)
                {
                    return -1;                           // Return error code for memory allocation failure
                }

                // Reads the number of stroke lines from font file into allocated arrays
                for (moveIndex = 0; moveIndex < moveCount; moveIndex++)
                {
                    // Read one complete stroke point (X Y Z)
                    if (fscanf(fontFile, "%d %d %d", &strokePoint.X, &strokePoint.Y, &strokePoint.Z) != 3)
                    {
                        return -1;                       // Return error if incorrect format or missing data
                    }
                    // Convert integer coordinates from file to float and store in destination arrays
                    charData->X[moveIndex] = (float)strokePoint.X;  // Cast int to float for X coordinate
                    charData->Y[moveIndex] = (float)strokePoint.Y;  // Cast int to float for Y coordinate
                    charData->Z[moveIndex] = strokePoint.Z;         // Copy pen state (already int)
                }
                return moveCount;                        // Return number of strokes successfully loaded
            }
        }
    }
    return -1;                                       // Character ASCII value not found in entire font file
}

// Function: loads stroke data for one word into StrokeData array
// Iterates through ASCII array, calls LoadStrokeForChar for each character
// Inputs: array of ASCII codes, number of characters, font file pointer, destination array, max array size
// Returns: number of characters successfully loaded, -1 on any failure
int ExtractStrokeData(const int *TextToAscii, int len, FILE *stroke_data, StrokeData *chars, int maxChars)
{
    // Check that destination array has enough space
    if (len > maxChars)
    {
        return -1;                                   // Return error if array too small for all characters
    }

    for (int charIdx = 0; charIdx < len; charIdx++)  // Loop over each character in the word
    {
        int movesLoaded = LoadStrokeForChar(TextToAscii[charIdx], stroke_data, &chars[charIdx]);        // Load stroke data for this specific ASCII character into chars[charIdx]
        if (movesLoaded < 0)                         // Check if loading failed for this character
        {
            return -1;                               // Return error if any single character fails to load
        }
    }
    return len;                                      // All characters loaded successfully
}
