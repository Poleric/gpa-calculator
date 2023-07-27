#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <student.h>
#include <db.h>
#include <stdlib.h>
#include <string.h>

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
			"course_id INT PRIMARY KEY,"
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
		fprintf(stderr, "SQL error: %s\n", exc);
		sqlite3_free(exc);
		return -1;
	}

	return 0;
}

int store_student(sqlite3* db, Student student) {
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
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	// bind is one-indexed, retrieving data (ie, sqlite_column) is zero-indexed
	sqlite3_bind_text(stmt, 1, student.id, strlen(student.id), NULL);  // (stmt, index, value, bytesize, flag) 
	sqlite3_bind_text(stmt, 2, student.name, strlen(student.name), NULL);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int store_student_courses(sqlite3* db, Student student) {
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
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	for (int i = 0; i < student.number_of_courses; i++) {
		sqlite3_bind_text(stmt, 1, student.courses[i].course_code, strlen(student.courses[i].course_code), NULL);
		sqlite3_bind_int(stmt, 2, student.courses[i].sem);
		sqlite3_bind_int(stmt, 3, student.courses[i].credit_hours);
		sqlite3_bind_text(stmt, 4, student.courses[i].grade, strlen(student.courses[i].grade), NULL);
		sqlite3_bind_text(stmt, 5, student.id, strlen(student.id), NULL);

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	return 0;
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
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
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

int get_student_courses(sqlite3* db, char* stud_id, Course* buff) {
	/* Get student's Courses with student_id from `registered_courses` table */
	sqlite3_stmt* stmt;
	
	int ret = sqlite3_prepare_v2(
		db,
		"SELECT * FROM registered_courses WHERE student_id=?",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return -1;
	};

	sqlite3_bind_text(stmt, 1, stud_id, strlen(stud_id), NULL);

	char* course_code;
	char* grade;
	for (
		int i = 0, ret = sqlite3_step(stmt);  // 1. run the sql
		ret == SQLITE_ROW;  // 2. check have result
		i++, ret = sqlite3_step(stmt)  // 3. go next row and continue at 2.
		) 
	{	
		// duplicating string because pointer is destroyed after sql_finalize
		// that also means it need to be freed when not used
		course_code = _strdup(sqlite3_column_text(stmt, 1));
		grade = _strdup(sqlite3_column_text(stmt, 4));

		// handling allocation failure
		if ((course_code == NULL) || (grade == NULL)) {  // allocation failure, will NOT return Student
			fprintf(stderr, "Allocation error when duplicating course_code and grade.");
			return;
		};

		// initialize Course
		buff[i] = (Course){
			course_code,
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			grade
		};
	};

	sqlite3_finalize(stmt);
	return 0;
}

Student get_student(sqlite3* db, char* stud_id) {
	/* Get Student with their student_id from `students` table 
	
	Remember to free Student.courses array.
	*/
	sqlite3_stmt* stmt;
	Student student = { 0 };

	int ret = sqlite3_prepare_v2(
		db,
		"SELECT * FROM students WHERE student_id=?",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return;
	};
	
	sqlite3_bind_text(stmt, 1, stud_id, strlen(stud_id), NULL);

	// run the sql
	ret = sqlite3_step(stmt);

	char* student_id;
	char* student_name;
	if (ret == SQLITE_ROW) {
		int number_of_courses = get_number_of_courses(db, stud_id);

		// courses get put into the courses buffer
		Course* courses = calloc(number_of_courses, sizeof(Course));
		get_student_courses(db, stud_id, courses);

		if (courses == NULL) {
			fprintf(stderr, "Courses cannot be loaded. Exiting the function");
			return;
		};

		// duplicating string because pointer is destroyed after sql_finalize
		// that also means it need to be freed when not used
		student_id = _strdup(sqlite3_column_text(stmt, 0));
		student_name = _strdup(sqlite3_column_text(stmt, 1));

		// handling allocation failure
		if ((student_id == NULL) || (student_name == NULL)) {  // allocation failure, will NOT return Student
			fprintf(stderr, "Allocation error when duplicating student_id and name.");
			return;
		};

		// constructing Student
		student = (Student){
			student_id,
			student_name,
			courses,
			number_of_courses
		};

		sqlite3_finalize(stmt);
		return student;
	};

	sqlite3_finalize(stmt);
	return;
}

// utils function for freeing memory allocated after finishing
int free_student(Student student) {
	free(student.id);
	free(student.name);

	for (int i = 0; i < student.number_of_courses; i++) {
		free(student.courses[i].course_code);
		free(student.courses[i].grade);
	};
	free(student.courses);
}
