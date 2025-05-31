#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 10000
#define MAX_LINE_LEN 1024
#define MAX_SKIPS 1024

typedef struct {
    int line;
    int col;
} Position;

char *lines[MAX_LINES];
int num_lines = 0;

Position cursor = {0, 0};
Position skip_stack[MAX_SKIPS];
int skip_top = -1;

int load_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), fp) && num_lines < MAX_LINES) {
        lines[num_lines] = strdup(buffer);
        lines[num_lines][strcspn(lines[num_lines], "\n")] = '\0';
        num_lines++;
    }

    fclose(fp);
    return 1;
}

void draw_screen(int top_line, int screen_rows, int screen_cols) {
    clear();
    for (int i = 0; i < screen_rows - 1 && (top_line + i) < num_lines; i++) {
        mvprintw(i, 0, "%s", lines[top_line + i]);
    }
    move(cursor.line - top_line, cursor.col);
    refresh();
}

char current_char() {
    if (cursor.line >= num_lines) return '\0';
    if (cursor.col >= strlen(lines[cursor.line])) return '\n';
    return lines[cursor.line][cursor.col];
}

void advance_cursor() {
    if (cursor.line >= num_lines) return;
    if (cursor.col + 1 < strlen(lines[cursor.line])) {
        cursor.col++;
    } else {
        cursor.line++;
        cursor.col = 0;
    }
}

void seek_forward(char ch) {
    for (int l = cursor.line; l < num_lines; l++) {
        int start = (l == cursor.line) ? cursor.col + 1 : 0;
        char *p = strchr(&lines[l][start], ch);
        if (p) {
            skip_stack[++skip_top] = cursor;
            cursor.line = l;
            cursor.col = p - lines[l];
            return;
        }
    }
    // If not found, beep
    beep();
}

void pop_skip() {
    if (skip_top >= 0) {
        cursor = skip_stack[skip_top--];
    } else {
        // Standard backspace behavior
        if (cursor.col > 0) {
            cursor.col--;
        } else if (cursor.line > 0) {
            cursor.line--;
            cursor.col = strlen(lines[cursor.line]);
        }
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

    if (argc == 3) {
        int start_line = atoi(argv[2]) - 1;
        if (start_line >= 0 && start_line < num_lines) {
            cursor.line = start_line;
        }
    }

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    int ch;
    int top_line = (cursor.line > 0) ? cursor.line - 1 : 0;

    while (1) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        // Scroll if needed
        if (cursor.line < top_line) top_line = cursor.line;
        if (cursor.line >= top_line + rows - 1) top_line = cursor.line - rows + 2;

        draw_screen(top_line, rows, cols);

        ch = getch();

        if (ch == 27) break; // ESC to exit
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            pop_skip();
        } else if (ch == current_char()) {
            advance_cursor();
        } else {
            seek_forward(ch);
        }
    }

    endwin();

    // Cleanup
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }

    return 0;
}
