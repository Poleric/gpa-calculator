#pragma once
#include <sqlite3.h>
#include <student.h>
#include <stdlib.h>

#ifndef DB_H
#define DB_H

typedef struct SQLCourse {
    char* course_code;
    int sem;
    int credit_hours;
    char* grade;
    int row_id;
} SQLCourse;

typedef struct SQLStudent {
    char* student_id;
    char* name;
    SQLCourse** pSQLCourses;
    int number_of_courses;
} SQLStudent;

int init_student_db(sqlite3* db);

int store_student(sqlite3* db, Student* pStudent);
int store_student_courses(sqlite3* db, Student* pStudent);

SQLStudent* get_student(sqlite3* db, char* stud_id);
int get_student_courses(sqlite3* db, char* stud_id, SQLCourse** buff, size_t buff_size);

int update_sql_student(sqlite3* db, SQLStudent* pSQLStudent);
int update_sql_student_courses(sqlite3* db, SQLStudent* pSQLStudent);

int delete_sql_student(sqlite3* db, SQLStudent* pSQLStudent);
int delete_sql_student_courses(sqlite3* db, SQLStudent* pSQLStudent);

// helper functions
int get_number_of_courses(sqlite3* db, char* stud_id);
int get_number_of_students(sqlite3* db);
int get_max_sem(sqlite3* db);
sqlite3_stmt* get_students_stmt(sqlite3* db, const char* extra_sql);
#define get_all_students_stmt(db) get_students_stmt(db, NULL)
sqlite3_stmt* get_courses_stmt(sqlite3* db, const char* extra_sql);
#define get_student_courses_stmt(db) get_courses_stmt(db, "WHERE student_id=?")
SQLStudent* build_sql_student_from_stmt(sqlite3_stmt* stmt);
SQLCourse* build_sql_course_from_stmt(sqlite3_stmt* stmt);
int free_student(SQLStudent* pSQLStudent);
int free_courses(SQLCourse** pSQLCourses, int number_of_courses);
int free_course(SQLCourse *pSQLCourse);

#endif