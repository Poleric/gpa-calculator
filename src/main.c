#include <stdio.h>
#include <student.h>
#include <calculation.h>
#include <db.h>
#include <utils.h>
#include <string.h>
#include <stdlib.h>
#include <gui.h>

#define LEN(x) sizeof(x)/sizeof(x[0])

int main() {
	// Constructing Student
	Course test1 = {
		"AAA1003",
		1,
		3,
		"B+"
	};
	Course test2 = {
		"AAA1014",
		1,
		4,
		"A-"
	};
	Course pc = {
		"PC101",
		2,
		3,
		"B"
	};
	Course awawa = {
		"AWAWA",
		2,
		2,
		"A"
	};
	Course aba = {
		"ABA101",
		3,
		4,
		"B+"
	};
	Course* pCourses[5] = { &test1, &test2, &pc, &awawa, &aba };
	Student stud1 = {
		"WB01234",
		"STUDENT",
		pCourses,
		LEN(pCourses)
	};

    // before sql
    printf("Before SQL\n");
    // courses enrolled
    for (int i = 0; i < stud1.number_of_courses; i++) {
        printf("course_code=%s sem=%d credit_hours=%d grade=%s\n", stud1.pCourses[i]->course_code, stud1.pCourses[i]->sem, stud1.pCourses[i]->credit_hours, stud1.pCourses[i]->grade);
    };
    // names
    printf("\nStudent student_id=%s name=%s\n", stud1.student_id, stud1.name);
    // gpa and cgpa
    printf("\nGPA (sem 2)=%f CGPA=%f\n", get_student_gpa(&stud1, 2), get_student_cgpa(&stud1));

    printf("\n");
	// creating db in memory
	sqlite3* db;
	sqlite3_open(":memory:", &db);
	init_db(db);

	// storing student
	if (store_student(db, &stud1) == 0)
		printf("stored Student\n");
	else
		printf("error encountered storing student\n");
	if (store_student_courses(db, &stud1) == 0)
		printf("stored student's Courses\n");
	else
		printf("error encountered storing courses\n");

	printf("\n");
	// getting student
	printf("Getting Student %s\n", "WB01234");
	SQLStudent* pSQLStudent = get_student(db, "WB01234");

	// printing values
	// courses enrolled
	for (int i = 0; i < pSQLStudent->number_of_courses; i++) {
		printf("row_id=%d course_code=%s sem=%d credit_hours=%d grade=%s\n", pSQLStudent->pSQLCourses[i]->row_id, pSQLStudent->pSQLCourses[i]->course_code, pSQLStudent->pSQLCourses[i]->sem, pSQLStudent->pSQLCourses[i]->credit_hours, pSQLStudent->pSQLCourses[i]->grade);
	};
	// names
    display_students_list(db);
	// gpa and cgpa
	printf("\nGPA (sem 2)=%f CGPA=%f\n", get_student_gpa((Student*)pSQLStudent, 2), get_student_cgpa((Student*)pSQLStudent));

    printf("\n");
    // updating course
    SQLCourse *ptr = (SQLCourse*)get_course_with_id("PC101", (Course**)pSQLStudent->pSQLCourses, pSQLStudent->number_of_courses);
    free(ptr->grade);
    ptr->grade = strdup("A");
    update_sql_student_courses(db, pSQLStudent);
    printf("Updating student course.\n");

    printf("\n");
    free_student(pSQLStudent);

    // getting student again
    printf("Getting Student %s again\n", "WB01234");
    pSQLStudent = get_student(db, "WB01234");

    // printing values
    // courses enrolled
    for (int i = 0; i < pSQLStudent->number_of_courses; i++) {
        printf("row_id=%d course_code=%s sem=%d credit_hours=%d grade=%s\n", pSQLStudent->pSQLCourses[i]->row_id, pSQLStudent->pSQLCourses[i]->course_code, pSQLStudent->pSQLCourses[i]->sem, pSQLStudent->pSQLCourses[i]->credit_hours, pSQLStudent->pSQLCourses[i]->grade);
    };
    // names
    display_students_list(db);
    // gpa and cgpa
    printf("\nGPA (sem 2)=%f CGPA=%f\n", get_student_gpa((Student*)pSQLStudent, 2), get_student_cgpa((Student*)pSQLStudent));

    printf("Number of students = %d", get_number_of_students(db));
	// cleanup
	free_student(pSQLStudent);
	sqlite3_close(db);
    //	start_admin_screen();
}
