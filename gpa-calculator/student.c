#include <stdlib.h>
#include <student.h>

int filter_sem_courses(int sem, Course* courses, size_t courses_len, Course* buff) {
	int j = 0;  // increments when adding element into dest
	for (int i = 0; i < courses_len; i++) {
		if (courses[i].sem == sem) {
			buff[j] = courses[i];
			j++;
		}
	};
	return 0;
}
