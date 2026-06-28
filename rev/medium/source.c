#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	char buff[24];
	char char_dec[4];
	char result[9];
	int index = 1;

	printf("Please enter key: ");
	scanf("%23s", buff);
	result[0] = 'd';
	if (buff[0] != '0' || buff[1] != '0') {
		printf("Nope.\n");
		exit(1);
	}
	for (int i = 2; i < 24; i += 3) {
		for (int j = 0; j < 3; j++)
			char_dec[j] = buff[i + j];
		char_dec[3] = '\0';
		int tmp = atoi(char_dec);
		result[index] = (char)tmp;
		index++;
	}
	if (strcmp(result, "delabere") != 0) {
		printf("Nope.\n");
		return 1;
	}
	printf("Good job.\n");
	return 0;
}
