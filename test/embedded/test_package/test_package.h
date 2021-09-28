#include "package.h"
#include "exception.h"
#include <unity.h>


void test_function_package(void) {
    uint8_t size = 9;
    Package pack = Package(size);

    for (int i=0; i<size; i+=3) {
        pack.addData(i, i*10);
        TEST_ASSERT_TRUE(pack.hasValue(i));
        TEST_ASSERT_EQUAL(pack.getValue(i), i*10);
        TEST_ASSERT_EQUAL(pack.getIndex(), i+3);
    }

    try {
        pack.addData(0x0C, 0x0A0A);
    }
    catch (Exception& err) {
        TEST_ASSERT_EQUAL(err.getCode(), package_error::FULL);
    }

    try {
        pack.getValue(0xFF);
    }
    catch (Exception& err){
        TEST_ASSERT_EQUAL(err.getCode(), package_error::NOT_FOUND);
    }
    
    uint8_t payload[] = {0, 1, 2};

    Package test_pack = Package(size);
    test_pack.loads(payload, 3);

    uint8_t invalid_payload[] = {0, 1, 2, 3, 2};
    try {
        test_pack.loads(invalid_payload, 5);
    }
    catch (Exception& err) {
        TEST_ASSERT_EQUAL(err.getCode(), package_error::INVALID_PAYLOAD);
    }

    uint8_t* data = test_pack.dump();
    for(int i=0; i<0; i++){
        TEST_ASSERT_EQUAL(data[i], i);
    }

    Package second_package = Package(payload, 3);
    TEST_ASSERT_EQUAL(second_package.getSize(), 3);
    TEST_ASSERT_EQUAL(second_package.getValue(0), (1 << 8 | 2));
}
