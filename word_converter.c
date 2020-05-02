#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hashCode(const char *str) {
	int hash = 0;

	for (int i = 0; i < strlen(str); i++) {
		hash = 31 * hash + str[i];
	}

	return hash;
}


int main(void) {
	FILE *fp_read;
	FILE *fp_write;

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp_read = fopen("./dictionary/words.txt", "r");
	fp_write = fopen("./dictionary/converted.txt", "w");

	if (fp_read == NULL) return 1;
	if (fp_write == NULL) return 1;

	while((read = getline(&line, &len, fp_read)) != -1) {
		fprintf(fp_write, "%d\n", hashCode(strtok(line, "\n")));
	}

	fclose(fp_read);
	fclose(fp_write);
	
	if (line) free(line);

	return 0;
}
