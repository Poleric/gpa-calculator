#include <stdlib.h>
#include <student.h>

// not really used tbh, might remove
int filter_sem_courses(int sem, Course** pCourses, size_t courses_len, Course** buff) {
	int j = 0;  // increments when adding element into dest
	for (int i = 0; i < courses_len; i++) {
		if (pCourses[i]->sem == sem) {
			buff[j] = pCourses[i];
			j++;
		}
	};
	return 0;
}
