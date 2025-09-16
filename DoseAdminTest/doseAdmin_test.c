#include <string.h>
#include "doseAdmin.h"
#include "unity.h"
#include <stdlib.h>

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
    // This is run before EACH test
}

void tearDown(void)
{
    // This is run after EACH test
}

void test_FailTest(void)
{
    TEST_ASSERT_EQUAL(1, 0); 
}

void test_LeakTest(void)
{
	int *a = (int*) malloc(sizeof(int));
    TEST_ASSERT_EQUAL(1, 1); 
	*a = 666;
}

void test_OutOfRangeTest(void)
{
	int array[10];
	
	array[10] = 666;
	(void)array;
}

// add here all your dose admin testcases, and call them in main!! Remove the given testcases

int main()
{
    UnityBegin();

    MY_RUN_TEST(test_FailTest);
    MY_RUN_TEST(test_LeakTest);
    MY_RUN_TEST(test_OutOfRangeTest);

    UnityEnd();
}
