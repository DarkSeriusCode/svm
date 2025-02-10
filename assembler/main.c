#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include "io.h"
#include "common/vector.h"
#include "common/arch.h"
#include "image.h"
#include "parser.h"
#include "error.h"

struct argp_option options[] = {
    { 0, 'o', "OUTPUT_FILE", 0, "Name of the output file.", 0 },
    { 0, 't', 0, 0, "Shows tokens in the input file.", 0 },
    { 0, 'i', 0, 0, "Shows the symbol tabel and hex of the output file", 0 },
    { 0 }
};

const char *INPUT_FILE_NAME;
char *OUTPUT_FILE_NAME = "a.out";
bool SHOW_TOKENS = false;
bool SHOW_IMAGE = false;

int parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'o':
            if (arg) {
                OUTPUT_FILE_NAME = arg;
            }
        break;
        case 't': SHOW_TOKENS = true; break;
        case 'i': SHOW_IMAGE = true; break;

        case ARGP_KEY_ARG:
            INPUT_FILE_NAME = arg;
        break;

        case ARGP_KEY_END:
            if (INPUT_FILE_NAME == NULL) {
                argp_usage(state);
            }
        break;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int file_count = 1;
    struct argp argp = { options, parse_opt, "input_file", 0, 0, 0, 0 };
    int exit_code = argp_parse(&argp, argc, argv, 0, 0, &file_count);
    if (exit_code) { return exit_code; }

    Image image = new_image();
    image_add_declaration(&image, ENTRY_POINT_NAME, 0);
    vector_push_back_many(image.data, byte, 0, 0)

    Parser parser = new_parser(INPUT_FILE_NAME);
    if (SHOW_TOKENS) {
        for (size_t i = 0; i < vector_size(parser.tokens); i++) {
            print_token(parser.tokens[i]);
        }
    }
    vector(Label) labels = NULL;
    vector_set_destructor(labels, free_label);
    for (Label lbl = parse_label(&parser); lbl.name != NULL; lbl = parse_label(&parser)) {
        if (lbl.is_empty) {
            warning_empty_label(lbl);
            continue;
        }
        vector_push_back(labels, lbl);
    }
    // Data labels
    for (size_t i = 0; i < vector_size(labels); i++) {
        Label lbl = labels[i];
        if (!lbl.is_data) continue;
        if (strcmp(lbl.name, ENTRY_POINT_NAME) == 0) {
            printf("In %s: _main label cannot contain data!\n", INPUT_FILE_NAME);
            exit(EXIT_FAILURE);
        }
        image_add_definition(&image, lbl.name, image_content_size(image));
        for (size_t j = 0; j < vector_size(lbl.declarations); j++) {
            image_add_data(&image, lbl.declarations[j]);
        }
    }
    // Code labels
    for (size_t i = 0; i < vector_size(labels); i++) {
        Label lbl = labels[i];
        if (lbl.is_data) continue;
        image_add_definition(&image, lbl.name, image_content_size(image));
        for (size_t j = 0; j < vector_size(lbl.instructions); j++) {
            image_add_code(&image, lbl.instructions[j]);
        }
    }

    if (SHOW_IMAGE) {
        print_image(image);
    }
    image_check_unresolved_names(image);
    Symbol *entry_point = image_get_symbol(image, "_main");
    if (!entry_point || !entry_point->is_resolved) {
        printf("In %s: Cannot generate a program image: no _main label!\n", INPUT_FILE_NAME);
        exit(EXIT_FAILURE);
    }
    dump_image(image, OUTPUT_FILE_NAME);

    free_vector(labels);
    free_parser(&parser);
    free_image(&image);

    return 0;
}
