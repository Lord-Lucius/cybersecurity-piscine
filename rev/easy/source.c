#include <stdio.h>
#include <string.h>

int main(void) {
	char buff[64];
	printf("Please enter key:");
	scanf("%s", buff);
	if (strcmp(buff, "__stack_check") != 0) {
		printf("Nope.\n");
		return 1;
	}
	printf("Good job.\n");
	return 0;
}
