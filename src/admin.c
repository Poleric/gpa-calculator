#include <stdio.h>	
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <db.h>
#include <calculation.h>
#include <student.h>

char* get_day(int day);
void admin ();
void student ();


void clear_screen(){
	system("cls");
}



void main() {

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char input[30];
	int tries = 3;
	int id;

	printf("          GPA/CGPA CALCULATOR          \n");
	printf("=======================================\n");
	printf("ADMIN ENTER 1\n");
	printf("STUDENT ENTER 2\n");
	printf("ENTER AS ADMIN/STUDENT : ");
	do{
		rewind(stdin);
		scanf("%i",&id);

	switch(id){
		case 1:
			admin();
			break;
		case 2:
			student();
			break;
		default:
		printf("PLEASE CHOOSE THE NUMBER AGAIN: ");
	}}while(1);
}

void admin (){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char input[30];
	int tries = 3;

	printf("=======================================\n");
	printf("==        GPA/CGPA CALCULATOR        ==\n");
	printf("=======================================\n");
	printf("ADMINISTRATOR\n");
	printf("SCHOOL: COLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	printf("ENTER THE PASSWORD: ");
	rewind(stdin);

	
	for(tries = 3; ; tries--){
		gets(input);
		if(strcmp(input,"123456789abc") == 0) {
			// clear_screen();
			printf("WELCOME TO THE ADMIN SCREEN");
			break;
		}
		else if(tries > 0){
			// clear_screen();
			printf("PASSWORD INCORRECT\n");
			printf("PLEASE ENTER THE PASSWORD AGAIN(%i): ",tries);
			// clear_screen();
		}
		else{
			printf("!!!YOU HAVE ENTER THE INCORRECT ANSWER TOO MANY TIMES, PLEASE TRY AGAIN LATER!!!\n");
			printf("================================================================================\n");
			printf("If you have forgotten the password, please contact to look for help:012-879-3965\n");
			printf("OR send us a email:ColejPasarService@gmail.com\n");
			printf("--------------------------------------------------------------------------------\n");
			printf("\n");
			break;
		}
    }

}

void student(){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char studentID[100];
	SQLStudent* student;
	SQLCourse* course;
	sqlite3* db;

	printf("=======================================\n");
	printf("==        GPA/CGPA CALCULATOR        ==\n");
	printf("=======================================\n");
	printf("ADMINISTRATOR\n");
	printf("SCHOOL: COLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	printf("PLEASE ENTER YOUR STUDENT ID: ");
	scanf("%s",&studentID);  //get student id
	sqlite3_open("students.db",&db);  //connect database
	student = get_student(db,studentID);  //read student from database
	printf("%s\n",student -> name);  //print student name from the database 
	printf("%s\n",student -> student_id);  //print student id from the database 

	course = student -> pSQLCourses[0];  //read the first course from student
	printf("%s\n",course ->course_code);  //read course code from first course
	printf("%i\n",course ->credit_hours);  //read credit hours from course
	printf("%s\n",course ->grade);  //read grade from course


	printf("%f",get_gpa_from_courses((Course**)student -> pSQLCourses,student -> number_of_courses,1));


	free_student(student);  //free memory
	sqlite3_close(db);  //close database
}







char* get_day(int day) {

	switch (day) {
	case 0:
		return "MONDAY";
	case 1:
		return "TUESDAY";
	case 2:
		return "WEDNESDAY";
	case 3:
		return "THURSDAY";
	case 4:
		return "FRIDAY";
	case 5:
		return "SATURDAY";
	case 6:
		return "SUNDAY";
	}
}



