#include <stdio.h>
#include <unistd.h>

void test_print() {
    printf("original test_print\n");
}

int main() {
    while (1) {
        test_print();
        sleep(3);
    }
    return 0;
}

