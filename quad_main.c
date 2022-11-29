#include <string.h> 
#include <math.h>
#include "quadtree.h"
#include <string.h>
#include "drawing.c"
#include <stdlib.h>

int max(int a, int b);
int min(int a, int b);
void IN_ORDER_PRINT(struct rect* rectangle, int trace);
void PRINT_RECTANGLES(struct qnode* curr, int quad_LX, int quad_LY, int quad_RX, int quad_RY);
struct rect* DELETE_RECTANGLE_AUX(char *N, struct qnode* curr, int trace);
struct qnode* INSERT_AUX(struct rect* rectangle, struct qnode *curr, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY, int trace);
int INTERSECT(char* N, struct qnode *curr, int LLX, int URX, int LLY, int URY, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY, int trace);
struct rect* INSERT_BST(struct rect* rectangle, struct rect* curr);
struct rect* BST_SEARCH(char* N, struct rect* curr);
struct rect* SEARCH_POINT_AUX(int PX, int PY, struct qnode *curr, int quad_RX, int quad_RY, int quad_LX, int quad_LY, int trace);
int CONTAINS(int LLX, int LLY, int URX, int URY, int quad_LX, int quad_LY, int quad_RX, int quad_RY);

int width = 0; /* determines width of each square covered by quadtree. */
int count = 0; /* number of rectangles in database */
char rectangles_found[MAX_STRING_LEN] = {0};

void INIT_QUADTREE(int WIDTH, int trace) {
    WorldLX = (int)pow(2, WIDTH);
    WorldLY = (int)pow(2, WIDTH);

    if(qt == NULL && rt == NULL) {
        qt = malloc(sizeof(struct qnode));
        rt = malloc(sizeof(struct qnode));
        qt->data = NULL;
        qt->son[NW] = NULL;
        qt->son[NE] = NULL;
        qt->son[SW] = NULL;
        qt->son[SE] = NULL;
    } else {
        free(qt);
        qt = malloc(sizeof(struct qnode));
        qt->data = NULL;
        qt->son[NW] = NULL;
        qt->son[NE] = NULL;
        qt->son[SW] = NULL;
        qt->son[SE] = NULL;
    }
    printf("INIT_QUADTREE(%d): initialized a quadtree of width %d\n", WIDTH, WorldLX);
}

void DISPLAY() {
    StartPicture(WorldLX, WorldLY);
    DrawLine(0, WorldLY / 2, WorldLX, WorldLY / 2);
    PRINT_RECTANGLES(qt, 0, 0, WorldLX, WorldLY);
    EndPicture();
}

