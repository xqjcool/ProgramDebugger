#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void test_print(long rnd) {
    printf("original test_print rnd:%ld\n", rnd);
}

int main() {
    srandom(time(NULL));
    while (1) {
        long rnd = random();
        test_print(rnd);
        sleep(3);
    }
    return 0;
}

