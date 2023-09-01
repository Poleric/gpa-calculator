#pragma once
#include <student.h>
#include <stdlib.h>

#ifndef UTILS_H
#define UTILS_H

void clear_screen();
int is_char_in(char character, char* character_arr, size_t arr_len);

int filter_sem_courses(int sem, Course** pCourses, size_t course_len, Course** buff);
Course* get_course_with_id(char* course_code, Course** pCourse, size_t course_len);

#endif