void PRINT_RECTANGLES(struct qnode* curr, int quad_LX, int quad_LY, int quad_RX, int quad_RY) {
    if(curr) {
        if(curr->data) {
        DrawLine(quad_LX, quad_RY / 2, quad_RX, quad_RY / 2);
        DrawLine(quad_RX / 2, quad_LY, quad_RX / 2, quad_RY);
        DrawRect(curr->data->corner->x, curr->data->corner->y, curr->data->size->x, curr->data->size->y);
        DrawName(curr->data->name, curr->data->corner->x, curr->data->corner->y);
        } else {
            PRINT_RECTANGLES(curr->son[NW], quad_LX, 
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY);
            PRINT_RECTANGLES(curr->son[NE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY);
            PRINT_RECTANGLES(curr->son[SW], quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2);
            PRINT_RECTANGLES(curr->son[SE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2);
        }
    }  
}

void LIST_RECTANGLES(int trace) {
    printf("LIST_RECTANGLES(): listing %d rectangles: \n", count);
    if(rt->data) {
        IN_ORDER_PRINT(rt->data, trace);
    } 
}

/* helper function for LIST_RECTANGLES */
void IN_ORDER_PRINT(struct rect* rectangle, int trace) {
    if(rectangle) {
        IN_ORDER_PRINT(rectangle->son[LEFT], trace);
        printf("%s: %d %d %d %d\n", rectangle->name, rectangle->corner->x, rectangle->corner->y,
        rectangle->size->x, rectangle->size->y);
        IN_ORDER_PRINT(rectangle->son[RIGHT], trace);
    } else {
        return;
    }
}

void CREATE_RECTANGLE(char* N, int LLX, int LLY, int LX, int LY) {
    struct rect* new_rect = malloc(sizeof (struct rect));
    struct point* new_corner = malloc(sizeof (struct point));
    struct point* new_size = malloc(sizeof (struct point));

    strcpy(new_rect->name, N); 

    new_corner->x = LLX;
    new_corner->y = LLY;
    new_size->x = LX;
    new_size->y = LY;

    new_rect->corner = new_corner;
    new_rect->size = new_size;
    new_rect->son[LEFT] = NULL;
    new_rect->son[RIGHT] = NULL;

    if(rt->data) {
        INSERT_BST(new_rect, rt->data);
    } else {
        rt->data = new_rect;
    }
    count++; /* new rectangle added to database, so we increment count*/
    printf("CREATE_RECTANGLE(%s, %d, %d, %d, %d): created rectangle %s\n", N, LLX, LLY, LX, LY, N);
}

/* Inserts created rectangle into the binary search tree */
struct rect* INSERT_BST(struct rect* rectangle, struct rect* curr) {
    if (curr == NULL) {      /* when r tree is empty */
        return rectangle;
    } 
    else {
        /* if name to be inserted is less than the name at node */
        if(strcmp(rectangle->name, curr->name) < 0) {
            curr->son[LEFT] = INSERT_BST(rectangle, curr->son[LEFT]);
        } else {
            curr->son[RIGHT] = INSERT_BST(rectangle, curr->son[RIGHT]);
        }
    }
    return curr;
}

// come back to this function afterward
void CREATE_RECTANGLE_RANDOM(char* N) {
    printf("4 %s\n", N);
}

int max(int a, int b) {
    if(a > b) {
        return a;
    } else {
        return b;
    }
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

/* checks if rectangle is in within given quadrant
    (quad_LX, quad_LY) is the leftmost corner of the quadrant
    (quad_RX, quad_RY) is the rightmost corner of the quadrant) 
*/
int QUAD_CHECK(int LLX, int LLY, int URX, int URY, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY) {
    int x_max = max(LLX, quad_LX);
    int y_max = max(LLY, quad_LY);
    /* get min of upper right corners */
    int x_min = min(URX, quad_RX);
    int y_min = min(URY, quad_RY);
    /* compare max of LL corners with min of upper right corners */
    if(x_max < x_min && y_max < y_min) {
        return 1;
    } else {
        return 0;
    }
}

//new auxiliary method for rectangle search
int RECTANGLE_SEARCH_NEW_AUX(struct qnode* curr, int LLX, int LLY, int URX, int URY, 
                         int quad_LX, int quad_LY, int quad_RX, int quad_RY, int total, int trace) {
    if(curr) {
        if(curr->data) {
            int curr_LLX = curr->data->corner->x, 
            curr_LLY = curr->data->corner->y, 
            curr_URX = curr->data->size->x, 
            curr_URY = curr->data->size->y,
            min_URX = min(curr_URX, URX),
            min_URY = min(curr_URY, URY),
            max_LLX = max(curr_LLX, LLX),
            max_LLY = max(curr_LLY, LLY);
            if(max_LLX < min_URX && max_LLY < min_URX) {
                if(strstr(rectangles_found, curr->data->name) == NULL) {
                    strcat(rectangles_found, " ");
                    strcat(rectangles_found, curr->data->name);
                    total += 1;
                }
            }
        }
        if(trace >= 0) {
            printf(" %d", trace);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY)) {
            total = RECTANGLE_SEARCH_NEW_AUX(curr->son[NW], LLX, LLY, URX, URY, 
                        quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, total, 4 * trace + 1);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY)) {
            total = RECTANGLE_SEARCH_NEW_AUX(curr->son[NE], LLX, LLY, URX, URY,  
                        quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, total, 4 * trace + 2);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = RECTANGLE_SEARCH_NEW_AUX(curr->son[SW], LLX, LLY, URX, URY, 
                        quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 3);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = RECTANGLE_SEARCH_NEW_AUX(curr->son[SE], LLX, LLY, URX, URY, 
                        quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 4);
        }
    }
    return total;
}

void RECTANGLE_SEARCH_NEW(char *N, int trace) {
    struct rect* rectangle = BST_SEARCH(N, rt->data);
    int total = 0;
    if(rectangle) {
        if(trace >= 0) {
            printf(" [");
        }
        total = RECTANGLE_SEARCH_NEW_AUX(qt, rectangle->corner->x, rectangle->corner->y, rectangle->size->x, rectangle->size->y, 
                            0, 0, WorldLX, WorldLY, total, trace);
    } 
    if(total == 0) {
        printf("found 0 rectangles\n");
    } else {
        if(trace >= 0) {
            printf("] "); 
        }
        printf(": found %d rectangles:%s\n", total, rectangles_found);
    }
    memset(rectangles_found, 0, sizeof(rectangles_found));
}

//this is my wrong implementation of rectangle search, but it is important for 
//a majority of later functions 
int RECTANGLE_SEARCH(char* N, int trace) {
    struct rect* rectangle = BST_SEARCH(N, rt->data);
    int LLX = rectangle->corner->x, //lower left x
        LLY = rectangle->corner->y, //lower left y
        URY = rectangle->size->y, //upper right y
        URX = rectangle->size->x, //upper right x
        NW_FLAG = -1, NE_FLAG = -1, SW_FLAG = -1, SE_FLAG = -1,
        bool = 0;
    
    if(LLX < 0 || LLX > WorldLX ||
       LLY < 0 || LLY > WorldLX ||
       URY < 0 || LLY > WorldLY ||
       URX < 0 || URX > WorldLY) {
        printf("rectangle N lies outside of space spanned by subtree\n");
        return -1;
    }
    
    /* checks if rectangle is in NW quadrant */
    NW_FLAG = QUAD_CHECK(LLX, LLY, URX, URY, 0, WorldLY / 2, WorldLX / 2, WorldLY);

    /* checks if rectangle is in NE quadrant */
    NE_FLAG = QUAD_CHECK(LLX, LLY, URX, URY, WorldLX / 2, WorldLY / 2, WorldLX, WorldLY);

    /* checks if rectangle is in NW quadrant */
    SW_FLAG = QUAD_CHECK(LLX, LLY, URX, URY, 0, 0, WorldLX / 2, WorldLY / 2);

    /* checks if rectangle is in NE quadrant */
    SE_FLAG = QUAD_CHECK(LLX, LLY, URX, URY, WorldLX / 2, 0, WorldLX, WorldLY / 2);

    if(qt->data) {
        int x_max = max(LLX, qt->data->corner->x);
        int y_max = max(LLY, qt->data->corner->y);
        /* get min of upper right corners */
        int x_min = min(URX, qt->data->size->x);
        int y_min = min(URY, qt->data->size->y);
        /* compare max of LL corners with min of upper right corners */
        if(x_max < x_min && y_max < y_min) {
            if(trace == 0) {
                printf("failed: intersects with %s\n", qt->data->name);
            } else {
                printf("found rectangle %s\n", qt->data->name);
            }
            return 1;
        }
    }

    if(NW_FLAG) {
        bool = bool | INTERSECT(N, qt->son[NW], LLX, URX, LLY, URY, 0, WorldLY / 2, WorldLX / 2, WorldLY, trace);
    }
    if(NE_FLAG) {
        bool = bool | INTERSECT(N, qt->son[NE], LLX, URX, LLY, URY, WorldLX / 2, WorldLY / 2, WorldLX, WorldLY, trace);
    }
    if(SW_FLAG) {
        bool = bool | INTERSECT(N, qt->son[SW], LLX, URX, LLY, URY, 0, 0, WorldLX / 2, WorldLY / 2, trace);
    }
    if(SE_FLAG) {
        bool = bool | INTERSECT(N, qt->son[SE], LLX, URX, LLY, URY, WorldLX / 2, 0, WorldLX, WorldLY / 2, trace);
    }   
    if(bool == 0) {
        if(trace) {
            printf("%s does not intersect an existing rectangle. \n", N);
        }
    }
    return bool;
}

/* helper function for RECTANGLE_SEARCH. searches BST for rectangle with a given name */
struct rect* BST_SEARCH(char* N, struct rect* curr) {
    while(curr != NULL) {
        if(strcmp(N, curr->name) < 0) {
            curr = curr->son[LEFT];
        } else if(strcmp(N, curr->name) > 0) {
            curr = curr->son[RIGHT];
        } else {
            return curr;
        }
    }
    return NULL;
}

/* RECTANGLE_SEARCH helper function */
/* traverses quadtree */
/* returns 1 when it does intersect, returns 0 when it doesn't */
int INTERSECT(char* N, struct qnode *curr, int LLX, int URX, int LLY, int URY, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY, int trace) {
    int bool, bool_in = 0;
    if(curr == NULL) {
        return 0;
    } else {
        if(curr->data) {
            if(LLX == curr->data->corner->x && LLY == curr->data->corner->y) {
                if(trace == 0) {
                    printf("failed: intersects with %s \n", curr->data->name);
                } else {
                    printf("found rectangle %s\n", curr->data->name);
                }
                return 1;
            }
            /* PROCESS CURRENT RECT IN QUADRANT */
            /* get max of lower left corners */
            int x_max = max(LLX, curr->data->corner->x); //SEG FAULT HERE -- ACCESSING NULL DATA
            int y_max = max(LLY, curr->data->corner->y);
            /* get min of upper right corners */
            int x_min = min(URX, curr->data->size->x);
            int y_min = min(URY, curr->data->size->y);
            /* compare max of LL corners with min of upper right corners */
            if(x_max < x_min && y_max < y_min) {
                if(trace == 0) {
                    printf("failed: intersects with %s \n", curr->data->name);
                } else {
                    printf("found rectangle %s\n", curr->data->name);
                }
                return 1;
            }
        }
        //check NW subtree
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, 
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY)) {
            bool = INTERSECT(N, curr->son[NW], LLX, URX, LLY, URY, 
                quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, trace);
            if(bool) {
                return 1;
            }
        }
        //check NE subtree
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2,
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY)) {
            bool = INTERSECT(N, curr->son[NE], LLX, URX, LLY, URY, quad_RX - (quad_RX - quad_LX) / 2,
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, trace);
            if(bool) {
                return 1;
            }
        }
        //check SW subtree
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_LY, 
                      quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2)) {
            bool = INTERSECT(N, curr->son[SW], LLX, URX, LLY, URY, quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, 
                quad_RY - (quad_RY - quad_LY) / 2, trace);
            if(bool) {
                return 1;
            }
        }
        //check SE subtree
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2,
                        quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2)) {
            bool = INTERSECT(N, curr->son[SE], LLX, URX, LLY, URY, quad_RX - (quad_RX - quad_LX) / 2,
                        quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, trace);
            if(bool) {
                return 1;
            }
        }
        //rectangle does not intersect
        return 0;
    }
}

