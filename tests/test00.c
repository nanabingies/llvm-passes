#include <stdio.h>

int test_a();
void test_b(int);

void test_c(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char* argv[]) {
    //printf("Hello World!\n");

    int a = 10, b = 20;

    test_a();
    test_b(10);

    test_c(&a, &b);
}

int test_a() {
    int x = 1, y = 2, c = 0;

    x = x * y + 8;
    y = y * 2 * 10;

    if (x > y)
        c = x + 1;
    else
        c = y + 1;

    return c;
}

void test_b(int x) {

}
