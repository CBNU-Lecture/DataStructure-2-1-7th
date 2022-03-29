#include <stdio.h>
#include <stdlib.h>

#define ASCII_ZERO ('0')
#define DIR_COUNT (8)
#define REVERSE_DIR (7)

#define TEST (1)
#define DEBUG (1)

enum {
    TRUE = 1,
    FALSE = 0
};

enum {
    PATH = 0,
    NONPATH = 1
};

typedef enum {
    SOUTH_EAST = 0,
    SOUTH = 1,
    EAST = 2,
    NORTH_EAST = 3,
    SOUTH_WEST = 4,
    WEST = 5,
    NORTH = 6,
    NORTH_WEST = 7
} dir_t;

typedef struct {
    int x;
    int y;
} xy_t;

typedef struct {
    xy_t xy;
    dir_t dir;
} xyd_t;

typedef struct path {
    xyd_t xyd;
    struct path* link;
} path_t;

// ======== stack ========
void push(path_t** ptop, xyd_t xy);
xyd_t pop(path_t** ptop);

// =====maze setting=====
xy_t get_maze_size(void);
void load_maze(int* p_maze, xy_t size_out);
void deep_copy_maze(int* p_maze, int* p_mark, xy_t size_out);


// ======find path ======
dir_t search_dir(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre);
int count_direction_opt(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre);
xy_t move_xy(xy_t xy_crr, dir_t dir_pre);
void delete_mark_path(int* p_mark, xy_t size_out, xyd_t xyd_crr);

// ======= print maze =====
void print_maze_by_ptr(int* p_maze, xy_t size_in);


// ====== debugging =======


// global var
xy_t g_move[DIR_COUNT] = {
    { 1, 1 }, // SOUTH_EAST = 0,
    { 1, 0 }, // SOUTH = 1,
    { 0, 1 }, // EAST = 2,
    { -1, 1 }, // NORTH_EAST = 3,
    { 1, -1 }, // SOUTH_WEST = 4,
    { 0, -1 }, //WEST = 5,
    { -1, 0 }, //NORTH = 6,
    { -1, -1}, // NORTH_WEST = 7
};


int main(void)
{
    int* p_maze = NULL;
    int* p_mark = NULL;
    xy_t size_in = get_maze_size();
    
    xy_t size_out;
    size_out.x = size_in.x + 2;
    size_out.y = size_in.y + 2;

    xyd_t xyd_des;
    xyd_des.xy.x = size_in.x;
    xyd_des.xy.y = size_in.y;

    p_maze = (int*)malloc(size_out.x * size_out.y * sizeof(int));
    p_mark = (int*)malloc(size_out.x * size_out.y * sizeof(int));

    load_maze(p_maze, size_in);
    printf("<<INPUT MAZE>>\n");
    print_maze_by_ptr(p_maze, size_in);

    deep_copy_maze(p_maze, p_mark, size_out);
    if (DEBUG) {
        printf("<<MARK MAZE>>\n");
        print_maze_by_ptr(p_mark, size_in);
    }

    path_t* path_top = NULL;
    path_t* cross_top = NULL;

    dir_t dir_pre = SOUTH_EAST; // 첫 노드는 임시로 SOUTH_EAST
    xyd_t xyd_crr;
    xyd_crr.xy.x = 1;
    xyd_crr.xy.y = 1;
    xyd_crr.dir = search_dir(p_maze, p_mark, size_out, xyd_crr, dir_pre);
    int dir_opt_count = count_direction_opt(p_maze, p_mark, size_out, xyd_crr, dir_pre);

    int d_j = 0;
    while (TRUE) {
        if (dir_opt_count > 1) {
            push(&path_top, xyd_crr);
            push(&cross_top, xyd_crr);
        } else if (dir_opt_count == 1) {
            push(&path_top, xyd_crr);
        } else if (dir_opt_count == 0) {
            while (cross_top->xyd.xy.x != path_top->xyd.xy.x
            && cross_top->xyd.xy.y != path_top->xyd.xy.y) {
                xyd_crr = pop(&path_top);
                delete_mark_path(p_mark, size_out, xyd_crr);
            }
            pop(&cross_top);
            xyd_crr = pop(&path_top);
        }

        dir_pre = xyd_crr.dir;
        xyd_crr.xy = move_xy(xyd_crr.xy, dir_pre);
        xyd_crr.dir = search_dir(p_maze, p_mark, size_out, xyd_crr, dir_pre);
        dir_opt_count = count_direction_opt(p_maze, p_mark, size_out, xyd_crr, dir_pre);

        if (xyd_crr.xy.x == xyd_des.xy.x && xyd_crr.xy.y == xyd_des.xy.y ) {
            push(&path_top, xyd_crr);
            break;
        }

        if (DEBUG) {
            d_j++;
            printf("[%d]번째 [운동이름] 이동\n", d_j);
            print_maze_by_ptr(p_mark, size_in);
            if (d_j > 10) {
                break;
            }
        }
    }

    return 0;
}