void INSERT(char* N, int trace) {
    int search = RECTANGLE_SEARCH(N, 0);
    struct rect* rectangle;
    if (search == 1) { //1 when it intersects with another rectangle
    } else if (search == -1) { //-1 when outside of quadtree
        printf("rectangle %s lies outside of space spanned by quadtree", N);
    } else { //0 when it doesn't intersect with another rectangle
        rectangle = BST_SEARCH(N, rt->data);
        qt = INSERT_AUX(rectangle, qt, 0, 0, WorldLX, WorldLY, trace);
        if(trace == -10) {
            printf("moved rectangle %s\n", rectangle->name);
        } else {
            printf("inserted rectangle %s\n", rectangle->name);
        }
    }
}

struct qnode* INSERT_AUX(struct rect* rectangle, struct qnode *curr, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY, int trace) {
    int LLX = rectangle->corner->x, //lower left x
        LLY = rectangle->corner->y, //lower left y
        URY = rectangle->size->y, //upper right y
        URX = rectangle->size->x; //upper right x
    //check if rect is in current quadrant. if not, return current node
    if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_LY, quad_RX, quad_RY) == 0) {
        return curr;
    }
    //if current node is null, (and is in curr quadrant -- quad_check is 1) we are ready to insert a new rectangle.
    //make a new node and return it.
    if(curr == NULL) {
        // printf("what is this?? %s\n", rectangle->name);
        struct qnode *insert_node = malloc(sizeof(struct qnode));
        insert_node->data = rectangle;
        return insert_node;
    } 
    //if the current node has a rectangle, keep moving the rectangle down the tree
    if(curr->data != NULL) {
        //insert into all children
        curr->son[NW] = INSERT_AUX(rectangle, curr->son[NW], quad_LX, 
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, 4 * trace + 1);
        curr->son[NE] = INSERT_AUX(rectangle, curr->son[NE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, 4 * trace + 2);
        curr->son[SW] = INSERT_AUX(rectangle, curr->son[SW], quad_LX, quad_LY, 
                        quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 3);
        curr->son[SE] = INSERT_AUX(rectangle, curr->son[SE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 4);
        //now that the rectangle has been inserted down into the tree, move the rectangle in the current node
        //down into the tree.
        struct rect* rect_move = curr->data;
        curr->data = NULL;
        INSERT_AUX(rect_move, curr, quad_LX, quad_LY, quad_RX, quad_RY, trace);

    //if the current node DOESN'T have a rectangle
    //   1) if there ARE children, send the rectangle farther down into the tree
    //   1) if there are no children, insert the rectangle into the node
    } else {
        if(curr->son[NW] == NULL && curr->son[NE] == NULL && curr->son[SW] == NULL && curr->son[NE] == NULL) {
            curr->data = rectangle;
        } else {
            curr->son[NW] = INSERT_AUX(rectangle, curr->son[NW], quad_LX, 
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, 4 * trace + 1);
            curr->son[NE] = INSERT_AUX(rectangle, curr->son[NE], quad_RX - (quad_RX - quad_LX) / 2,
                            quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, 4 * trace + 2);
            curr->son[SW] = INSERT_AUX(rectangle, curr->son[SW], quad_LX, quad_LY, 
                            quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 3);
            curr->son[SE] = INSERT_AUX(rectangle, curr->son[SE], quad_RX - (quad_RX - quad_LX) / 2,
                            quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 4);
        }
    }
    return curr; 
}

int POINT_CHECK(int X, int Y, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY) {
    if ((X < quad_RX && X > quad_LX) && (Y < quad_RY && Y > quad_LY)) {
        return 1;
    } 
    return 0;
}

struct rect* SEARCH_POINT(int PX, int PY, int trace) {
    if(qt) {
        return SEARCH_POINT_AUX(PX, PY, qt, 0, 0, WorldLX, WorldLY, trace);
    } else {
        return NULL;
    }
}

struct rect* SEARCH_POINT_AUX(int PX, int PY, struct qnode *curr, int quad_LX, int quad_LY, int quad_RX, int quad_RY, int trace) {
    int LL_compare = 0;
    int UR_compare = 0;      
    if(POINT_CHECK(PX, PY, quad_LX, quad_LY, quad_RX, quad_RY) == 0) {
        if(trace >= 0) {
            printf("]: ");
        }  
        if(trace != -500) {
            printf("no rectangle found\n");
        }
        return NULL;
    }    
    if(curr == NULL) {
        if(trace >= 0) {
            printf("]: ");
        } 
        if(trace != -500) {
            printf(" no rectangle found\n");
        }
        return NULL;
    }
    if(trace >= 0) {
        printf(" %d", trace);
    }  
    if(curr->data) {
        LL_compare = (PX >= curr->data->corner->x) && (PY >= curr->data->corner->y);
        UR_compare = (PX < curr->data->size->x) && 
                     (PY < curr->data->size->y);
        if(LL_compare && UR_compare) {
            //when it finds it, it is returned.
            if(trace >= 0) {
                printf("]: ");
            } 
            if(trace != -500) {
                printf("found rectangle %s\n", curr->data->name);
            }
            return curr->data;
        } else {
            if(trace >= 0) {
                printf("]: ");
            }
            if(trace != -500) {
                printf(" no rectangle found\n");
            }
            return NULL;
        }
    } else {
        if(PX < quad_RX - (quad_RX - quad_LX) / 2) { //NW or SW
            if(PY > quad_RY - (quad_RY - quad_LY) / 2) { //NW
                return SEARCH_POINT_AUX(PX, PY, curr->son[NW], quad_LX, 
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, 4 * trace + 1);
            } else { //SW
                return SEARCH_POINT_AUX(PX, PY, curr->son[SW], quad_LX, quad_LY, 
                      quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 3);
            }
        } else { //NE or SE
            if(PY > quad_RY - (quad_RY - quad_LY) / 2) { //NE
                return SEARCH_POINT_AUX(PX, PY, curr->son[NE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, 4 * trace + 2);
            } else { //SE
                return SEARCH_POINT_AUX(PX, PY, curr->son[SE], quad_RX - (quad_RX - quad_LX) / 2,
                        quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, 4 * trace + 4);
            }
        }
    }
    if(trace >= 0) {
        printf("]: ");
    }
    if(trace != -500) {
            printf(" no rectangle found\n");
    }
    printf("no rectangle found\n");
    return NULL; //erm...
}

struct rect* DELETE_RECTANGLE(char* N, int trace) {
    struct rect* rectangle = malloc(sizeof(struct rect));
    rectangle = DELETE_RECTANGLE_AUX(N, qt, 0);
    if(trace != 0) {
        if(rectangle) {
            printf("%s was deleted\n", N);
        } else {
            printf("no rectangle deleted -- this rectangle does not exist in the tree.\n");
        }
    }
    return rectangle;
}

struct rect* DELETE_RECTANGLE_AUX(char *N, struct qnode* curr, int trace) {
    int bool = 0; 
    struct rect* rectangle = malloc(sizeof(struct rect));
    char name[MAX_NAME_LEN + 1] = " ";
    //if current node is null, the rectangle does not exist in the tree
    if(curr == NULL) {
        return NULL;
    } else {
        //check if the current node is a leaf
        if(curr->data) {
            //check if this is the rectangle to be deleted
            if(strcmp(N, curr->data->name) == 0) {
                rectangle = curr->data;
                curr->data = NULL;
            }
            else {
                return NULL;
            }
        //if it's not a leaf, this means that we can go further down the tree
        } else {
            DELETE_RECTANGLE_AUX(N, curr->son[NW], 4 * trace + 1);
            DELETE_RECTANGLE_AUX(N, curr->son[NE], 4 * trace + 2);
            DELETE_RECTANGLE_AUX(N, curr->son[SW], 4 * trace + 3);
            DELETE_RECTANGLE_AUX(N, curr->son[SE], 4 * trace + 4);
        }
        //now check if you need to collapse the children
        //THIS HAPPENS WHEN ALL THE CHILDREN HAVE THE SAME R PIECES
        //if node exists
        if(curr->son[NW]) {
            //if node has rectangle
            if(curr->son[NW]->data) {
                //if there is no name, store the name
                strcpy(name, curr->son[NW]->data->name);
                bool = 1;
                //compare current node's name with the stored name
                //bool is 1 when the name is the same, bool is 0 when the name is diff.
                //IT ONLY TAKES ONE ZERO TO STOP THE MERGE PROCESS
            }
        }
        if(curr->son[NE]) {
            if(curr->son[NE]->data) {
                if(strcmp(name, " ") == 0) {
                    strcpy(name, curr->son[NE]->data->name);
                }
                if(strcmp(name, curr->son[NE]->data->name) != 0) {
                    bool = 0;
                } else {
                    bool = 1;
                }
            }
        }
        if(curr->son[SW]) {
            if(curr->son[SW]->data) {
                if(strcmp(name, " ") == 0) {
                    strcpy(name, curr->son[SW]->data->name);
                }
                if(strcmp(name, curr->son[SW]->data->name) != 0) {
                    bool = 0;
                } else {
                    bool = 1;
                }
            }
        }
        if(curr->son[SE]) {
            if(curr->son[SE]->data) {
                if(strcmp(name, " ") == 0) {
                    strcpy(name, curr->son[SE]->data->name);
                }
                if(strcmp(name, curr->son[SE]->data->name) != 0) {
                    bool = 0;
                } else {
                    bool = 1;
                }
            }
        }
        // all the children are r pieces. make sure these are all leaf nodes!
        if(bool == 1) {
            curr->data = BST_SEARCH(name, rt->data);
            if(curr->son[NW]) {
                curr->son[NW] = NULL;
            }
            if(curr->son[NE]) {
                curr->son[NE] = NULL;
            }
            if(curr->son[SW]) {
                curr->son[SW] = NULL;
            }
            if(curr->son[SE]) {
                curr->son[SE] = NULL;
            }
        }
    }
    return rectangle;
}

void DELETE_POINT(int PX, int PY, int trace) {
    DELETE_RECTANGLE(SEARCH_POINT(PX, PY, trace)->name, 0);
}

void MOVE(char* N, int CX, int CY, int trace) {
    struct rect* rectangle = BST_SEARCH(N, rt->data);
    DELETE_RECTANGLE(rectangle->name, trace);
    rectangle->corner->x = rectangle->corner->x + CX;
    rectangle->corner->y = rectangle->corner->y + CY;
    INSERT(N, -10);
}
/* PART 4 */

int TOUCH_AUX(struct qnode* curr, int LLX, int LLY, int URX, int URY, int total, int trace) {
    if(curr) {
        if(curr->data) {
            int curr_LLX = curr->data->corner->x, 
                curr_LLY = curr->data->corner->y, 
                curr_URX = curr->data->size->x, 
                curr_URY = curr->data->size->y,
                min_URX = min(curr_URX, URX),
                min_URY = min(curr_URY, URY),
                max_LLX = max(curr_LLX, LLX),
                max_LLY = max(curr_LLY, LLY);
            if(max_LLX == min_URX && max_LLY == min_URY) {
                if(strstr(rectangles_found, curr->data->name) == NULL) {
                    strcat(rectangles_found, " ");
                    strcat(rectangles_found, curr->data->name);
                    total += 1;
                }
            }
        }
        if(trace >= 0) {
            printf(" %d", trace);
        } 
        int NW_total = TOUCH_AUX(curr->son[NW], LLX, LLY, URX, URY, total, 4 * trace + 1);
        int NE_total = TOUCH_AUX(curr->son[NE], LLX, LLY, URX, URY, NW_total, 4 * trace + 2);
        int SW_total = TOUCH_AUX(curr->son[SW], LLX, LLY, URX, URY, NE_total, 4 * trace + 3);
        int SE_total = TOUCH_AUX(curr->son[SE], LLX, LLY, URX, URY, SW_total, 4 * trace + 4);
        total = SE_total;
    }
    return total;
}

void TOUCH(char* N, int trace) {
    struct rect* rectangle = BST_SEARCH(N, rt->data);
    int total = 0;
    if(rectangle) {
        if(trace >= 0) {
            printf(" [");
        }
        // printf("1: %d, 2: %d, 3: %d, 4: %d\n", rectangle->corner->x, rectangle->corner->y, rectangle->size->x, rectangle->size->y);
        total = TOUCH_AUX(qt, rectangle->corner->x, rectangle->corner->y, rectangle->size->x, rectangle->size->y, total, trace);
    }
    if(total == 0) {
        printf("found 0 rectangles\n");
    } else {
        if(trace >= 0) {
            printf("] "); 
        }
        printf(": found %d rectangles:%s\n", total, rectangles_found);
    }
    memset(rectangles_found, 0, sizeof(rectangles_found));
}

int WITHIN_AUX(struct qnode* curr, int LLX, int LLY, int URX, int URY, 
               int quad_LX, int quad_LY, int quad_RX, int quad_RY, int total, int trace) {
    if(curr) {
        if(curr->data) {
            int curr_LLX = curr->data->corner->x, 
            curr_LLY = curr->data->corner->y, 
            curr_URX = curr->data->size->x, 
            curr_URY = curr->data->size->y,
            min_URX = min(curr_URX, URX),
            min_URY = min(curr_URY, URY),
            max_LLX = max(curr_LLX, LLX),
            max_LLY = max(curr_LLY, LLY);
            if(max_LLX < min_URX && max_LLY < min_URX &&
                !CONTAINS(curr_LLX, curr_LLY, curr_URX, curr_URY, LLX, LLY, URX, URY)) {
                if(strstr(rectangles_found, curr->data->name) == NULL) {
                    strcat(rectangles_found, " ");
                    strcat(rectangles_found, curr->data->name);
                    total += 1;
                }
            }
        }
        if(trace >= 0) {
            printf(" %d", trace);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY)) {
            total = WITHIN_AUX(curr->son[NW], LLX, LLY, URX, URY, 
                        quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, total, 4 * trace + 1);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY)) {
            total = WITHIN_AUX(curr->son[NE], LLX, LLY, URX, URY,  
                        quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, total, 4 * trace + 2);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = WITHIN_AUX(curr->son[SW], LLX, LLY, URX, URY, 
                        quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 3);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = WITHIN_AUX(curr->son[SE], LLX, LLY, URX, URY, 
                        quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 4);
        }
    }
    return total;
}

