#pragma once
#include <sqlite3.h>
#include <student.h>

#ifndef DB_H
#define DB_H

int init_db(sqlite3* db);
int store_student_courses(sqlite3* db, Student student);
int store_student(sqlite3* db, Student student);

int get_number_of_courses(sqlite3* db, char* stud_id);
int get_student_courses(sqlite3* db, char* stud_id, Course* buff);
Student get_student(sqlite3* db, char* stud_id);

int free_student(Student student);

#endif