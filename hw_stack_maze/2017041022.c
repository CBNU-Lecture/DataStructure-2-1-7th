#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ASCII_ZERO ('0')
#define DIR_COUNT (8)
#define REVERSE_DIR (7)

#define TIME_CHECK (0)
#define DEBUG (0)

enum {
    TRUE = 1,
    FALSE = 0
};

enum {
    PATH = 0,
    NONPATH = 1
};

typedef enum {
    EAST = 0,
    SOUTH_EAST = 1,
    SOUTH = 2,
    SOUTH_WEST = 3,
    WEST = 4,
    NORTH_WEST = 5,
    NORTH = 6,
    NORTH_EAST = 7
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
xy_t get_maze_size_in(void);
void load_maze(int* p_maze, int* p_mark, xy_t size_in);
void load_solv(int* p_solv, xy_t size_out);
void set_solution(int* p_solv, xy_t size_out, path_t** p_top);

// ======find path ======
dir_t search_dir(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre);
int count_direction_opt(int* p_maze, int* p_mark, xy_t size_out, xyd_t xyd_crr, dir_t dir_pre);
xy_t move_xy(xy_t xy_crr, dir_t dir_pre);
void delete_mark_path(int* p_mark, xy_t size_out, xyd_t xyd_crr);

// ======= print maze =====
void print_maze_by_ptr(int* p_maze, xy_t size_in);
void print_maze_solution(int* p_solv, xy_t size_in, path_t* path_top);

// global var
xy_t g_move[DIR_COUNT] = {
    { 1, 0 }, // EAST = 0,
    { 1, 1 }, // SOUTH_EAST = 1,
    { 0, 1 }, // SOUTH = 2,
    { -1, 1 }, // SOUTH_WEST = 3,
    { -1, 0 }, // WEST = 4,
    { -1, -1 }, // NORTH_WEST = 5,
    { 0, -1 }, // NORTH = 6,
    { 1, -1 }, // NORTH_EAST = 7
};

path_t* path_top = NULL;
path_t* cross_top = NULL;
path_t* path_sol = NULL;

int main(void)
{
    clock_t start, end;
    start = clock();

    int* p_maze = NULL;
    int* p_mark = NULL;
    int* p_solv = NULL;

    xy_t size_in = get_maze_size_in();
    xy_t size_out;
    size_out.x = size_in.x + 2;
    size_out.y = size_in.y + 2;

    xyd_t xyd_des;
    xyd_des.xy.x = size_in.x;
    xyd_des.xy.y = size_in.y;

    p_maze = (int*)malloc(size_out.x * size_out.y * sizeof(int));
    p_mark = (int*)malloc(size_out.x * size_out.y * sizeof(int));
    p_solv = (int*)malloc(size_out.x * size_out.y * sizeof(int));

    load_maze(p_maze, p_mark, size_in);
    load_solv(p_solv, size_out);

    printf("<<<INPUT MAZE>>>\n");
    print_maze_by_ptr(p_maze, size_in);

    if (TIME_CHECK) {
        end = clock();
        printf("time to loading maze : %f\n", (double)(end - start));
    }

    dir_t dir_pre = SOUTH_EAST; // 첫 노드는 임시로 SOUTH_EAST
    xyd_t xyd_crr;
    xyd_crr.xy.x = 1;
    xyd_crr.xy.y = 1;
    xyd_crr.dir = search_dir(p_maze, p_mark, size_out, xyd_crr, dir_pre);
    int dir_opt_count = count_direction_opt(p_maze, p_mark, size_out, xyd_crr, dir_pre);

    while (TRUE) {
        if (dir_opt_count > 1) {
            push(&path_top, xyd_crr);
            delete_mark_path(p_mark, size_out, xyd_crr);
            push(&cross_top, xyd_crr);
        } else if (dir_opt_count == 1) {
            push(&path_top, xyd_crr);
            delete_mark_path(p_mark, size_out, xyd_crr);
        } else if (dir_opt_count == 0) {
            while (cross_top->xyd.xy.x != path_top->xyd.xy.x
            && cross_top->xyd.xy.y != path_top->xyd.xy.y) {
                xyd_crr = pop(&path_top);
                delete_mark_path(p_mark, size_out, xyd_crr);
            }
            pop(&cross_top);
            dir_pre = xyd_crr.dir;
            xyd_crr.dir = search_dir(p_maze, p_mark, size_out, xyd_crr, dir_pre);
            dir_opt_count = count_direction_opt(p_maze, p_mark, size_out, xyd_crr, dir_pre);
            continue;
        }

        dir_pre = xyd_crr.dir;
        xyd_crr.xy = move_xy(xyd_crr.xy, dir_pre);
        xyd_crr.dir = search_dir(p_maze, p_mark, size_out, xyd_crr, dir_pre);
        dir_opt_count = count_direction_opt(p_maze, p_mark, size_out, xyd_crr, dir_pre);

        if (xyd_crr.xy.x == xyd_des.xy.x && xyd_crr.xy.y == xyd_des.xy.y ) {
            push(&path_top, xyd_crr);
            printf("<<<SOLUTION>>>\n");
            break;
        }
    }

    set_solution(p_solv, size_out, &path_top);

    print_maze_by_ptr(p_solv, size_in);

    if (TIME_CHECK) {
        end = clock();
        printf("time to complete detecting path : %f\n", (double)(end - start));
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
    // free(top_node);

    return pre_pos;
}

void reverse_stack(path_t** p_top1, path_t** p_top2)
{
    xyd_t xyd_tmp;
    while (*p_top1 != NULL) {
        xyd_tmp = pop(p_top1);
        push(p_top2, xyd_tmp);
    }
}

// =======================


// =====maze setting=====
xy_t get_maze_size_in(void)
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

void load_maze(int* p_maze, int* p_mark, xy_t size_in)
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
            *(p_mark + (r + 1) * (size_in.x + 2) + c + 1) = ch - ASCII_ZERO;
            ch = fgetc(file);
            if (ch == '\n') {
                ch = fgetc(file);
            }
        }
        *(p_maze + (r + 1) * (size_in.x + 2)) = NONPATH;
        *(p_maze + (r + 1) * (size_in.x + 2) + (size_in.x + 1)) = NONPATH;
        *(p_mark + (r + 1) * (size_in.x + 2)) = NONPATH;
        *(p_mark + (r + 1) * (size_in.x + 2) + (size_in.x + 1)) = NONPATH;
    }

    for (c = 0; c < size_in.y + 2; ++c) {
        *(p_maze + c) = NONPATH;
        *(p_maze + (size_in.y + 1) * (size_in.x + 2) + c) = NONPATH;
        *(p_mark + c) = NONPATH;
        *(p_mark + (size_in.y + 1) * (size_in.x + 2) + c) = NONPATH;
    }

    fclose(file);
}

