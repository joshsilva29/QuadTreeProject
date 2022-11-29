#include <stdio.h>

#define MAX_STRING_LEN 256
#define MAX_NAME_LEN 6

int worldX = 0,     worldY = 0;     /* Coordinates of the lower left corner */
int WorldLX = 128,  WorldLY = 128;  
int MinLX = 3,      MinLY = 3;      /* The smallest rectangle is 3x3 */
int MaxLX = 128,    MaxLY = 128;    /* The largest rectangle is 128x128 */
int PixelLX = 1,    PixelLY = 1;    /* Pixel is 1x1 - the smallest unit into which our
                                    quadtree will decompose the world */

typedef enum { LEFT, RIGHT } direction;
typedef enum { NW, NE, SW, SE } quadrant;
typedef enum { GRAY, BLACK, WHITE } nodetype;

struct point {
    int x, y;
};

struct rect {
    char name[MAX_NAME_LEN + 1];   /* Name of the rectangle */
    struct point *corner;          /* Lower left corner */
    struct point *size;            /* Horizontal and vertical size */
    int label;                     /* Used for op #11: LABEL() */
    struct rect *son[2];           /* Left and right sons */
};

struct qnode {
    nodetype type;                 /* GRAY, BLACK, WHITE */
    struct rect *data;             /* Pointer to rectangle contained in this quadrant */
    struct qnode *son[4];          /* Four principal quadrant directions */
};

struct qnode *qt = NULL;           /* Rectangle quadtree: points to the root of the rectangle quadtree.
                                   Initially qt=NULL */
struct qnode *rt = NULL;           /* rectangle-tree: points to the root of sorted binary search tree
                                    of rectangles (sorted with respect to rectangle names). */