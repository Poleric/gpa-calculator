#pragma once
#include <student.h>

#ifndef CALCULATION_H
#define CALCULATION_H

float get_grade_point(char* grade);
float get_gpa_from_courses(Course** pCourses, int number_of_courses, int sem);
int get_gpas_from_courses(Course** pCourses, int number_of_courses, float* buff, int buff_len);
float get_cgpa_from_courses(Course** pCourses, int number_of_courses);
float get_student_gpa(Student* pStudent, int sem);
int get_student_gpas(Student* pStudent, float* buff, int buff_len);
float get_student_cgpa(Student* pStudent);
int get_total_credit_hours(Course** pCourses, int number_of_courses);

#endif