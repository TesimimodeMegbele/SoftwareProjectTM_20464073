#include <stdlib.h>          

// Data structure definition for character stroke data
typedef struct {
    int   ascii;             
    int   nMoves;            
    float *X;                
    float *Y;                
    int   *Z;                
} StrokeData;

// Memory cleanup function: safely deallocates all dynamic arrays in a StrokeData structure
// Called after each word is drawn to reclaim memory for next word processing
// Input: pointer to StrokeData structure (single character stroke data)
// No return value
void FreeStrokeData(StrokeData *stroke)
{
    if (stroke == NULL) return;                 // Exit immediately if there is nothing to free

    free(stroke->X);                             // Release X coordinate array memory 
    free(stroke->Y);                             // Release Y coordinate array memory 
    free(stroke->Z);                             // Release pen state array memory 

    stroke->X = NULL;                            // Mark X array pointer as invalid (no longer points to valid memory)
    stroke->Y = NULL;                            // Mark Y array pointer as invalid
    stroke->Z = NULL;                            // Mark Z array pointer as invalid
    
    stroke->nMoves = 0;                          // Indicate no strokes remain
}
