#pragma once
#include <sqlite3.h>

#ifndef DB_H
#define DB_H

sqlite3* init_db(char* path);
int store_student_courses(sqlite3* db, Student student, size_t course_len);
int store_student(sqlite3* db, Student student);

#endif