void WITHIN(char* N, int D, int trace) {
    struct rect* rectangle = BST_SEARCH(N, rt->data);
    int total = 0;
    if(rectangle) {
        if(trace >= 0) {
            printf(" [");
        }
        // printf("llx: %d, lly: %d, urx: %d, ury: %d\n", rectangle->corner->x - D, rectangle->corner->y - D, rectangle->size->x + D, rectangle->size->y + D);
        total = WITHIN_AUX(qt, rectangle->corner->x - D, rectangle->corner->y - D, rectangle->size->x + D, rectangle->size->y + D, 
                            0, 0, WorldLX, WorldLY, total, trace);
    } 
    if(total == 0) {
        printf(": found 0 rectangles\n");
    } else {
        if(trace >= 0) {
            printf("] "); 
        }
        printf(": found %d rectangles:%s\n", total, rectangles_found);
    }
    memset(rectangles_found, 0, sizeof(rectangles_found));
}

int abs(int x) {
    if(x < 0) {
        return -1 * x;
    } else {
        return x;
    }
}

//node for priority queue
typedef struct node {
    struct qnode* q_node;
    int distance; //nodes with shorter distance to query come first (this indicates priority)
    int trace;
    struct node* next;
} Node;

Node* create_node(struct qnode* q_node, struct rect* rectangle, int trace, int method) {
    //method = 0 for horiz distance, 1 for vert distance, 2 for nearest_rectangle, 3 for nearest_neighbor
    Node* node = (Node*) malloc(sizeof(Node));
    //if q_node has children, there is no real distance value
    if(q_node->son[NW] || q_node->son[NE] || q_node->son[SW] || q_node->son[SE]) {
        node->distance = WorldLX + 10; //makes sure this node will be put at end of priority queue
    } else {
        if(method == 0) {
            node->distance = max(abs(q_node->data->corner->x - rectangle->size->x),
                               abs(q_node->data->size->x - rectangle->corner->x));
        } else if(method == 1) {
            node->distance = max(abs(q_node->data->corner->y - rectangle->size->y),
                               abs(q_node->data->size->y - rectangle->corner->y));
        } else if(method == 2) {
            int LLX_diff = q_node->data->corner->x - rectangle->corner->x,
                LLY_diff = q_node->data->corner->y - rectangle->corner->y,
                URX_diff = rectangle->corner->x - q_node->data->size->x,
                URY_diff = rectangle->corner->y - q_node->data->size->y,
                x_max = max(LLX_diff, URX_diff),
                y_max = max(LLY_diff, URY_diff);
            node->distance = (int) sqrt(pow(x_max, 2.0) + pow(y_max, 2.0));
        } else {
            int LL_URI_DIFF_X = q_node->data->corner->x - rectangle->size->x,
                LL_URI_DIFF_Y = q_node->data->corner->y - rectangle->size->y,
                LLI_UR_DIFF_X = rectangle->corner->x - q_node->data->size->x,
                LLI_UR_DIFF_Y = rectangle->corner->y - q_node->data->size->y,
                x_max = max(LL_URI_DIFF_X, LLI_UR_DIFF_X),
                y_max = max(LL_URI_DIFF_Y, LLI_UR_DIFF_Y);
            node->distance = (int) sqrt(pow(x_max, 2.0) + pow(y_max, 2.0));
        }
    }
    node->trace = trace;
    node->q_node = q_node;
    node->next = NULL;
    return node;
}

