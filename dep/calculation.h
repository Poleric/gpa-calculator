#pragma once
#include <student.h>

#ifndef CALCULATION_H
#define CALCULATION_H

float get_grade_point(char* grade);
float get_gpa_from_courses(Course** pCourses, int number_of_courses, int sem);
float get_cgpa_from_courses(Course** pCourses, int number_of_courses);
#define get_student_gpa(pStudent, sem) get_gpa_from_courses(pStudent->pCourses, pStudent->number_of_courses, sem)
#define get_student_cgpa(pStudent) get_cgpa_from_courses(pStudent->pCourses, pStudent->number_of_courses)

#endif