// ======== stack ========
void push(path_t** ptop, xyd_t xyd)
{
    path_t* new_path;
    new_path = malloc(sizeof(path_t));

    new_path->xyd = xyd;
    new_path->link = *ptop;
    *ptop = new_path;
}

xyd_t pop(path_t** ptop)
{
    xyd_t pre_pos;
    path_t* top_node;
    top_node = *ptop;

    pre_pos = top_node->xyd;
    *ptop = top_node->link;
    top_node->link = NULL;
    free(top_node);

    return pre_pos;
}

// =======================



// =====maze setting=====
xy_t get_maze_size(void)
{
    xy_t size = { 0, };
    int ch;

    FILE* file = fopen("maze.txt", "r");
    if (file == NULL) {
        perror("error while opening");
        return size;
    }

    ch = fgetc(file);
    while (ch != '\n') {
        size.x++;
        ch = fgetc(file);
    }

    fseek(file, 0, SEEK_END);
    size.y = ftell(file) / (size.x + 1) + 1;

    fclose(file);

    return size;
}

void load_maze(int* p_maze, xy_t size_in)
{
    int ch;
    int r, c;

    FILE* file = fopen("maze.txt", "r");
    if (file == NULL) {
        perror("error while opening");
        return;
    }

    ch = fgetc(file);
    for (r = 0; r < size_in.y; ++r) {
        for (c = 0; c < size_in.x; ++c) {
            *(p_maze + (r + 1) * (size_in.x + 2) + c + 1) = ch - ASCII_ZERO;
            ch = fgetc(file);
            if (ch == '\n') {
                ch = fgetc(file);
            }
        }
        *(p_maze + (r + 1) * (size_in.x + 2)) = NONPATH;
        *(p_maze + (r + 1) * (size_in.x + 2) + (size_in.x + 1)) = NONPATH;
    }

    for (c = 0; c < size_in.y + 2; ++c) {
        *(p_maze + c) = NONPATH;
        *(p_maze + (size_in.y + 1) * (size_in.x + 2) + c) = NONPATH;
    }

    fclose(file);
}

void deep_copy_maze(int* p_maze, int* p_mark, xy_t size_out)
{
    int* p_maze_tmp = p_maze;
    int* p_mark_tmp = p_mark;
    int count = size_out.x * size_out.y;
    int i;
    for (i = 0; i < count; ++i) {
        *p_mark_tmp++ = *p_maze_tmp++;
    }
}

// =======================


// ======find path ======
dir_t search_dir(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre)
{
    int* maze_curr = p_maze + xyd_crr.xy.x + xyd_crr.xy.y * size_out.x;
    int* mark_curr = p_mark + xyd_crr.xy.x + xyd_crr.xy.y * size_out.x;
    int* maze_next;
    int* mark_next;
    dir_t from_dir = REVERSE_DIR - dir_pre;

    dir_t dir;

    for (dir = 0; dir < DIR_COUNT; ++dir) {
        maze_next = maze_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        mark_next = mark_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        if (*maze_next == PATH && *mark_next == PATH && dir != from_dir) {
            return dir;
        }
    }
}

int count_direction_opt(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre)
{
    int* maze_curr = p_maze + xyd_crr.xy.x + xyd_crr.xy.y * size_out.x;
    int* mark_curr = p_mark + xyd_crr.xy.x + xyd_crr.xy.y * size_out.x;
    int* maze_next;
    int* mark_next;
    dir_t from_dir = REVERSE_DIR - dir_pre;

    dir_t dir;
    int count = 0;

    for (dir = 0; dir < DIR_COUNT; ++dir) {
        maze_next = maze_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        mark_next = mark_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        if (*maze_next == PATH && *mark_next == PATH && dir != from_dir) {
            ++count;
        }
    }

    return count;
}

xy_t move_xy(xy_t xy_crr, dir_t dir_pre)
{
    xy_t xy_next;
    xy_next.x = xy_crr.x + g_move[dir_pre].x;
    xy_next.y = xy_crr.y + g_move[dir_pre].y;

    return xy_next;
}

void delete_mark_path(int* p_mark, xy_t size_out, xyd_t xyd_crr)
{
    int* mark_curr = p_mark + xyd_crr.xy.y * size_out.x + xyd_crr.xy.x;
    *mark_curr = 'X';

    return;
}

// =======================


// ======= print maze =====
void print_maze_by_ptr(int* p_maze, xy_t size_in)
{
    int ch;
    int r, c;

    for (r = 0; r < size_in.y; ++r) {
        for (c = 0; c < size_in.x; ++c) {
            ch = *(p_maze + (r + 1) * (size_in.x + 2) + c + 1);
            printf("%d", ch);
        }
        printf("\n");
    }
    printf("\n");
}

// =======================




// ====== debugging =======


// =======================