Node* pop(Node** head) {
    Node* node = *head;
    if(node) {
        char* name = " ";
        (*head) = (*head)->next;
        return node;
    }
    return NULL;
}

void push(Node** head, struct qnode* q_node, struct rect* rectangle, int trace, int method) {
    if(q_node) {
        Node* start = (*head);
        // create node
        Node* node = create_node(q_node, rectangle, trace, method);
        if(start == NULL) {
            (*head) = node;
        } else { 
            // if curr node has higher priority than the head
            if ((*head)->distance > node->distance || ((*head)->distance == node->distance && (*head)->trace > node->trace)) {
                node->next = *head;
                (*head) = node;
            } 
            else {
                // go through list to find position
                while (start->next != NULL && start->next->distance < node->distance) {
                    start = start->next;
                }
                node->next = start->next;
                start->next = node;
            }
        }
    }
}

//checks if one rectangle contains another
int CONTAINS(int LLX, int LLY, int URX, int URY, 
                int quad_LX, int quad_LY, int quad_RX, int quad_RY) {
    /* compare max of LL corners with min of upper right corners */
    int LL_compare = (quad_LX <= LLX) && (quad_LY <= LLY);
    int UR_compare = (URX <= quad_RX) && (URY <= quad_RY);
    if(LL_compare && UR_compare) {
        return 1;
    } else {
        return 0;
    }
}

