#include <stdio.h>           

// Function converts the word string into an array of ASCII decimal integers
// Inputs: word string, TextToAscii (destination array), maxLengthASCII (maximum array size)
// Returns: Number of characters successfully converted
int WordArraytoASCII(const char *word, int *TextToAscii, int maxLengthASCII)
{
    int charPos = 0;             // Index position for reading characters from word and writing to TextToAscii array

    while (word[charPos] != '\0' && charPos < maxLengthASCII)  // Continue while not at string end AND while array space remains
    {
        TextToAscii[charPos] = (unsigned char)word[charPos];   // Cast character to unsigned char (0-255 ASCII range), store decimal value
        charPos++;                                           // Advance to next character position in both source and destination arrays
    }

    return charPos;              // Return the count of characters successfully converted to ASCII values
}

