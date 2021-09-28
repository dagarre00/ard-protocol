#include <unity.h>
#include "test_package/test_package.h"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_function_package);
    UNITY_END();
    return 0;
}