#define TEST_FAILED(msg) { printf(csString("Test Failed: ")+msg+csString("\n")); fatal_exit(-1, 0); }
#define TEST_SUCCEED() { printf("Test Succeeded!\n"); }

#define TEST_SETUP() \
void cleanup() {} \
void debug_dump() {}
