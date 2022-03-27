#include <stdio.h>
#include <stdlib.h>

#define ASCII_ZERO ('0')
#define DIR_COUNT (8)

typedef enum {
    TRUE = 1,
    FALSE = 0
} bool_t;

typedef enum {
    PATH = 0,
    NONPATH = 1
} wall_t;

typedef struct {
    int row;
    int col;
} pos_t;

typedef enum {
    NORTH = 0,
    NORTH_EAST = 1,
    EAST = 2,
    SOUTH_EAST = 3,
    SOUTH = 4,
    SOUTH_WEST = 5,
    WEST = 6,
    NORTH_WEST = 7
} direction_t;

pos_t g_move[8] = {
    {.row = -1, .col = 0},
    {.row = -1, .col = 1},
    {.row = 0, .col = 1},
    {.row = 1, .col = 1},
    {.row = 1, .col = 0},
    {.row = -1, .col = -1},
    {.row = 0, .col = -1},
    {.row = -1, .col = -1}
};

typedef struct {
    int row;
    int col;
    direction_t dir;
} path_t;

typedef struct {
    path_t* path;
    int capacity;
    int top;
} stack_t;

pos_t get_maze_size(void);
void load_maze(int* p_maze_in, pos_t maze_size);
void print_maze(int* p_maze_in, pos_t maze_size);
void deep_copy_maze(int* p_maze_in, int* p_mark, pos_t maze_size);

void init_stack(stack_t* s);
int is_full(stack_t* s);
int is_empty(stack_t* s);
void push(stack_t* s, path_t point);
path_t pop(stack_t* s);
void free_stack(stack_t* s);

direction_t detect_path_start(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point);
int detect_path(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point, direction_t footprint);
void delete_mark(int* p_mark, pos_t maze_size, path_t point);
int count_optional_path_start(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point);
int count_optional_path(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point, direction_t footprint);

int main(void)
{
    int* p_maze_in = NULL;
    pos_t maze_size = { 0, 0 };
    int* p_mark = NULL;

    maze_size = get_maze_size();

    p_maze_in = (int*)malloc((maze_size.row + 2) * (maze_size.col + 2) * sizeof(int));

    load_maze(p_maze_in, maze_size);
    printf("<<INPUT MAZE>>\n");
    print_maze(p_maze_in, maze_size);

    p_mark = (int*)malloc((maze_size.row + 2) * (maze_size.col + 2) * sizeof(int));

    deep_copy_maze(p_maze_in, p_mark, maze_size);

    stack_t* s;
    s = (stack_t*)malloc(sizeof(stack_t));
    init_stack(s);

    stack_t* cross;
    cross = (stack_t*)malloc(sizeof(stack_t));
    init_stack(cross);

    path_t point;
    point.row = 0;
    point.col = 0;

    path_t crossroad_point;
    
    if (count_optional_path_start(p_maze_in, p_mark, maze_size, point) > 1) {
        crossroad_point.row = point.row;
        crossroad_point.col = point.col;
        point.dir = detect_path_start(p_maze_in, p_mark, maze_size, point);
        crossroad_point.dir = point.dir;
        push(cross, crossroad_point);
        push(s, point);
    }

    printf("<<Start MAZE>>\n");
    print_maze(p_mark, maze_size);

    int i = 0;

    while (TRUE) {
        point.row += g_move[point.dir].row;
        point.col += g_move[point.dir].col;
        direction_t footprint = (point.dir + 4) % 8;
        point.dir = detect_path(p_maze_in, p_mark, maze_size, point, footprint);

        if (point.dir < 0) {
            while (crossroad_point.row != point.row && crossroad_point.col != point.col) {
                delete_mark(p_mark, maze_size, point);
                point = pop(s);
                printf("Roll Back...\n");
                print_maze(p_mark, maze_size); 
            }
            crossroad_point = pop(cross);
        }

        if (count_optional_path(p_maze_in, p_mark, maze_size, point, footprint) > 1) {
            crossroad_point = point;
            push(cross, crossroad_point);
        }

        if (point.row == maze_size.row && point.col == maze_size.col) {
            printf("Success MAZE\n");
            print_maze(p_mark, maze_size);
            return 0;
        }

        push(s, point);

        printf("<<Next Path>>\n");
        print_maze(p_mark, maze_size);

        i++;
        if (i > 5) {
            return 0;
        }
    }

    return 0;
}

pos_t get_maze_size(void)
{
    pos_t maze_size = { 0, 0, };
    int ch;

    FILE* file = fopen("maze.txt", "r");
    if (file == NULL) {
        perror("error while opening");
        return;
    }

    ch = fgetc(file);
    while (ch != '\n') {
        maze_size.col++;
        ch = fgetc(file);
    }

    fseek(file, 0, SEEK_END);
    maze_size.row = ftell(file) / (maze_size.col + 1) + 1;

    return maze_size;
}

