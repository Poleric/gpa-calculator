#include <stdlib.h>
#include <student.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define CLEAR "cls"
#else
#define CLEAR "clear"  // assume POSIX
#endif


void clear_screen() {
	system(CLEAR);
}

int is_char_in(char character, const char* character_arr, size_t arr_len) {
	/* Returns 0 if character is in the array, returns -1 if otherwise.
	Based on python `in` keyword
	*/

	for (int i = 0; i < arr_len; i++) {
		if (character == character_arr[i])
			return 0;
	}
	return -1;
}

/* Students related utils */
int filter_sem_courses(int sem, Course** pCourses, size_t courses_len, Course** buff) {
    /* Filters courses from a given semester from an array of Courses. */

    int j = 0;  // increments when adding element into dest
    for (int i = 0; i < courses_len; i++) {
        if (pCourses[i]->sem == sem) {
            buff[j] = pCourses[i];
            j++;
        }
    }
    return 0;
}

Course* get_course_with_id(char* course_code, Course** pCourse, size_t course_len) {
    /* Get the Course pointer given a course_code from an array of Courses. Returns NULL if not found. */

    for (int i = 0; i < course_len; i++) {
        if (strcmp(pCourse[i]->course_code, course_code) == 0) {
            return pCourse[i];
        }
    }
    return NULL;
}


