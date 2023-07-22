#pragma once

#ifndef STUDENT_H
#define STUDENT_H

typedef struct Course {
	char* course_code;
	unsigned int sem;
	unsigned int credit_hours;
	char* grade;
} Course;

typedef struct Student {
	char* id;
	char* name;
	struct Course* courses;
	int number_of_courses;  // number of courses over all semesters (to keep track of array size lmao)
} Student;

int filter_sem_courses(int sem, struct Course*, size_t course_len, struct Course* buff);

#endif
