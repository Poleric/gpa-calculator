#include <utils.h>
#include <string.h>

 void clear_screen() {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
        system("cls");
    #else
        system("clear"); // assume POSIX
    #endif
}

int is_char_in(char character, const char* string, size_t arr_len) {
	/* Returns 0 if character is in the array, returns -1 if otherwise.
	Based on python `in` keyword
	*/

	for (int i = 0; i < arr_len; i++) {
		if (character == string[i])
			return 0;
	}
	return -1;
}

char* get_day(int day) {
    switch (day) {
        case 0:
            return "MONDAY";
        case 1:
            return "TUESDAY";
        case 2:
            return "WEDNESDAY";
        case 3:
            return "THURSDAY";
        case 4:
            return "FRIDAY";
        case 5:
            return "SATURDAY";
        case 6:
            return "SUNDAY";
        default:
            return "INVALID DAY";
    }
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


