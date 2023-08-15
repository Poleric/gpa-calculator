#pragma once
#include <sqlite3.h>
#include <student.h>

#ifndef DB_H
#define DB_H

typedef struct SQLCourse {
    char* course_code;
    unsigned int sem;
    unsigned int credit_hours;
    char* grade;
    unsigned int sql_id;
} SQLCourse;

typedef struct SQLStudent {
    char* id;
    char* name;
    SQLCourse** pSQLCourses;
    unsigned int number_of_courses;
} SQLStudent;

int init_db(sqlite3* db);
int store_student(sqlite3* db, Student* pStudent);
int store_student_courses(sqlite3* db, Student* pStudent);

int get_number_of_courses(sqlite3* db, char* stud_id);
int get_student_courses(sqlite3* db, char* stud_id, SQLCourse** buff);
SQLStudent * get_student(sqlite3* db, char* stud_id);

int update_sql_student(sqlite3* db, SQLStudent* pSQLStudent);
int update_sql_student_courses(sqlite3* db, SQLStudent* pSQLStudent);

int free_student(SQLStudent* pSQLStudent);

#endif