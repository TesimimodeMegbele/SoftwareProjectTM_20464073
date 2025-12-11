#include <stdio.h>           
#include <float.h>           
#include <math.h>            

// Data structure containing all information needed to draw a single character with pen strokes
typedef struct {
    int   ascii;             // ASCII decimal code identifying which character this data represents
    int   nMoves;            // Total count of stroke points (each point is an X,Y and Z coordinate)
    float *X;                // Pointer to array holding X coordinates of all stroke points
    float *Y;                // Pointer to array holding Y coordinates of all stroke points
    int   *Z;                // Pointer to array holding pen states for each stroke
} StrokeData;

// Function: scales font coordinates to physical size and adjusts the positions of characters correctly in the drawing area
// Performs word wrapping at maxWidth, next linee with 5mm spacing and vertical bounds checking at maxHeight
// Updates curX/curY pointers with final cursor position for next word placement
// Inputs: chars array (to transform), nChars (count), FontSize (mm), curX/curY (current position pointers), bounds

void ScaleandAdjustStrokeData(StrokeData *chars, int nChars, float FontSize, float *curX, float *curY, float maxWidth, float maxHeight)
{
    float scaleFactor = FontSize / 18.0f;        // Desired height (mm) divided by font's nominal unit height

    float xPosition = *curX;                     // Current horizontal position where next character starts (mm from left)
    float yBaseline = *curY;                     // Current vertical baseline position for text bottom alignment (mm from top)

    float lineSpacing = FontSize + 5.0f;         // Font height plus mandatory 5mm new line gap

    float wordMinX = INFINITY, wordMaxX = -INFINITY;  // Initialize extremes for word boundary (plus miuns infinity)
    for (int charIdx = 0; charIdx < nChars; charIdx++) // Iterate through every character in this word
    {
        for (int moveIdx = 0; moveIdx < chars[charIdx].nMoves; moveIdx++)       // Check every stroke point of every character to compute X boundaries
        {
            if (chars[charIdx].X[moveIdx] < wordMinX) wordMinX = chars[charIdx].X[moveIdx];
            if (chars[charIdx].X[moveIdx] > wordMaxX) wordMaxX = chars[charIdx].X[moveIdx];
        }
    }

    float totalWordWidth = (wordMaxX - wordMinX) * scaleFactor;  // Scales word width

    if (xPosition + totalWordWidth > maxWidth)          // Check if placing this entire word would exceed right margin boundary
    {
        xPosition = 0.0f;                        // Triggers word wrapping - reset to left margin (X=0)
        yBaseline -= lineSpacing;                // Move baseline downward by one line height
    }

    if (-yBaseline > maxHeight)         // Check if new baseline position still fits within vertical drawing boundary
    {
        return;                                  // No more vertical space - exit without drawing this word
    }

    for (int charIdx = 0; charIdx < nChars; charIdx++)
    {
        float charMinX = INFINITY, charMaxX = -INFINITY;        // Bounds for this specific character only
        
        for (int moveIdx = 0; moveIdx < chars[charIdx].nMoves; moveIdx++)       // Scan all stroke points to find this character's unscaled width boundaries
        {
            if (chars[charIdx].X[moveIdx] < charMinX) charMinX = chars[charIdx].X[moveIdx];     // Find leftmost point of this character (minimum X)
            if (chars[charIdx].X[moveIdx] > charMaxX) charMaxX = chars[charIdx].X[moveIdx];     // Find rightmost point of this character (maximum X)
        }
        
        float charWidth = (charMaxX - charMinX) * scaleFactor;      // Compute physical width of character after scaling
        float nextCharOffset = charWidth + (FontSize * 0.15f);   // Letter spacing = 15% of font height

        for (int moveIdx = 0; moveIdx < chars[charIdx].nMoves; moveIdx++)
        {
            float relativeX = chars[charIdx].X[moveIdx] - charMinX;
            float relativeY = chars[charIdx].Y[moveIdx];

            float globalX = xPosition + (relativeX * scaleFactor);  // Current position + scaled relative offset
            float globalY = yBaseline + (relativeY * scaleFactor);  // Baseline + scaled relative height

            chars[charIdx].X[moveIdx] = globalX;                 // Update X array element
            chars[charIdx].Y[moveIdx] = globalY;                 // Update Y array element
        }
        
        xPosition += nextCharOffset;        // Advance cursor for positioning next character
    }
    
    *curX = xPosition;                               // Update X cursor position
    *curY = yBaseline;                               // Update Y baseline position
}