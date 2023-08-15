#include <stdio.h>
#include <string.h>  // strdup
#include <sqlite3.h>
#include <student.h>
#include <db.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define strdup _strdup  //  warning C4996: 'strdup': The POSIX name for this item is deprecated.
#endif

int init_db(sqlite3* db) {
	/* Create the tables needed */

	char* exc;
	int ret;
	ret = sqlite3_exec(
		db,
		"CREATE TABLE IF NOT EXISTS students ("
			"student_id VARCHAR(9) PRIMARY KEY,"
			"student_name TEXT NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS registered_courses ("
			"course_id INTEGER PRIMARY KEY,"
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
		fprintf(stderr, "init_db: SQL error: %s\n", exc);
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
	sqlite3_bind_text(stmt, 1, pStudent->id, strlen(pStudent->id), NULL);  // (stmt, index, value, bytesize, flag) 
	sqlite3_bind_text(stmt, 2, pStudent->name, strlen(pStudent->name), NULL);

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
		sqlite3_bind_text(stmt, 1, pStudent->pCourses[i]->course_code, strlen(pStudent->pCourses[i]->course_code), NULL);
		sqlite3_bind_int(stmt, 2, pStudent->pCourses[i]->sem);
		sqlite3_bind_int(stmt, 3, pStudent->pCourses[i]->credit_hours);
		sqlite3_bind_text(stmt, 4, pStudent->pCourses[i]->grade, strlen(pStudent->pCourses[i]->grade), NULL);
		sqlite3_bind_text(stmt, 5, pStudent->id, strlen(pStudent->id), NULL);

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

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
	
	sqlite3_bind_text(stmt, 1, stud_id, strlen(stud_id), NULL);

	// run the sql
	ret = sqlite3_step(stmt);

	if (ret == SQLITE_ROW) {
		number_of_courses = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return number_of_courses;
}

int get_student_courses(sqlite3* db, char* stud_id, Course** buff) {
	/* Get student's Courses with student_id from `registered_courses` table */
	sqlite3_stmt* stmt;
	Course* pCourse;
	
	int ret = sqlite3_prepare_v2(
		db,
		"SELECT * FROM registered_courses WHERE student_id=?",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "get_student_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	sqlite3_bind_text(stmt, 1, stud_id, strlen(stud_id), NULL);

	char* course_code;
	char* grade;
	for (
		int i = 0, ret = sqlite3_step(stmt);  // 1. run the sql
		ret == SQLITE_ROW;  // 2. check have result
		i++, ret = sqlite3_step(stmt)  // 3. go next row and continue at 2.
		) 
	{	
		pCourse = malloc(sizeof(Course));
		if (pCourse == NULL) {
			fprintf(stderr, "get_student_courses: Allocation error when allocating Course pointer. Possible Out of Memory.");
			return EXIT_FAILURE;
		}

		// duplicating string because pointer is destroyed after sql_finalize
		// that also means it need to be freed when not used
		course_code = strdup(sqlite3_column_text(stmt, 1));
		grade = strdup(sqlite3_column_text(stmt, 4));

		// handling allocation failure
		if ((course_code == NULL) || (grade == NULL)) {  // allocation failure, will NOT return Student
			fprintf(stderr, "get_student_courses: Allocation error when duplicating course_code and grade. Possible Out of Memory.");
			return EXIT_FAILURE;
		}

		// initialize Course
		pCourse->sql_id = sqlite3_column_int(stmt, 0);  // sql id
		pCourse->course_code = course_code;
		pCourse->sem = sqlite3_column_int(stmt, 2);  // sem
		pCourse->credit_hours = sqlite3_column_int(stmt, 3);  // credit_hours
		pCourse->grade = grade;
		buff[i] = pCourse;
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

Student* get_student(sqlite3* db, char* stud_id) {
	/* Get Student with their student_id from `students` table.
	Returns NULL if error.
	
	Remember to free Student when getting it using this function.
	*/
	sqlite3_stmt* stmt;
	Student* pStudent = malloc(sizeof(Student));

	if (pStudent == NULL) {
		fprintf(stderr, "get_student: Allocation error when allocating for student pointer. Possible Out of Memory.");
		return NULL;
	}

	int ret = sqlite3_prepare_v2(
		db,
		"SELECT * FROM students WHERE student_id=?",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "get_student: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return NULL;
	}
	
	sqlite3_bind_text(stmt, 1, stud_id, strlen(stud_id), NULL);

	// run the sql
	ret = sqlite3_step(stmt);

	char* student_id;
	char* student_name;
	if (ret == SQLITE_ROW) {
		int number_of_courses = get_number_of_courses(db, stud_id);

		// courses get put into the courses buffer
		Course** pCourses = calloc(number_of_courses, sizeof(Course *));
		// checking allocation
		if (pCourses == NULL) {
			fprintf(stderr, "get_student: Allocation error when allocating courses buffer. Possible Out of Memory.");
			return NULL;
		}

		if (get_student_courses(db, stud_id, pCourses) == EXIT_FAILURE) {  // if error
			fprintf(stderr, "get_student: Courses cannot be loaded. Exiting the function");
			return NULL;
		}

		// duplicating string because pointer is destroyed after sql_finalize
		// that also means it need to be freed when not used
		student_id = strdup(sqlite3_column_text(stmt, 0));
		student_name = strdup(sqlite3_column_text(stmt, 1));

		// handling allocation failure
		if ((student_id == NULL) || (student_name == NULL)) {  // allocation failure, will NOT return Student
			fprintf(stderr, "get_student: Allocation error when duplicating student_id and name. Possible Out of Memory.");
			return NULL;
		}

		// constructing Student
		pStudent->id = student_id;
		pStudent->name = student_name;
		pStudent->pCourses = pCourses;
		pStudent->number_of_courses = number_of_courses;

		sqlite3_finalize(stmt);
		return pStudent;
	}

	sqlite3_finalize(stmt);
	return NULL;
}

// alias
int update_student(sqlite3* db, Student* pStudent) {
	return store_student(db, pStudent);
}

// carbon copy of store_student_courses, except for sql statement and an extra value bind
int update_student_courses(sqlite3* db, Student* pStudent) {
	/* Store student's Courses into `registered_courses` table */

	sqlite3_stmt* stmt;

	int ret = sqlite3_prepare(
		db,
		"INSERT OR REPLACE INTO registered_courses (course_id, course_code, semester, credit_hours, grade, student_id) VALUES (?, ?, ?, ?, ?, ?);",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "update_student_courses: Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return EXIT_FAILURE;
	}

	for (int i = 0; i < pStudent->number_of_courses; i++) {
		if (pStudent->pCourses[i] != NULL) {
			sqlite3_bind_int(stmt, 1, pStudent->pCourses[i]->sql_id);
			sqlite3_bind_text(stmt, 2, pStudent->pCourses[i]->course_code, strlen(pStudent->pCourses[i]->course_code), NULL);
			sqlite3_bind_int(stmt, 3, pStudent->pCourses[i]->sem);
			sqlite3_bind_int(stmt, 4, pStudent->pCourses[i]->credit_hours);
			sqlite3_bind_text(stmt, 5, pStudent->pCourses[i]->grade, strlen(pStudent->pCourses[i]->grade), NULL);
			sqlite3_bind_text(stmt, 6, pStudent->id, strlen(pStudent->id), NULL);

			sqlite3_step(stmt);
			sqlite3_reset(stmt);
		}
		else {
			fprintf(stderr, "update_student_course: Course %s cannot be updated because sql_id is NULL.", pStudent->pCourses[i]->course_code);
		}
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

// utils function for freeing memory allocated after finishing
int free_student(Student *student) {
	free(student->id);
	free(student->name);

	for (int i = 0; i < student->number_of_courses; i++) {
		free(student->pCourses[i]->course_code);
		free(student->pCourses[i]->grade);
	}
	free(student->pCourses);
	free(student);

	return EXIT_SUCCESS;
}
