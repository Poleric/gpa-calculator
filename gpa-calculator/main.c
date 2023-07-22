#include <stdio.h>
#include <student.h>
#include <calculation.h>
#include <db.h>
#include <stdlib.h>


int main() {
	// Constructing Student
	Course test1 = {
		"AAA1003",
		1,
		3,
		"B+"
	};
	Course test2 = {
		"AAA1014",
		1,
		4,
		"A-"
	};
	Course pc = {
		"PC101",
		2,
		3,
		"B"
	};
	Course awawa = {
		"AWAWA",
		2,
		2,
		"A"
	};
	Course aba = {
		"ABA101",
		3,
		4,
		"B+"
	};
	Course courses[5] = { test1, test2, pc, awawa, aba };
	Student stud1 = {
		"WB01234",
		"STUDENT",
		courses,
		sizeof(courses) / sizeof(*courses)
	};

	// getting courses of a semester
	Course* buff = (Course*)calloc(stud1.number_of_courses, sizeof(Course));  // initialize null array to store result
	filter_sem_courses(1, courses, sizeof(courses) / sizeof(*courses), buff);
	// printing results
	for (int i = 0; i < stud1.number_of_courses; i++) {
		if (buff[i].course_code) {
			// if element exists
			printf("%s grade=%s credit_hours=%d\n", buff[i].course_code, buff[i].grade, buff[i].credit_hours);
		}
	}
	free(buff);

	printf("\n");

	// count gpa
	printf("GPA = %f", get_student_gpa(stud1, 1));
	return 0;
}
