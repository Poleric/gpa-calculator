#pragma once
#include <student.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef UTILS_H
#define UTILS_H

void clear_screen();
void flush_stdin();
void pause();
int is_char_in(char character, const char* string, size_t arr_len);
int is_valid_grade(char* grade);
char* get_day(int day);

int filter_sem_courses(int sem, Course** pCourses, size_t course_len, Course** buff);
Course* get_course_with_id(char* course_code, Course** pCourse, size_t course_len);

#define cannot_alloc(ptr) (ptr == NULL)
#define log_alloc_error(module_name, variable_name) fprintf(stderr, "%s: Allocation error. Out of memory when allocating %s", module_name, variable_name)
#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

#endif
