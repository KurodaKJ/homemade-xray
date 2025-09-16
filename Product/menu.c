#include "menu.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

static int getInt();

static const char* MenuStrings[] = {
    "Add Patient",
    "Delete Patient",
    "Select Patient",
    "Select Examination Type", 
    "Quit"
};

MenuOptions getMenuChoice(void)
{ 
   return (MenuOptions) getInt();
}

void displayMenu()
{
	size_t NrMenuStrings = sizeof(MenuStrings) / sizeof(MenuStrings[0]);
	
	printf("\n\nMENU\n");
	for (int i = 0; i < NrMenuStrings; i++) printf("  [%d] %s\n", i, MenuStrings[i]);
	printf("choice: ");
}

#define MAX_STRLEN 80

static int getInt()
{
    char line[MAX_STRLEN];
    char* result = NULL;
    int value = -1;

    result = fgets(line, sizeof(line), stdin);
    if (result != NULL) {
        sscanf(result, "%d", &value);
    }
    return value;
}

