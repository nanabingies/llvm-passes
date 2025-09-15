#include <stdio.h>

int main() {
    int a = 15;
    double b;

    if (a > 20) {
        b = 10.0f;
    } else {
        printf("Bye!!!\n");
    }

    double c = b * 2 + a;
    printf("val: %f\n", c);

    return 0;
}