void load_maze(int* p_maze_in, pos_t maze_size)
{
    int ch;
    int r, c;

    FILE* file = fopen("maze.txt", "r");
    if (file == NULL) {
        perror("error while opening");
        return;
    }

    ch = fgetc(file);
    for (r = 0; r < maze_size.row; ++r) {
        for (c = 0; c < maze_size.col; ++c) {
            *(p_maze_in + (r + 1) * (maze_size.col + 2) + c + 1) = ch - ASCII_ZERO;
            ch = fgetc(file);
            if (ch == '\n') {
                ch = fgetc(file);
            }
        }
        *(p_maze_in + (r + 1) * (maze_size.col + 2)) = NONPATH;
        *(p_maze_in + (r + 1) * (maze_size.col + 2) + (maze_size.col + 1)) = NONPATH;
    }

    for (c = 0; c < maze_size.row + 2; ++c) {
        *(p_maze_in + c) = NONPATH;
        *(p_maze_in + (maze_size.row + 1) * (maze_size.col + 2) + c) = NONPATH;
    }
}

void print_maze(int* p_maze_in, pos_t maze_size)
{
    int ch;
    int r, c;

    for (r = 0; r < maze_size.row; ++r) {
        for (c = 0; c < maze_size.col; ++c) {
            ch = *(p_maze_in + (r + 1) * (maze_size.col + 2) + c + 1);
            printf("%d", ch);
        }
        printf("\n");
    }
    printf("\n");
}

void deep_copy_maze(int* p_maze_in, int* p_mark, pos_t maze_size)
{
    int* p_maze_tmp = p_maze_in;
    int* p_mark_tmp = p_mark;
    int count = (maze_size.row + 2) * (maze_size.col + 2);
    int i;
    for (i = 0; i < count; ++i) {
        *p_mark_tmp++ = *p_maze_tmp++;
    }
}

void init_stack(stack_t* s)
{
    s->top = -1;
    s->capacity = 1;
    s->path = (path_t*)malloc(sizeof(path_t)*(s->capacity));
}

int is_full(stack_t* s) {
    return (s->top >= (s->capacity - 1));
}

int is_empty(stack_t* s) {
    return (s->top <= -1);
}

void push(stack_t* s, path_t point) {
    if (is_full(s)) {
        s->capacity *= 2;
        s->path = (path_t*)realloc(s->path, s->capacity * sizeof(path_t));
    }
    s->path[++(s->top)] = point;
}

path_t pop(stack_t* s) {
    if (is_empty(s)) {
        fprintf(stderr, "Stack is already empty\n");
        exit(1);
    }
    return s->path[(s->top)--];
}

void free_stack(stack_t* s) {
    free(s);
}

direction_t detect_path_start(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point)
{
    int* p_maze_crr = p_maze_in + (point.row + 1) * (maze_size.col + 2) + point.col;
    int* p_mark_crr = p_mark + (point.row + 1) * (maze_size.col + 2) + point.col;

    direction_t dir;

    for (dir = NORTH; dir <= NORTH_WEST; ++dir) {
        if (*(p_maze_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && *(p_mark_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH) {
            return dir;
        }
    }
}

int detect_path(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point, direction_t footprint)
{
    int* p_maze_crr = p_maze_in + (point.row + 1) * (maze_size.col + 2) + point.col;
    int* p_mark_crr = p_mark + (point.row + 1) * (maze_size.col + 2) + point.col;

    direction_t dir;

    for (dir = NORTH; dir <= NORTH_WEST; ++dir) {
        if (*(p_maze_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && *(p_mark_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && dir != footprint) {
            return dir;
        }
    }

    return -1;
}

void delete_mark(int* p_mark, pos_t maze_size, path_t point)
{
    int* p_mark_crr = p_mark + (point.row + 1) * (maze_size.col + 2) + point.col;
    // int* p_mark_dir_point = p_mark_crr + g_move[point.dir].row * (maze_size.col + 2) + g_move[point.dir].col;

    *p_mark_crr = NONPATH;
    // *p_mark_dir_point = NONPATH;
}

int count_optional_path_start(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point)
{
    int* p_maze_crr = p_maze_in + (point.row + 1) * (maze_size.col + 2) + point.col;
    int* p_mark_crr = p_mark + (point.row + 1) * (maze_size.col + 2) + point.col;
    int count = 0;
    direction_t dir;

    for (dir = NORTH; dir <= NORTH_WEST; ++dir) {
        if (*(p_maze_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && *(p_mark_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH) {
            ++count;
        }
    }

    return count;
}

int count_optional_path(int* p_maze_in, int* p_mark, pos_t maze_size, path_t point, direction_t footprint)
{
    int* p_maze_crr = p_maze_in + (point.row + 1) * (maze_size.col + 2) + point.col;
    int* p_mark_crr = p_mark + (point.row + 1) * (maze_size.col + 2) + point.col;
    int count = 0;
    direction_t dir;

    for (dir = NORTH; dir <= NORTH_WEST; ++dir) {
        if (*(p_maze_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && *(p_mark_crr + g_move[dir].row * (maze_size.col + 2) + g_move[dir].col) == PATH
        && dir != footprint) {
            ++count;
        }
    }

    return count;
}
