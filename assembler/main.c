#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include "io.h"
#include "common/vector.h"
#include "image.h"
#include "parser.h"

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

    vector(int) v = NULL;
    vector_push_back_many(v, int, 1, 4, 8, 8, 5, 4, 7, 0);

    Image image = new_image();
    // Reserve a word for pointer to _main
    vector(byte) ptr_to_main = NULL;
    vector_push_back_many(ptr_to_main, byte, 0, 0);
    image_add_data(&image, ptr_to_main);
    image_add_declaration(&image, "_main", 0);

    Parser parser = new_parser(INPUT_FILE_NAME);
    if (SHOW_TOKENS) {
        for (size_t i = 0; i < vector_size(parser.tokens); i++) {
            print_token(parser.tokens[i]);
        }
    }
    parse_data_section(&parser, &image);
    parse_code_section(&parser, &image);
    image_analize(image);

    if (SHOW_IMAGE) {
        print_image(image);
    }
    if (!image_get_symbol(image, "_main")->is_resolved) {
        printf("In %s: Cannot generate a program image: no _main label!\n", INPUT_FILE_NAME);
    }
    dump_image(image, OUTPUT_FILE_NAME);

    free_parser(&parser);
    free_image(&image);

    return 0;
}
