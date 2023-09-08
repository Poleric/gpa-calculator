#include <stdio.h>  // fprintf, stderr
#include <string.h>  // strdup
#include <sqlite3.h>
#include <student.h>  // Student, Course
#include <db.h>
#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define strdup _strdup  //  warning C4996: 'strdup': The POSIX name for this item is deprecated.
#define _CRT_SECURE_NO_WARNINGS 1
#endif

int init_student_db(sqlite3* db) {
	/* Create the tables needed */

	char* exc;
	int ret;
	ret = sqlite3_exec(
		db,
		"CREATE TABLE IF NOT EXISTS students ("
			"student_id VARCHAR(9) NOT NULL UNIQUE,"
			"student_name TEXT NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS registered_courses ("
			"rowid INTEGER PRIMARY KEY,"
			"course_code VARCHAR(7) NOT NULL,"
			"semester INT NOT NULL,"
			"credit_hours INT NOT NULL,"
			"grade VARCHAR(2) NOT NULL,"
			"student_id VARCHAR(9) NOT NULL,"
			"FOREIGN KEY(student_id) REFERENCES students(student_id)"
		");"
		,
		NULL,
		NULL,
		&exc
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "init_student_db: SQL error: %s\n", exc);
		sqlite3_free(exc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int store_student(sqlite3* db, Student* pStudent) {
	/* Store Student struct into `students` table */

	sqlite3_stmt* stmt;

	int ret = sqlite3_prepare(
		db,
		"INSERT OR REPLACE INTO students (student_id, student_name) VALUES (?, ?);",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "store_student: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	// bind is one-indexed, retrieving data (ie, sqlite_column) is zero-indexed
	sqlite3_bind_text(stmt, 1, pStudent->student_id, -1, NULL);  // (stmt, index, value, bytesize, flag)
	sqlite3_bind_text(stmt, 2, pStudent->name, -1, NULL);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return EXIT_SUCCESS;
}

int store_student_courses(sqlite3* db, Student* pStudent) {
	/* Store student's Courses into `registered_courses` table */
	
	sqlite3_stmt* stmt;

	int ret = sqlite3_prepare(
		db,
		"INSERT OR REPLACE INTO registered_courses (course_code, semester, credit_hours, grade, student_id) VALUES (?, ?, ?, ?, ?);",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "store_student_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	for (int i = 0; i < pStudent->number_of_courses; i++) {
		sqlite3_bind_text(stmt, 1, pStudent->pCourses[i]->course_code, -1, NULL);
		sqlite3_bind_int(stmt, 2, pStudent->pCourses[i]->sem);
		sqlite3_bind_int(stmt, 3, pStudent->pCourses[i]->credit_hours);
		sqlite3_bind_text(stmt, 4, pStudent->pCourses[i]->grade, -1, NULL);
		sqlite3_bind_text(stmt, 5, pStudent->student_id, -1, NULL);

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

SQLStudent* get_student(sqlite3* db, char* stud_id) {
    /* Get Student with their student_id from `students` table.
    Returns NULL if error.

    Remember to free Student when getting it using this function.
    */

    SQLStudent* pSQLStudent;
    sqlite3_stmt* stmt = get_students_stmt(db, "WHERE student_id=?");
    if (stmt == NULL) {
        fprintf(stderr, "get_student: Failed to allocate memory for stmt\n");
        return NULL;
    }
    sqlite3_bind_text(stmt, 1, stud_id, -1, NULL);

    // run the sql
    int ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW) {
        pSQLStudent = build_sql_student_from_stmt(stmt);
        if (pSQLStudent == NULL) {
            fprintf(stderr, "get_student: Allocation error when allocating for student pointer. Possible Out of Memory.");
            return NULL;
        }

        // getting courses
        int number_of_courses = get_number_of_courses(db, stud_id);

        // courses get put into the courses buffer
        SQLCourse** pSQLCourses = calloc(number_of_courses, sizeof(Course *));
        // checking allocation
        if (pSQLCourses == NULL) {
            fprintf(stderr, "get_student: Allocation error when allocating courses buffer. Possible Out of Memory.");
            return NULL;
        }

        if (get_student_courses(db, stud_id, pSQLCourses, number_of_courses) == EXIT_FAILURE) {  // if error
            fprintf(stderr, "get_student: Courses cannot be loaded. Exiting the function");
            return NULL;
        }
        pSQLStudent->pSQLCourses = pSQLCourses;
        pSQLStudent->number_of_courses = number_of_courses;
        sqlite3_finalize(stmt);
        return pSQLStudent;
    }
    sqlite3_finalize(stmt);
    return NULL;
}

int get_student_courses(sqlite3* db, char* stud_id, SQLCourse** buff, size_t buff_len) {
	/* Get student's Courses with student_id from `registered_courses` table */
	sqlite3_stmt* stmt = get_student_courses_stmt(db);
    sqlite3_bind_text(stmt, 1, stud_id, -1, NULL);
    SQLCourse* pSQLCourse;

	for (
		int i = 0, ret = sqlite3_step(stmt);  // 1. run the sql, and assign the return value
		i < buff_len || ret == SQLITE_ROW;  // 2. break if buffer no space or no more sqlite results
		i++, ret = sqlite3_step(stmt)  // 3. go next row and continue at 2.
    )
	{
        pSQLCourse = build_sql_course_from_stmt(stmt);
        if (pSQLCourse == NULL) {
            return EXIT_FAILURE;
        }
		buff[i] = pSQLCourse;
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

// alias
int update_sql_student(sqlite3* db, SQLStudent* pSQLStudent) {
	return store_student(db, (Student*)pSQLStudent);
}

// carbon copy of store_student_courses, except for sql statement and an extra value bind
int update_sql_student_courses(sqlite3* db, SQLStudent* pSQLStudent) {
	/* Store student's Courses into `registered_courses` table */

	sqlite3_stmt* stmt;

	int ret = sqlite3_prepare(
		db,
		"INSERT OR REPLACE INTO registered_courses (rowid, course_code, semester, credit_hours, grade, student_id) VALUES (?, ?, ?, ?, ?, ?);",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "update_sql_student_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	for (int i = 0; i < pSQLStudent->number_of_courses; i++) {
		if (pSQLStudent->pSQLCourses[i] != NULL) {
			sqlite3_bind_int(stmt, 1, pSQLStudent->pSQLCourses[i]->row_id);
			sqlite3_bind_text(stmt, 2, pSQLStudent->pSQLCourses[i]->course_code, strlen(pSQLStudent->pSQLCourses[i]->course_code), NULL);
			sqlite3_bind_int(stmt, 3, pSQLStudent->pSQLCourses[i]->sem);
			sqlite3_bind_int(stmt, 4, pSQLStudent->pSQLCourses[i]->credit_hours);
			sqlite3_bind_text(stmt, 5, pSQLStudent->pSQLCourses[i]->grade, strlen(pSQLStudent->pSQLCourses[i]->grade), NULL);
			sqlite3_bind_text(stmt, 6, pSQLStudent->student_id, strlen(pSQLStudent->student_id), NULL);

			sqlite3_step(stmt);
			sqlite3_reset(stmt);
		}
		else {
			fprintf(stderr, "update_student_course: Course %s cannot be updated because rowid is NULL.", pSQLStudent->pSQLCourses[i]->course_code);
		}
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

/*
    Helper functions
*/
//sqlite3_stmt select_stmt(sqlite3* db, char* table)

int get_number_of_courses(sqlite3* db, char* stud_id) {
    /* get number of course for allocating memory. -1 means error*/

    sqlite3_stmt* stmt;
    int number_of_courses = -1;

    int ret = sqlite3_prepare_v2(
            db,
            "SELECT COUNT(*) FROM registered_courses WHERE student_id=?",
            -1,
            &stmt,
            NULL
    );

    if (ret != SQLITE_OK) {
        fprintf(stderr, "get_number_of_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, stud_id, -1, NULL);

    // run the sql
    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        number_of_courses = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return number_of_courses;
}

int get_number_of_students(sqlite3* db) {
    /* get number of students in the database. -1 means error*/

    sqlite3_stmt* stmt;
    int number_of_students = -1;

    int ret = sqlite3_prepare_v2(
            db,
            "SELECT COUNT(*) FROM students",
            -1,
            &stmt,
            NULL
    );

    if (ret != SQLITE_OK) {
        fprintf(stderr, "get_number_of_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    // run the sql
    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        number_of_students = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return number_of_students;
}

int get_max_sem(sqlite3* db) {
    /* get the highest semester in the list */

    sqlite3_stmt* stmt;
    int max_sem = -1;

    int ret = sqlite3_prepare_v2(
            db,
            "SELECT MAX(semester) FROM registered_courses",
            -1,
            &stmt,
            NULL
    );

    if (ret != SQLITE_OK) {
        fprintf(stderr, "get_max_sem: Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        max_sem = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return max_sem;
}

sqlite3_stmt* get_students_stmt(sqlite3* db, const char* extra_sql) {
    /* Low level helper function to get the sql statement to get data from the table students.
     * Manually bind & step. Free when done.
     *
     * Returns NULL when error.
     */

    sqlite3_stmt* stmt;
    char* base_cmd = "SELECT * FROM students";
    char* sql_cmd = malloc(strlen(base_cmd) + (extra_sql != NULL ? strlen(extra_sql) + 2 : 1) + 1);

    // build sql_cmd
    strcpy(sql_cmd, base_cmd);
    if (extra_sql != NULL) {
        strcat(sql_cmd, " ");
        strcat(sql_cmd, extra_sql);
    }

    int ret = sqlite3_prepare_v2(
            db,
            sql_cmd,
            -1,
            &stmt,
            NULL
    );
    free(sql_cmd);

    if (ret != SQLITE_OK) {
        fprintf(stderr, "get_students_stmt: Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    return stmt;
}

sqlite3_stmt* get_courses_stmt(sqlite3* db, const char* extra_sql) {
    sqlite3_stmt* stmt;
    char* base_cmd = "SELECT * FROM registered_courses";
    char* sql_cmd = malloc(strlen(base_cmd) + (extra_sql != NULL ? strlen(extra_sql) + 2 : 1) + 1);

    // build sql_cmd
    strcpy(sql_cmd, base_cmd);
    if (extra_sql != NULL) {
        strcat(sql_cmd, " ");
        strcat(sql_cmd, extra_sql);
    }

    int ret = sqlite3_prepare_v2(
            db,
            sql_cmd,
            -1,
            &stmt,
            NULL
    );
    free(sql_cmd);

    if (ret != SQLITE_OK) {
        fprintf(stderr, "get_courses_stmt: Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    return stmt;
}

SQLStudent* build_sql_student_from_stmt(sqlite3_stmt* stmt) {
    /* Note: The returned SQLStudent only have fields that are retrievable from the get_student stmt, ie only student_id & name. < SQLCourses need to be manually added afterwards > */

    SQLStudent* pSQLStudent = malloc(sizeof(SQLStudent));

    // duplicating string because pointer is destroyed after sql_finalize

    // assigning variable to check memory allocation
    char* student_id = strdup(sqlite3_column_text(stmt, 0));
    char* student_name = strdup(sqlite3_column_text(stmt, 1));

    // handling allocation failure
    if ((student_id == NULL) || (student_name == NULL)) {  // allocation failure, will NOT return Student
        fprintf(stderr, "get_student: Allocation error when duplicating student_id and name. Possible Out of Memory.");
        free(student_id);
        free(student_name);
        return NULL;
    }

    // constructing Student
    pSQLStudent->student_id = student_id;
    pSQLStudent->name = student_name;
    return pSQLStudent;
}

SQLCourse* build_sql_course_from_stmt(sqlite3_stmt* stmt) {
    SQLCourse* pSQLCourse = malloc(sizeof(SQLCourse));
    if (pSQLCourse == NULL) {
        fprintf(stderr, "build_sql_course_from_stmt: Allocation error when allocating SQLCourse pointer. Possible Out of Memory.");
        return NULL;
    }

    // duplicating string because pointer is destroyed after sql_finalize

    // assigning variable to check memory allocation
    char* course_code = strdup(sqlite3_column_text(stmt, 1));
    char* grade = strdup(sqlite3_column_text(stmt, 4));

    // handling allocation failure
    if ((course_code == NULL) || (grade == NULL)) {  // allocation failure, will NOT return Student
        fprintf(stderr, "get_student_courses: Allocation error when duplicating course_code and grade. Possible Out of Memory.");
        free(course_code);
        free(grade);
        return NULL;
    }

    // initialize Course
    pSQLCourse->row_id = sqlite3_column_int(stmt, 0);  // sql student_id
    pSQLCourse->course_code = course_code;
    pSQLCourse->sem = sqlite3_column_int(stmt, 2);  // sem
    pSQLCourse->credit_hours = sqlite3_column_int(stmt, 3);  // credit_hours
    pSQLCourse->grade = grade;
    return pSQLCourse;
}


// utils function for freeing memory allocated after finished using it.
int free_student(SQLStudent *pSQLStudent) {
    /* frees SQLStudent and its SQLCourses */

	free(pSQLStudent->student_id);
	free(pSQLStudent->name);
	free_courses(pSQLStudent->pSQLCourses, (int)pSQLStudent->number_of_courses);
	free(pSQLStudent);

	return EXIT_SUCCESS;
}

int free_courses(SQLCourse** pSQLCourses, int number_of_courses) {
    for (int i = 0; i < number_of_courses; i++) {
        free_course(pSQLCourses[i]);
    }
    free(pSQLCourses);
    return EXIT_SUCCESS;
}

int free_course(SQLCourse *pSQLCourse) {
    free(pSQLCourse->course_code);
    free(pSQLCourse->grade);
    free(pSQLCourse);
    return EXIT_SUCCESS;
}