//horiz and vert neighbor implement trace
struct rect* HORIZ_NEIGHBOR(char* N, int trace) {
    Node* queue = create_node(qt, BST_SEARCH(N, rt->data), trace, 0), //initalize queue with the root
          *popped;
    int min_dist = WorldLX + 1, distance;
    struct rect* compare_rect = BST_SEARCH(N, rt->data), *closest_rect, *rectangle;
    if(trace >= 0) {
        printf("[");
    }
    while(queue) {
        popped = pop(&queue);
        if(trace >= 0) {
            printf(" %d", popped->trace);
        }
        //if the popped node is a leaf
        if(popped->q_node->son[NW] == NULL && popped->q_node->son[NE] == NULL && popped->q_node->son[SW] == NULL && 
        popped->q_node->son[SE] == NULL) {
            rectangle = popped->q_node->data;
            distance = popped->distance;
            if(distance < min_dist && !CONTAINS(compare_rect->corner->x, compare_rect->corner->y,
                                                 compare_rect->size->x, compare_rect->size->y, rectangle->corner->x,
                                                 rectangle->corner->y, rectangle->size->x, rectangle->size->y)) {
                closest_rect = rectangle;
                min_dist = distance;
            }
        //else if popped node has children
        } else if(popped->q_node->son[NW] || popped->q_node->son[NE] || popped->q_node->son[SW] || 
        popped->q_node->son[SE]) {
            // printf("non-leaf node made it\n");
            if(popped->q_node->son[NW]) {
                push(&queue, popped->q_node->son[NW], compare_rect, 4 * popped->trace + 1, 0);
            }
            if(popped->q_node->son[NE]) {
                push(&queue, popped->q_node->son[NE], compare_rect, 4 * popped->trace + 2, 0);
            }
            if(popped->q_node->son[SW]) {
                push(&queue, popped->q_node->son[SW], compare_rect, 4 * popped->trace + 3, 0);
            }
            if(popped->q_node->son[SE]) {
                push(&queue, popped->q_node->son[SE], compare_rect, 4 * popped->trace + 4, 0);
            }
        }
    }
    if(trace >= 0) {
        printf("]");
    }
    if(closest_rect) {
        printf(": found rectangle %s\n", closest_rect->name);
    } else {
        printf(": no rectangle found\n");
    }
    free(popped);
    return closest_rect;
}

struct rect* VERT_NEIGHBOR(char* N, int trace) {
    Node* queue = create_node(qt, BST_SEARCH(N, rt->data), trace, 1), //initalize queue with the root
          *popped;
    int min_dist = WorldLX + 1, distance;
    struct rect* compare_rect = BST_SEARCH(N, rt->data), *closest_rect, *rectangle;
    if(trace >= 0) {
        printf("[");
    }
    while(queue) {
        popped = pop(&queue);
        if(trace >= 0) {
            printf(" %d", popped->trace);
        }
        //if the popped node is a leaf
        if(popped->q_node->son[NW] == NULL && popped->q_node->son[NE] == NULL && popped->q_node->son[SW] == NULL && 
        popped->q_node->son[SE] == NULL) {
            rectangle = popped->q_node->data;
            distance = popped->distance;
            if(distance < min_dist && !CONTAINS(compare_rect->corner->x, compare_rect->corner->y,
                                                 compare_rect->size->x, compare_rect->size->y, rectangle->corner->x,
                                                 rectangle->corner->y, rectangle->size->x, rectangle->size->y)) {
                closest_rect = rectangle;
                min_dist = distance;
            }
        //else if popped node has children
        } else if(popped->q_node->son[NW] || popped->q_node->son[NE] || popped->q_node->son[SW] || 
        popped->q_node->son[SE]) {
            // printf("non-leaf node made it\n");
            if(popped->q_node->son[NW]) {
                push(&queue, popped->q_node->son[NW], compare_rect, 4 * popped->trace + 1, 1);
            }
            if(popped->q_node->son[NE]) {
                push(&queue, popped->q_node->son[NE], compare_rect, 4 * popped->trace + 2, 1);
            }
            if(popped->q_node->son[SW]) {
                push(&queue, popped->q_node->son[SW], compare_rect, 4 * popped->trace + 3, 1);
            }
            if(popped->q_node->son[SE]) {
                push(&queue, popped->q_node->son[SE], compare_rect, 4 * popped->trace + 4, 1);
            }
        }
    }
    if(trace >= 0) {
        printf("]");
    }
    if(closest_rect) {
        printf(": found rectangle %s\n", closest_rect->name);
    } else {
        printf(": no rectangle found\n");
    }
    free(popped);
    return closest_rect;
}

struct rect* NEAREST_RECTANGLE(int PX, int PY, int trace) {
    //store point as a rectangle
    struct rect* point = malloc(sizeof(struct rect));
    point->corner = malloc(sizeof(struct point));
    point->size = malloc(sizeof(struct point));
    point->corner->x = PX;
    point->corner->y = PY;
    Node* queue = create_node(qt, point, trace, 2), //initalize queue with the root
          *popped;
    int min_dist = WorldLX + 1, distance;
    struct rect* compare_rect = point, *closest_rect, *rectangle;
    if(trace >= 0) {
        printf("[");
    }
    while(queue) {
        popped = pop(&queue);
        if(trace >= 0) {
            printf(" %d", popped->trace);
        }
        //if the popped node is a leaf
        if(popped->q_node->son[NW] == NULL && popped->q_node->son[NE] == NULL && popped->q_node->son[SW] == NULL && 
        popped->q_node->son[SE] == NULL) {
            rectangle = popped->q_node->data;
            distance = popped->distance;
            if(distance < min_dist) {
                closest_rect = rectangle;
                min_dist = distance;
            }
        //else if popped node has children
        } else if(popped->q_node->son[NW] || popped->q_node->son[NE] || popped->q_node->son[SW] || 
        popped->q_node->son[SE]) {
            if(popped->q_node->son[NW]) {
                push(&queue, popped->q_node->son[NW], compare_rect, 4 * popped->trace + 1, 2);
            }
            if(popped->q_node->son[NE]) {
                push(&queue, popped->q_node->son[NE], compare_rect, 4 * popped->trace + 2, 2);
            }
            if(popped->q_node->son[SW]) {
                push(&queue, popped->q_node->son[SW], compare_rect, 4 * popped->trace + 3, 2);
            }
            if(popped->q_node->son[SE]) {
                push(&queue, popped->q_node->son[SE], compare_rect, 4 * popped->trace + 4, 2);
            }
        }
    }
    if(trace >= 0) {
        printf("]");
    }
    if(closest_rect) {
        printf(": found rectangle %s\n", closest_rect->name);
    } else {
        printf(": no rectangle found\n");
    }
    free(popped);
    return closest_rect;
}

