#include <string.h>
#include "doseAdmin.h"
#include "unity.h"
#include <stdlib.h>

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
    CreateHashTable();
}

void tearDown(void)
{
    RemoveAllDataFromHashTable();
}

void test_AddPatient(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    char longName[81];

    // Test adding a patient successfully
    TEST_ASSERT_EQUAL(0, AddPatient(johnDoe));

    // Test adding a patient with a name that is too long
    memset(longName, 'A', 80);
    longName[80] = '\0';
    TEST_ASSERT_EQUAL(-3, AddPatient(longName));

    // Test adding a duplicate patient
    TEST_ASSERT_EQUAL(-1, AddPatient(johnDoe));
}

void test_IsPatientPresent(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    char janeDoe[MAX_PATIENTNAME_SIZE] = "JaneDoe";

    AddPatient(johnDoe);

    // Test checking if a patient is present
    TEST_ASSERT_EQUAL(0, IsPatientPresent(johnDoe));

    // Test checking if a non-existent patient is present
    TEST_ASSERT_EQUAL(-1, IsPatientPresent(janeDoe));
}

void test_AddPatientDose(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    char janeDoe[MAX_PATIENTNAME_SIZE] = "JaneDoe";
    Date date = {15, 6, 2023};

    AddPatient(johnDoe);

    // Test adding a dose to an existing patient
    TEST_ASSERT_EQUAL(0, AddPatientDose(johnDoe, &date, 100));

    // Test adding a dose to a non-existent patient
    TEST_ASSERT_EQUAL(-1, AddPatientDose(janeDoe, &date, 100));
}

void test_PatientDoseInPeriod(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    Date date1 = {15, 6, 2023};
    Date date2 = {20, 6, 2023};
    Date startDate = {1, 6, 2023};
    Date endDate = {30, 6, 2023};
    uint32_t totalDose;

    AddPatient(johnDoe);
    AddPatientDose(johnDoe, &date1, 100);
    AddPatientDose(johnDoe, &date2, 50);

    // Test calculating the total dose for a patient within a specific date range
    TEST_ASSERT_EQUAL(0, PatientDoseInPeriod(johnDoe, &startDate, &endDate, &totalDose));
    TEST_ASSERT_EQUAL(150, totalDose);

    // Test calculating the total dose for a non-existent patient
    char janeDoe[MAX_PATIENTNAME_SIZE] = "JaneDoe";
    TEST_ASSERT_EQUAL(-1, PatientDoseInPeriod(janeDoe, &startDate, &endDate, &totalDose));
}

void test_RemovePatient(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    char janeDoe[MAX_PATIENTNAME_SIZE] = "JaneDoe";

    AddPatient(johnDoe);

    // Test removing an existing patient
    TEST_ASSERT_EQUAL(0, RemovePatient(johnDoe));
    TEST_ASSERT_EQUAL(-1, IsPatientPresent(johnDoe));

    // Test removing a non-existent patient
    TEST_ASSERT_EQUAL(-1, RemovePatient(janeDoe));
}

void test_GetNumberOfMeasurements(void)
{
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    char janeDoe[MAX_PATIENTNAME_SIZE] = "JaneDoe";
    Date date1 = {15, 6, 2023};
    Date date2 = {20, 6, 2023};
    size_t numberOfMeasurements;

    AddPatient(johnDoe);
    AddPatientDose(johnDoe, &date1, 100);
    AddPatientDose(johnDoe, &date2, 50);

    // Test getting the number of measurements for an existing patient
    TEST_ASSERT_EQUAL(0, GetNumberOfMeasurements(johnDoe, &numberOfMeasurements));
    TEST_ASSERT_EQUAL(2, numberOfMeasurements);

    // Test getting the number of measurements for a non-existent patient
    TEST_ASSERT_EQUAL(-1, GetNumberOfMeasurements(janeDoe, &numberOfMeasurements));
}

void test_GetHashPerformance(void)
{
    size_t total = 0;
    double avg = 0.0;
    double stdDev = 0.0;

    GetHashPerformance(&total, &avg, &stdDev);
    TEST_ASSERT_EQUAL_UINT(0, total);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, (float)avg);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, (float)stdDev);

    AddPatient("JohnDoe");
    GetHashPerformance(&total, &avg, &stdDev);

    TEST_ASSERT_EQUAL_UINT(1, total);

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.003906f, (float)avg);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.062f, (float)stdDev);

    AddPatient("JaneDoe");
    AddPatient("Alice");

    GetHashPerformance(&total, &avg, &stdDev);
    TEST_ASSERT_EQUAL_UINT(3, total);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, (float)(3.0/HASHTABLE_SIZE), (float)avg);
}

void test_WriteToFile_ReadFromFile(void)
{
    char testFilePath[MAX_FILEPATH_LEGTH] = "test_file.txt";
    char johnDoe[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    Date date1 = {15, 6, 2023};
    Date date2 = {20, 6, 2023};

    AddPatient(johnDoe);
    AddPatientDose(johnDoe, &date1, 100);
    AddPatientDose(johnDoe, &date2, 50);

    // Test writing patient data to a file
    TEST_ASSERT_EQUAL(0, WriteToFile(testFilePath));

    // Clear the hash table
    RemoveAllDataFromHashTable();

    // Test reading patient data from a file
    TEST_ASSERT_EQUAL(0, ReadFromFile(testFilePath));

    // Verify the data was read correctly
    uint32_t totalDose;
    Date startDate = {1, 6, 2023};
    Date endDate = {30, 6, 2023};
    TEST_ASSERT_EQUAL(0, PatientDoseInPeriod(johnDoe, &startDate, &endDate, &totalDose));
    TEST_ASSERT_EQUAL(150, totalDose);

    // Clean up the test file
    remove(testFilePath);
}

void test_Persistence_Failures(void)
{
    char validPath[MAX_FILEPATH_LEGTH] = "test_duplicate.txt";
    char ghostPath[MAX_FILEPATH_LEGTH] = "ghost_file.txt";
    char badPath[MAX_FILEPATH_LEGTH]   = "non_existent_folder/file.txt";

    TEST_ASSERT_EQUAL(-1, WriteToFile(badPath));

    TEST_ASSERT_EQUAL(-1, ReadFromFile(ghostPath));

    AddPatient("UniqueUser");
    WriteToFile(validPath);

    TEST_ASSERT_EQUAL(-1, ReadFromFile(validPath));

    // Cleanup
    remove(validPath);
}

int main()
{
    UnityBegin();

    MY_RUN_TEST(test_AddPatient);
    MY_RUN_TEST(test_IsPatientPresent);
    MY_RUN_TEST(test_AddPatientDose);
    MY_RUN_TEST(test_PatientDoseInPeriod);
    MY_RUN_TEST(test_RemovePatient);
    MY_RUN_TEST(test_GetHashPerformance);
    MY_RUN_TEST(test_GetNumberOfMeasurements);
    MY_RUN_TEST(test_WriteToFile_ReadFromFile);
    MY_RUN_TEST(test_Persistence_Failures);

    UnityEnd();
}
