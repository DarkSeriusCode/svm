#define VECTOR_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "io.h"
#include "common/vector.h"
#include "common/arch.h"
#include "image.h"
#include "parser.h"
#include "analysis.h"

const char *INPUT_FILE_NAME;
char *OUTPUT_FILE_NAME = "a.out";
bool SHOW_TOKENS = false;
bool SHOW_IMAGE = false;

void print_help(const char *name);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 0;
    }
    int res = 0;

    INPUT_FILE_NAME = argv[argc - 1];
    while ( (res = getopt(argc, argv, "hito:")) != -1 ) {
        switch (res) {
            case 'h':
                print_help(argv[0]);
                return 0;
            case '?':
                return 1;
            case 't':
                SHOW_TOKENS = true;
                break;
            case 'i':
                SHOW_IMAGE = true;
                break;
            case 'o':
                OUTPUT_FILE_NAME = optarg;
        }
    }

    Image image = new_image();
    image_add_usage(&image, ENTRY_POINT_NAME, 0);
    vector_push_back_many(image.buffer, byte, 0, 0)

    Parser parser = new_parser(INPUT_FILE_NAME);
    if (SHOW_TOKENS) {
        for (size_t i = 0; i < vector_size(parser.tokens); i++) {
            print_token(parser.tokens[i]);
        }
    }
    for (Label lbl = parse_label(&parser); lbl.name != NULL; lbl = parse_label(&parser)) {
        if (lbl.is_empty) {
            warning_empty_label(lbl);
            continue;
        }
        image_add_label(&image, lbl);
    }
    analyse_program(image.labels);

    image_codegen(&image);

    if (SHOW_IMAGE) {
        print_image(image);
    }
    Symbol *entry_point = image_get_symbol(image, ENTRY_POINT_NAME);
    if (!entry_point || !entry_point->is_resolved) {
        printf("In %s: Cannot generate a program image: no "ENTRY_POINT_NAME" label!", INPUT_FILE_NAME);
        printf(" Try add:\n_main:\n<your code goes here>\nret\n");
        exit(EXIT_FAILURE);
    }
    image_check_unresolved_names(image);
    dump_image(image, OUTPUT_FILE_NAME);

    free_parser(&parser);
    free_image(&image);

    return 0;
}

void print_help(const char *name) {
    printf("%s - an assembler for svm.\n", name);
    printf("Usage: %s [options] file\n", name);
    printf("Options:\n");
    printf("  -h          Prints this message and exit\n");
    printf("  -o <file>   Place output to <file>\n");
    printf("  -i          Prints information about file\n");
    printf("  -t          Prints token tree\n");
}