int WINDOW_AUX(struct qnode* curr, int LLX, int LLY, int URX, int URY, int quad_LX, int quad_LY, int quad_RX, int quad_RY, int total, int trace) {
    if(curr) {
        if(curr->data) {
            int curr_LLX = curr->data->corner->x, 
                curr_LLY = curr->data->corner->y, 
                curr_URX = curr->data->size->x, 
                curr_URY = curr->data->size->y;
            // printf("min_URX: %d, min_URY: %d, max_LLX: %d, max_LLY: %d\n", min_URX, min_URY, max_LLX, max_LLY);
            if(CONTAINS(curr_LLX, curr_LLY, curr_URX, curr_URY, LLX, LLY, URX, URY)) {
                if(strstr(rectangles_found, curr->data->name) == NULL) {
                    strcat(rectangles_found, " ");
                    strcat(rectangles_found, curr->data->name);
                    total += 1;
                }
            }
        }
        if(trace >= 0) {
            printf(" %d", trace);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY)) {
            total = WINDOW_AUX(curr->son[NW], LLX, LLY, URX, URY, 
                        quad_LX, quad_RY - (quad_RY - quad_LY) / 2, quad_RX - (quad_RX - quad_LX) / 2, quad_RY, total, 4 * trace + 1);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY)) {
            total = WINDOW_AUX(curr->son[NE], LLX, LLY, URX, URY,  
                        quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, quad_RX, quad_RY, total, 4 * trace + 2);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = WINDOW_AUX(curr->son[SW], LLX, LLY, URX, URY, 
                        quad_LX, quad_LY, quad_RX - (quad_RX - quad_LX) / 2, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 3);
        }
        if(QUAD_CHECK(LLX, LLY, URX, URY, quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2)) {
            total = WINDOW_AUX(curr->son[SE], LLX, LLY, URX, URY, 
                        quad_RX - (quad_RX - quad_LX) / 2, quad_LY, quad_RX, quad_RY - (quad_RY - quad_LY) / 2, total, 4 * trace + 4);
        }
    }
    return total;
}

void WINDOW(int LLX, int LLY, int LX, int LY, int trace) {
    int total = 0;
    if(trace >= 0) {
        printf(" [");
    }
    total = WINDOW_AUX(qt, LLX, LLY, LX, LY, 0, 0, WorldLX, WorldLY, total, trace);
    if(total == 0) {
        printf("found 0 rectangles\n");
    } else {
        if(trace >= 0) {
            printf("] "); 
        }
        printf(": found %d rectangles:%s\n", total, rectangles_found);
    }
    memset(rectangles_found, 0, sizeof(rectangles_found));
}

//EXTRA CREDIT

void NEAREST_NEIGHBOR(char* N, int trace) {
    Node* queue = create_node(qt, BST_SEARCH(N, rt->data), trace, 3), //initalize queue with the root
          *popped;
    int min_dist = WorldLX + 1, distance;
    struct rect* compare_rect = BST_SEARCH(N, rt->data), *closest_rect, *rectangle;
    if(trace >= 0) {
        printf("[");
    }
    while(queue) {
        popped = pop(&queue);
        if(trace >= 0) {
            printf(" %d", popped->trace);
        }
        //if the popped node is a leaf
        if(popped->q_node->son[NW] == NULL && popped->q_node->son[NE] == NULL && popped->q_node->son[SW] == NULL && 
        popped->q_node->son[SE] == NULL) {
            rectangle = popped->q_node->data;
            distance = popped->distance;
            if(distance < min_dist && !CONTAINS(compare_rect->corner->x, compare_rect->corner->y,
                                                 compare_rect->size->x, compare_rect->size->y, rectangle->corner->x,
                                                 rectangle->corner->y, rectangle->size->x, rectangle->size->y)) {
                closest_rect = rectangle;
                min_dist = distance;
            }
        //else if popped node has children
        } else if(popped->q_node->son[NW] || popped->q_node->son[NE] || popped->q_node->son[SW] || 
        popped->q_node->son[SE]) {
            // printf("non-leaf node made it\n");
            if(popped->q_node->son[NW]) {
                push(&queue, popped->q_node->son[NW], compare_rect, 4 * popped->trace + 1, 3);
            }
            if(popped->q_node->son[NE]) {
                push(&queue, popped->q_node->son[NE], compare_rect, 4 * popped->trace + 2, 3);
            }
            if(popped->q_node->son[SW]) {
                push(&queue, popped->q_node->son[SW], compare_rect, 4 * popped->trace + 3, 3);
            }
            if(popped->q_node->son[SE]) {
                push(&queue, popped->q_node->son[SE], compare_rect, 4 * popped->trace + 4, 3);
            }
        }
    }
    if(trace >= 0) {
        printf("]");
    }
    if(closest_rect) {
        printf(": found rectangle %s\n", closest_rect->name);
    } else {
        printf(": no rectangle found\n");
    }
    free(popped);
}

