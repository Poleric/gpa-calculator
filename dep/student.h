#pragma once

#ifndef STUDENT_H
#define STUDENT_H

typedef struct Course {
	int sql_id;
	char* course_code;
	unsigned int sem;
	unsigned int credit_hours;
	char* grade;
} Course;

typedef struct Student {
	char* id;
	char* name;
	struct Course** pCourses;
	int number_of_courses;  // number of courses over all semesters (to keep track of array size lmao)
} Student;

int filter_sem_courses(int sem, Course** pCourses, size_t course_len, Course** buff);

#endif
