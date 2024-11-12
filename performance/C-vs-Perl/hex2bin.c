#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define OUTPUT_FILE "test.bin"

// Function to convert a single hex character to its numerical value
unsigned char hex_char_to_value(char c) {
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    else if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A' + 10);
    else {
        fprintf(stderr, "Invalid hex character: %c\n", c);
        exit(EXIT_FAILURE);
    }
}

// Function to convert hex string to binary data
int hex_to_binary(const char *hex_data, size_t hex_length, unsigned char *binary_data) {
    if (hex_length % 2 != 0) {
        fprintf(stderr, "Hex data length is not even.\n");
        exit(EXIT_FAILURE);
    }

	 fprintf(stderr, "hex_to_binary hex_data: %s\n", hex_data);

    for (size_t i = 0; i < hex_length / 2; i++) {
        unsigned char high_nibble = hex_char_to_value(hex_data[2 * i]);
        unsigned char low_nibble = hex_char_to_value(hex_data[2 * i + 1]);
        binary_data[i] = (high_nibble << 4) | low_nibble;
    }
}

int main() {

	 char *hex_data = "FFD8FFE000104A464946000101000001";
	//hex_data[read_size] = '\0';

    size_t hex_length = strlen(hex_data);
    if (hex_length % 2 != 0) {
        fprintf(stderr, "Hex data length is not even.\n");
        //free(hex_data);
        //free(clean_hex_data);
        exit(EXIT_FAILURE);
    }
    size_t binary_size = hex_length / 2;

    // Allocate buffer for binary data
    unsigned char *binary_data = (unsigned char *)malloc(binary_size);
    if (binary_data == NULL) {
        perror("Memory allocation failed");
        //free(hex_data);
        //free(clean_hex_data);
        exit(EXIT_FAILURE);
    }

    // Convert hex to binary using the function
    int retVal = hex_to_binary(hex_data, hex_length, binary_data);

    // Write binary data to output file
    FILE *out_fp = fopen(OUTPUT_FILE, "wb");
    if (out_fp == NULL) {
        perror("Cannot open output file");
        //free(hex_data);
        //free(clean_hex_data);
        free(binary_data);
        exit(EXIT_FAILURE);
    }

    fwrite(binary_data, 1, binary_size, out_fp);
    fclose(out_fp);

    // Clean up
    //free(hex_data);
    //free(clean_hex_data);
    free(binary_data);

    printf("Conversion completed successfully.\n");

    return 0;
}

