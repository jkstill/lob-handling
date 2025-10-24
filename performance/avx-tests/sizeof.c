#include <stdio.h>

int main() {
    int myInt;
    char myChar;
    double myDouble;
	 char *hx = "F2C78D5A3E91B4C0F8D4730AB9E6254D";

    printf("Size of int: %zu bytes\n", sizeof(myInt));
    printf("Size of char: %zu bytes\n", sizeof(myChar));
    printf("Size of double: %zu bytes\n", sizeof(myDouble));
    printf("Size of hx: %zu bytes\n", sizeof(hx));

    // You can also use sizeof with data types directly
    printf("Size of long long: %zu bytes\n", sizeof(long long));

    return 0;
}
