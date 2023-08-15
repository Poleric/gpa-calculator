#pragma once

#ifndef STUDENT_H
#define STUDENT_H

typedef struct Course {
	char* course_code;
	unsigned int sem;
	unsigned int credit_hours;
	char* grade;

    unsigned int sql_id;
} Course;

typedef struct Student {
	char* id;
	char* name;
	struct Course** pCourses;
	unsigned int number_of_courses;  // number of courses over all semesters (to keep track of array size lmao)
} Student;

#endif
