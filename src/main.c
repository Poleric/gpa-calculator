#include <stdio.h>
#include <student.h>
#include <calculation.h>
#include <db.h>
#include <stdlib.h>
#include <admin.h>

#define LEN(x) sizeof(x)/sizeof(*x)

int main() {
	//// Constructing Student
	//Course test1 = {
	//	NULL,
	//	"AAA1003",
	//	1,
	//	3,
	//	"B+"
	//};
	//Course test2 = {
	//	NULL,
	//	"AAA1014",
	//	1,
	//	4,
	//	"A-"
	//};
	//Course pc = {
	//	NULL,
	//	"PC101",
	//	2,
	//	3,
	//	"B"
	//};
	//Course awawa = {
	//	NULL,
	//	"AWAWA",
	//	2,
	//	2,
	//	"A"
	//};
	//Course aba = {
	//	NULL,
	//	"ABA101",
	//	3,
	//	4,
	//	"B+"
	//};
	//Course* pCourses[5] = { &test1, &test2, &pc, &awawa, &aba };
	//Student stud1 = {
	//	"WB01234",
	//	"STUDENT",
	//	pCourses,
	//	LEN(pCourses)
	//};

	//// creating db in memory
	//sqlite3* db;
	//sqlite3_open(":memory:", &db);
	//init_db(db);
	//
	//// storing student
	//if (store_student(db, &stud1) == 0)
	//	printf("stored Student\n");
	//else
	//	printf("error encountered storing student\n");
	//if (store_student_courses(db, &stud1) == 0)
	//	printf("stored student's Courses\n");
	//else
	//	printf("error encountered storing courses\n");

	//printf("\n");
	//// getting student
	//printf("Getting Student %s\n", "WB01234");
	//Student* pStudent = get_student(db, "WB01234");

	//// printing values
	//// courses enrolled
	//for (int i = 0; i < pStudent->number_of_courses; i++) {
	//	printf("sql_id=%d course_code=%s sem=%d credit_hours=%d grade=%s\n", pStudent->pCourses[i]->sql_id, pStudent->pCourses[i]->course_code, pStudent->pCourses[i]->sem, pStudent->pCourses[i]->credit_hours, pStudent->pCourses[i]->grade);
	//};
	//// names
	//printf("\nStudent id=%s name=%s\n", pStudent->id, pStudent->name);
	//// gpa and cgpa
	//printf("\nGPA (sem 2)=%f CGPA=%f\n", get_student_gpa(pStudent, 2), get_student_cgpa(pStudent));

	//// cleanup
	//free_student(pStudent);
	//sqlite3_close(db);
	start_admin_screen();
}
