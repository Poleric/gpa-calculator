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
sqlite3* db;


void clear_screen(){
	system("cls");
}

int main() {
	int id;
	sqlite3_open("students.db",&db);  //connect database

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
        }
    } while(id == 1 || id == 2);
	
	sqlite3_close(db);  //close database
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
	printf("SCHOOL: KOLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	printf("ENTER THE PASSWORD: ");
	rewind(stdin);

	
	for(tries = 3; ; tries--){
		fgets(input, sizeof(input)/sizeof(input[0]), stdin);  // gets is removed from C14
        input[strcspn(input, "\n")] = '\0';  // remove trailing newline from fgets
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

	printf("=======================================\n");
	printf("==        GPA/CGPA CALCULATOR        ==\n");
	printf("=======================================\n");
	printf("ADMINISTRATOR\n");
	printf("SCHOOL: COLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	printf("PLEASE ENTER YOUR STUDENT ID: ");
	scanf("%s",&studentID);  //get student id
	student = get_student(db,studentID);  //read student from database

	if (student == NULL){
		printf("You entered a wrong ID");
		return;
	}

	printStudentDetails(student);

	free_student(student);  //free memory
}

void printStudentDetails(SQLStudent* student) {

	SQLCourse* course;
	char no[4];

	printf("Student : %s\n",student -> name);  //print student name from the database 
	printf("\t     -----------------\n");
	printf("\t           GRADE      \n");
	printf("\t     -----------------\n");

	for (int num = 0; num < student -> number_of_courses; num++) {

		course = student -> pSQLCourses[num];  //read the first course from student
		snprintf(no, 4, "%d.", num+1);
		printf("\t     %-3s %s > %s\n", no, course ->course_code,course ->grade);  //read course code from first course

	}	
	printf("\n");
	printf("\t  ---------   -----------\n");
	printf("\t     SEM          GPA    \n");
	printf("\t  ---------   -----------\n");

	for (int sem = 1; sem <= get_max_sem(db); sem++){
		printf("\t      %d\t\t  %.2f\n", sem, get_student_gpa((Student*)student, sem));
	}
	printf("\t\t      -----------\n");
	printf("\t\t          CGPA   \n");
	printf("\t\t      -----------\n");
	printf("\t\t\t  %.2f\n",get_student_cgpa((Student*)student));

}
