#pragma once

#ifndef STUDENT_H
#define STUDENT_H

typedef struct Course {
	char* course_code;
	int sem;
    int credit_hours;
	char* grade;
} Course;

typedef struct Student {
	char* student_id;
	char* name;
	struct Course** pCourses;
    int number_of_courses;  // number of courses over all semesters (to keep track of array size lmao)
} Student;

#endif
