#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <student.h>
#include <db.h>

sqlite3* init_db(char* path) {
	/* Initialize db and create the tables needed */

	sqlite3* db;
	char* exc;

	int ret;
	ret = sqlite3_open(path, &db);
	if (ret != SQLITE_OK) {
		// close db and return early
		fprintf(stderr, "Cannot open db: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	ret = sqlite3_exec(
		db,
		"CREATE TABLE IF NOT EXISTS students ("
		"student_id VARCHAR(9) PRIMARY KEY,"
		"student_name TEXT NOT NULL"
		");"
		"CREATE TABLE registered_courses ("
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
	}

	return db;
}

int store_student(sqlite3* db, Student student) {
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

	// sql is one-indexed
	sqlite3_bind_text(stmt, 1, student.id, strlen(student.id), NULL);  // (stmt, index, value, bytesize, flag) 
	sqlite3_bind_text(stmt, 2, student.name, strlen(student.name), NULL);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int store_student_courses(sqlite3* db, Student student, size_t course_len) {
	sqlite3_stmt* stmt;

	int ret = sqlite3_prepare(
		db,
		"INSERT OR REPLACE INTO students (course_code, semester, credit_hours, grade, student_id) VALUES (?, ?, ?, ?, ?);",
		-1,
		&stmt,
		NULL
	);

	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	for (int i = 0; i < course_len; i++) {
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

/*
int main() {
	sqlite3* db = init_db("test.db");
	sqlite3_close(db);
	return 0;
}
*/