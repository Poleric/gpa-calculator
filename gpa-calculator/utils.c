#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define CLEAR "cls"
#else
#define CLEAR "clear"  // assume POSIX
#endif

void clear_screen() {
	system(CLEAR);
}

int is_char_in(char character, char* character_arr, size_t arr_len) {
	/* Returns 0 if character is in the array, returns -1 if otherwise.
	Based on python `in` keyword
	*/

	for (int i = 0; i < arr_len; i++) {
		if (character == character_arr[i])
			return 0;
	}
	return -1;
}