#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 10000
#define MAX_LINE_LEN 1024

typedef struct {
    int line;
    int col;
} Position;

char *lines[MAX_LINES];
bool typed_map[MAX_LINES][MAX_LINE_LEN]; // Track correct characters
int num_lines = 0;

Position cursor = {0, 0};
int desired_col = 0;
short COLOR_GRAY_BLUE = 8;  // Custom color ID

int load_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), fp) && num_lines < MAX_LINES) {
        lines[num_lines] = strdup(buffer);
        num_lines++;
    }

    fclose(fp);
    return 1;
}

void draw_screen(int top_line, int screen_rows, int screen_cols) {
    (void)screen_cols;  // silence unused parameter warning
    clear();
    for (int i = 0; i < screen_rows - 1 && (top_line + i) < num_lines; i++) {
        int lineno = top_line + i;
        int len = strlen(lines[lineno]);
        move(i, 0);
        for (int j = 0; j < len; j++) {
            if (typed_map[lineno][j]) {
                attron(COLOR_PAIR(2));
            } else {
                attron(COLOR_PAIR(1));
            }
            addch(lines[lineno][j]);
            attroff(COLOR_PAIR(1));
            attroff(COLOR_PAIR(2));
        }
    }

    if (cursor.line >= top_line && cursor.line < top_line + screen_rows - 1) {
        int y = cursor.line - top_line;
        int line_len = strlen(lines[cursor.line]);
        int x = cursor.col;
        if (x >= line_len) x = line_len - 1;
        if (x >= 0) {
            attron(A_REVERSE);
            mvaddch(y, x, lines[cursor.line][x]);
            attroff(A_REVERSE);
        }
        move(y, cursor.col);
    }
    refresh();
}

char current_char() {
    if (cursor.line >= num_lines) return '\0';
    int line_len = strlen(lines[cursor.line]);
    if (cursor.col >= line_len) return '\0';  // past end of line

    return lines[cursor.line][cursor.col];
}


void advance_cursor() {
    if (cursor.line >= num_lines) return;
    int line_len = strlen(lines[cursor.line]);
    if (cursor.col + 1 < line_len) {
        cursor.col++;
    } else if (cursor.line + 1 < num_lines) {
        cursor.line++;
        cursor.col = 0;
    }
    desired_col = cursor.col;
}

void move_cursor_up() {
    if (cursor.line > 0) {
        cursor.line--;
        int line_len = strlen(lines[cursor.line]);
        cursor.col = (desired_col < line_len) ? desired_col : (line_len > 0 ? line_len - 1 : 0);
    }
}

void move_cursor_down() {
    if (cursor.line + 1 < num_lines) {
        cursor.line++;
        int line_len = strlen(lines[cursor.line]);
        cursor.col = (desired_col < line_len) ? desired_col : (line_len > 0 ? line_len - 1 : 0);
    }
}

void move_cursor_left() {
    if (cursor.col > 0) {
        cursor.col--;
        desired_col = cursor.col;
    } else if (cursor.line > 0) {
        cursor.line--;
        int line_len = strlen(lines[cursor.line]);
        cursor.col = (line_len > 0) ? line_len - 1 : 0;
        desired_col = cursor.col;
    }
}

void move_cursor_right() {
    int line_len = strlen(lines[cursor.line]);
    if (cursor.col + 1 < line_len) {
        cursor.col++;
        desired_col = cursor.col;
    } else if (cursor.line + 1 < num_lines) {
        cursor.line++;
        cursor.col = 0;
        desired_col = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s filename.txt [start_line]\n", argv[0]);
        return 1;
    }

    if (!load_file(argv[1])) {
        fprintf(stderr, "Failed to open file: %s\n", argv[1]);
        return 1;
    }

    memset(typed_map, 0, sizeof(typed_map));

    if (argc == 3) {
        int start_line = atoi(argv[2]) - 1;
        if (start_line >= 0 && start_line < num_lines) {
            cursor.line = start_line;
        }
    }
    desired_col = cursor.col;

    initscr();
	cbreak();  // ALLOW Ctrl+C and Ctrl+Z
    keypad(stdscr, TRUE);
    noecho();
    start_color();

    if (COLORS >= 16) {
        init_color(COLOR_GRAY_BLUE, 400, 400, 600); // light bluish-grey
    } else {
        COLOR_GRAY_BLUE = COLOR_CYAN; // fallback if limited colors
    }

    init_pair(1, COLOR_WHITE, COLOR_BLACK);       // Normal text
    init_pair(2, COLOR_GRAY_BLUE, COLOR_BLACK);   // Correctly typed text

    int ch;
    int top_line = (cursor.line > 0) ? cursor.line - 1 : 0;

    while (1) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        if (top_line < 0) top_line = 0;
        if (top_line > num_lines - rows + 1) top_line = num_lines - rows + 1;

        draw_screen(top_line, rows, cols);

        ch = getch();
        if (ch == 27) break; // ESC to exit

        if (ch == KEY_UP) {
            move_cursor_up();
        } else if (ch == KEY_DOWN) {
            move_cursor_down();
        } else if (ch == KEY_LEFT) {
            move_cursor_left();
        } else if (ch == KEY_RIGHT) {
            move_cursor_right();
		} else if (ch == current_char()) {
		    typed_map[cursor.line][cursor.col] = true;
		    advance_cursor();
		} else if (ch == '\n' || ch == KEY_ENTER) {
		    int line_len = strlen(lines[cursor.line]);
		    if (cursor.col >= line_len || line_len == 0) {
		        cursor.line++;
		        cursor.col = 0;
		        desired_col = 0;
		    } else {
		        beep(); // can't enter yet, still text to type
		    }
		} else {
		    beep();
		}

        // Scroll view to follow cursor
        if (cursor.line < top_line) top_line = cursor.line;
        if (cursor.line >= top_line + rows - 1) top_line = cursor.line - rows + 2;
    }

    endwin();
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }

    return 0;
}