void load_solv(int* p_solv, xy_t size_out)
{
    int r, c;

    for (r = 0; r < size_out.y; ++r) {
        for (c = 0; c < size_out.x; ++c) {
            *(p_solv + r * size_out.x + c) = 1;
        }
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

    dir_t from_dir = (dir_pre + DIR_COUNT / 2) % 8;

    dir_t dir;
    for (dir = 0; dir < DIR_COUNT; ++dir) {
        maze_next = maze_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        mark_next = mark_curr + g_move[dir].y * size_out.x + g_move[dir].x;
        if (*maze_next == PATH && *mark_next == PATH 
        && dir != (from_dir + 1) % DIR_COUNT && dir != (from_dir - 1 + DIR_COUNT) % DIR_COUNT
        && dir != from_dir) {
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
    dir_t from_dir = (dir_pre + DIR_COUNT / 2) % 8;

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
    *mark_curr = '~' - ASCII_ZERO;

    return;
}

// =======================


// ======= print maze =====
void print_maze_by_ptr(int* p_maze, xy_t size_in)
{
    int ch;
    int r, c;

    for (r = 1; r <= size_in.y; ++r) {
        for (c = 1; c <= size_in.x; ++c) {
            ch = *(p_maze + r * (size_in.x + 2) + c);
            printf("%c", ch + ASCII_ZERO);
        }
        printf("\n");
    }
    printf("\n");
}


void set_solution(int* p_solv, xy_t size_out, path_t** p_top)
{
    path_t* tmp_top;
    tmp_top = *p_top;

    while (tmp_top != NULL) {
        *(p_solv + (tmp_top->xyd.xy.y * size_out.x) + tmp_top->xyd.xy.x) = PATH;
        tmp_top = tmp_top->link;
    }
}