void LEXICALLY_GREATER_NEAREST_NEIGHBOR(char* N, int trace) {
    Node* queue = create_node(qt, BST_SEARCH(N, rt->data), trace, 3), //initalize queue with the root
          *popped;
    int min_dist = WorldLX + 1, distance;
    struct rect* compare_rect = BST_SEARCH(N, rt->data), *closest_rect, *rectangle;
    if(trace >= 0) {
        printf("[");
    }
    while(queue) {
        popped = pop(&queue);
        if(trace >= 0) {
            printf(" %d", popped->trace);
        }
        //if the popped node is a leaf
        if(popped->q_node->son[NW] == NULL && popped->q_node->son[NE] == NULL && popped->q_node->son[SW] == NULL && 
        popped->q_node->son[SE] == NULL) {
            rectangle = popped->q_node->data;
            distance = popped->distance;
            //check if str is lexically smaller than curr str
            if(strcmp(N, rectangle->name) < 0) {
                if(distance < min_dist && !CONTAINS(compare_rect->corner->x, compare_rect->corner->y,
                                                 compare_rect->size->x, compare_rect->size->y, rectangle->corner->x,
                                                 rectangle->corner->y, rectangle->size->x, rectangle->size->y)) {
                closest_rect = rectangle;
                min_dist = distance;
                }
            }
        //else if popped node has children
        } else if(popped->q_node->son[NW] || popped->q_node->son[NE] || popped->q_node->son[SW] || 
        popped->q_node->son[SE]) {
            // printf("non-leaf node made it\n");
            if(popped->q_node->son[NW]) {
                push(&queue, popped->q_node->son[NW], compare_rect, 4 * popped->trace + 1, 3);
            }
            if(popped->q_node->son[NE]) {
                push(&queue, popped->q_node->son[NE], compare_rect, 4 * popped->trace + 2, 3);
            }
            if(popped->q_node->son[SW]) {
                push(&queue, popped->q_node->son[SW], compare_rect, 4 * popped->trace + 3, 3);
            }
            if(popped->q_node->son[SE]) {
                push(&queue, popped->q_node->son[SE], compare_rect, 4 * popped->trace + 4, 3);
            }
        }
    }
    if(trace >= 0) {
        printf("]");
    }
    if(closest_rect) {
        printf(": found rectangle %s\n", closest_rect->name);
    } else {
        printf(": no rectangle found\n");
    }
    free(popped);
}

void LABEL() {
    printf("17\n");
}

int main() {
    //get input from scanner
    int MAX_LINE_LENGTH = 100;
    char command[MAX_LINE_LENGTH], func[MAX_LINE_LENGTH], params[MAX_LINE_LENGTH];
    //variables from input
    char N[100];
    int width, LLX, LLY, LX, LY, PX, PY, CX, CY, D, trace = 0;

    //go line by line through input (command is the indiv line)
    while(fgets(command, MAX_LINE_LENGTH, stdin) != NULL) {
        if(strcmp(command, "TRACE ON\n") == 0) {
            printf("TRACE ON\n");
            trace = 0;
        } else if (strcmp(command, "TRACE OFF\n") == 0) {
            printf("TRACE OFF\n");
            trace = -2;
        } else {
            sscanf(command, "%[^(](%s)", func, params);
            if(strcmp(func, "INIT_QUADTREE") == 0) {    
                sscanf(params, "%d", &width);    
                INIT_QUADTREE(width, trace);
            } else if (strcmp(func, "DISPLAY") == 0) {
                DISPLAY();
            } else if (strcmp(func, "LIST_RECTANGLES") == 0) {
                LIST_RECTANGLES(trace);
            } else if (strcmp(func, "CREATE_RECTANGLE") == 0) {
                sscanf(params, "%[^,],%d,%d,%d,%d)", N, &LLX, &LLY, &LX, &LY);
                CREATE_RECTANGLE(N, LLX, LLY, LX, LY);
            } else if (strcmp(func, "CREATE_RECTANGLE_RANDOM") == 0) {
                sscanf(params, "%[^)])", N);    
                CREATE_RECTANGLE_RANDOM(N);
            } else if (strcmp(func, "RECTANGLE_SEARCH") == 0) {
                sscanf(params, "%[^)])", N);    
                printf("RECTANGLE_SEARCH(%s)", N);
                RECTANGLE_SEARCH_NEW(N, trace);
            } else if (strcmp(func, "INSERT") == 0) {
                sscanf(params, "%[^)])", N);   
                printf("INSERT(%s): ", N); 
                INSERT(N, trace);
            } else if (strcmp(func, "SEARCH_POINT") == 0) {
                sscanf(params, "%d,%d)", &PX, &PY);    
                printf("SEARCH_POINT(%d,%d): ", PX, PY);
                if(trace >= 0) {
                    printf("[");
                }
                SEARCH_POINT(PX, PY, trace);
            } else if (strcmp(func, "DELETE_RECTANGLE") == 0) {
                sscanf(params, "%[^)])", N);   
                DELETE_RECTANGLE(N, trace);
            } else if (strcmp(func, "DELETE_POINT") == 0) {
                sscanf(params, "%d,%d)", &PX, &PY);   
                printf("DELETE POINT(%d,%d): ", PX, PY); 
                DELETE_POINT(PX, PY, -2);
            } else if (strcmp(func, "MOVE") == 0) {
                sscanf(params, "%[^,],%d,%d)", N, &PX, &PY);  
                printf("MOVE(%s,%d,%d): ", N, PX, PY);  
                MOVE(N, PX, PY, trace);
            } else if (strcmp(func, "TOUCH") == 0) {
                sscanf(params, "%[^)])", N);   
                printf("TOUCH(%s)", N);
                TOUCH(N, trace);
            } else if (strcmp(func, "WITHIN") == 0) {
                sscanf(params, "%[^,],%d)", N, &D);
                printf("WITHIN(%s,%d)", N, D);
                WITHIN(N, D, trace);
            } else if (strcmp(func, "HORIZ_NEIGHBOR") == 0) {
                sscanf(params, "%[^)])", N);   
                printf("HORIZ_NEIGHBOR(%s)", N); 
                HORIZ_NEIGHBOR(N, trace);
            } else if (strcmp(func, "VERT_NEIGHBOR") == 0) {
                sscanf(params, "%[^)])", N); 
                printf("VERT_NEIGHBOR(%s)", N);    
                VERT_NEIGHBOR(N, trace);
            } else if (strcmp(func, "NEAREST_RECTANGLE") == 0) {
                sscanf(params, "%d,%d)", &PX, &PY); 
                printf("NEAREST_RECTANGLE(%d,%d)", PX, PY);    
                NEAREST_RECTANGLE(PX, PY, trace);
            } else if (strcmp(func, "WINDOW") == 0) {
                sscanf(params, "%d,%d,%d,%d)", &LLX, &LLY, &LX, &LY);
                printf("WINDOW(%d,%d,%d,%d)", LLX, LLY, LX, LY);   
                WINDOW(LLX, LLY, LX, LY, trace);
            } else if (strcmp(func, "NEAREST_NEIGHBOR") == 0) {
                sscanf(params, "%[^)])", N); 
                printf("NEAREST_NEIGHBOR(%s)", N);       
                NEAREST_NEIGHBOR(N, trace);
            } else if (strcmp(func, "LEXICALLY_GREATER_NEAREST_NEIGHBOR") == 0) {
                printf("LEXICALLY_GREATER_NEAREST_NEIGHBOR(%s)", N); 
                sscanf(params, "%[^)])", N);    
                LEXICALLY_GREATER_NEAREST_NEIGHBOR(N, trace);
            } else if (strcmp(func, "LABEL") == 0) {
                LABEL();
            } 
        }
    }

    return 0;

}