#include <stdio.h>           
#include <ctype.h>           

// Function reads one word from input file
// Inputs: file pointer, destination buffer, maximum buffer size 
// Returns: 1 if word successfully read, 0 if end of file or error
int TexttoWordArray(FILE *file, char *word_buffer, int maxLengthWord)
{
    int inputChar;               // Stores the character read from file
    int writePos = 0;            // Current position in word_buffer where next character will be written

    if (file == NULL || word_buffer == NULL || maxLengthWord <= 1)  // Check for NULL pointers or if the buffer is too small
        return 0;                // Return failure if any input is invalid

    do {
        inputChar = fgetc(file); // Read one character from the file
        if (inputChar == EOF)    // Check if end of file reached (no more characters)
        {
            word_buffer[0] = '\0'; // Empty buffer
            return 0;            // Return failure (no word found, end of file)
        }
    } while (isspace(inputChar));

    word_buffer[writePos++] = (char)inputChar;  // Store character at current position, increment writePos

    while (writePos < maxLengthWord - 1)         // Continue while space remains
    {
        inputChar = fgetc(file);                 // Read next character from file
        if (inputChar == EOF || isspace(inputChar)) // Check for end of word
        {
            break;                               // Exit loop, word is complete
        }
        word_buffer[writePos++] = (char)inputChar; // Store character and advance write position
    }

    word_buffer[writePos] = '\0';                // Add string terminator at current write position

    return 1;                                    // Word successfully read
}
