#include <utils.h>
#include <string.h>
#include <libintl.h>

#define _(String) gettext(String)

void clear_screen() {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
        system("cls");
    #else
        system("clear"); // assume POSIX
    #endif
}

void flush_stdin() {
    /* rewind(stdin) or fflush(stdin) not working on my machine */
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

void pause() {
    /* cross platform solution to system("pause") lmao. */

    putchar('\n');
    printf(_("Press ENTER to proceed"));
    putchar('\n');

    getchar();
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

int is_valid_grade(char* grade) {
    if (strcmp(grade, "A") == 0)
        return 1;
    else if (strcmp(grade, "A-") == 0)
        return 1;
    else if (strcmp(grade, "B+") == 0)
        return 1;
    else if (strcmp(grade, "B") == 0)
        return 1;
    else if (strcmp(grade, "B-") == 0)
        return 1;
    else if (strcmp(grade, "C+") == 0)
        return 1;
    else if (strcmp(grade, "C") == 0)
        return 1;
    else if (strcmp(grade, "F") == 0)
        return 1;
    else
        return 0;
}

char* get_day(int day) {
    switch (day) {
        case 0:
            return _("MONDAY");
        case 1:
            return _("TUESDAY");
        case 2:
            return _("WEDNESDAY");
        case 3:
            return _("THURSDAY");
        case 4:
            return _("FRIDAY");
        case 5:
            return _("SATURDAY");
        case 6:
            return _("SUNDAY");
        default:
            return _("INVALID DAY");
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


