#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

typedef struct colored_symb {
    char symb;
    int color;
} colored_symb;

typedef struct colored_symb_array {
    colored_symb *array;
    int size;
} colored_symb_array;

int custom_random(int min, int max) { return min + rand() / (RAND_MAX / (max - min + 1) + 1); }

int render_text(colored_symb_array *text, int width, int height) {
    printf("\033[H");
    fflush(stdout);
    for (int i = 0; i < text->size; i++) {
        printf("\x1b[%dm%c\x1b[0m", text->array[i].color, text->array[i].symb);
    }
    fflush(stdout);
    return 0;
}

colored_symb_array *build_bash_text(char *text) {
    colored_symb_array *output = malloc(sizeof(colored_symb_array));
    output->size = strlen(text);
    output->array = malloc(output->size * sizeof(colored_symb));
    for (int i = 0; i < strlen(text); i++) {
        output->array[i].color = 2;
        output->array[i].symb = text[i];
    }
    return output;
}

char *build_text(int word_count) {
    int lines_count = 0;
    char *text = malloc(sizeof(char));
    text[0] = '\0';
    FILE *f = fopen("word_list.txt", "r");
    if (f == NULL) {
        perror("error opening a file");
        _exit(1);
    }
    char buf[128];
    while (fgets(buf, 15, f) > 0) {
        lines_count++;
    }
    for (int i = 0; i < word_count; i++) {
        fseek(f, 0, SEEK_SET);
        int line_n = custom_random(0, lines_count - 1);
        for (int i = 0; i <= line_n; i++) {
            fgets(buf, 15, f);
        }
        text = realloc(text, (strlen(text) + strlen(buf) + 1) * sizeof(char));
        if (text == NULL) {
            perror("couldn't realloc");
            _exit(1);
        }
        if (i != word_count - 1) {
            buf[strlen(buf) - 1] = ' ';
        } else {
            buf[strlen(buf) - 1] = '\0';
        }
        strcat(text, buf);
    }
    puts(text);
    return text;
}

void set_color(colored_symb_array *colored_text, int color, int letter) { colored_text->array[letter].color = color; }

int main(int argc, char **argv) {
    int word_count = 30;
    if (argc == 2) {
        word_count = atoi(argv[1]);
    }
    srand(time(NULL));

    struct winsize w;
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0) {
        perror("ioctl");
        return 1;
    }

    char *text = build_text(word_count);
    colored_symb_array *output = build_bash_text(text);
    char symb;
    int curr = 0;
    system("clear");
    time_t start = time(NULL);
    while (1) {
        render_text(output, w.ws_col, w.ws_row);
        symb = getchar();
        if (curr == 0) {
            time_t start = time(NULL);
        }
        if (symb == '\b' || symb == 127) {
            if (curr == 0) {
                continue;
            }
            curr -= 1;
            set_color(output, 2, curr);
            continue;
        }
        if (symb == text[curr]) {
            set_color(output, 42, curr);
        } else {
            set_color(output, 41, curr);
        }
        if (curr == output->size - 1) {
            break;
        }
        curr++;
    }
    time_t end = time(NULL);
    double delta = ((double) (int) end - (double) (int) start);
    system("clear");
    printf("delta: %f\n", delta);
    printf("size: %d\n", output->size);
    printf("wpm: %f\n", ((double) output->size / 5) * (60 / (double) delta));
    printf("lpm: %f\n", (double) output->size * (60 / (double) delta));
    free(output->array);
    free(output);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}
