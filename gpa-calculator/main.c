#include <stdio.h>
#include <student.h>
#include <calculation.h>
#include <db.h>
#include <stdlib.h>

#define len(x) sizeof(x)/sizeof(*x)


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
		"B+"
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
		len(courses)
	};

	// creating db in memory
	sqlite3* db;
	sqlite3_open(":memory:", &db);
	init_db(db);
	
	// storing student
	if (store_student(db, stud1) == 0)
		printf("stored Student\n");
	else
		printf("error encountered storing student\n");
	if (store_student_courses(db, stud1) == 0)
		printf("stored student's Courses\n");
	else
		printf("error encountered storing courses\n");
	
	// getting student
	Student student = get_student(db, "WB01234");

	printf("\n");
	// printing values
	// courses enrolled
	for (int i = 0; i < student.number_of_courses; i++) {
		printf("%s credit_hours=%d grade=%s\n", student.courses[i].course_code, student.courses[i].credit_hours, student.courses[i].grade);
	};
	// names
	printf("\nStudent id=%s name=%s\n", student.id, student.name);
	// gpa and cgpa
	printf("\nGPA (sem 1)=%f CGPA=%f", get_student_gpa(student, 1), get_student_cgpa(student));

	// cleanup
	free_student(student);
	sqlite3_close(db);
}
