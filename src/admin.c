#include <gui.h>
#include <time.h>
#include <string.h>
#include <calculation.h>
#include <utils.h>
#include <admin.h>

#define EXIT_FLAG (-1)

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

sqlite3* db;

int main() {
	int id, exit = 0;
	sqlite3_open("students.db",&db);  //connect database
    init_student_db(db);

    do {
        printf("          GPA/CGPA CALCULATOR          \n");
        printf("=======================================\n");
        printf("ADMIN ENTER 1\n");
        printf("STUDENT ENTER 2\n");
        printf("\n");
        printf("EXIT ENTER %d\n", EXIT_FLAG);
        printf("ENTER AS ADMIN/STUDENT : ");
        do {
            scanf("%d", &id);

            switch (id) {
                case EXIT_FLAG:
                    exit = 1;
                    break;
                case 1:
                    putchar('\n');
                    admin();
                    break;
                case 2:
                    putchar('\n');
                    student();
                    break;
                default:
                    printf("PLEASE CHOOSE THE NUMBER AGAIN: ");
                    id = 0;
            }
        } while (!id);  // id != 0
        clear_screen();
    } while(!exit);

	sqlite3_close(db);  //close database
}

void admin(){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("=======================================\n");
	printf("==        GPA/CGPA CALCULATOR        ==\n");
	printf("=======================================\n");
	printf("ADMINISTRATOR\n");
	printf("SCHOOL: KOLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	switch (adminLogin()) {
        case EXIT_SUCCESS:  // success login
            break;
        case EXIT_FAILURE:  // fail login
            pause();
            return;
    }

    int exit = 0;
    int option;

    SQLStudent* student = get_student(db, "WBA0000ND");

    do {
        clear_screen();
        printf("WELCOME TO THE ADMIN SCREEN\n");
        printf("=======================================\n");
        printf("VIEW STUDENT LIST ENTER 1\n");
        printf("GET A STUDENT DETAILS ENTER 2\n");
        printf("STORE STUDENT ENTER 3\n");
        printf("UPDATE STUDENT ENTER 4\n");
        printf("\n");
        printf("EXIT ENTER %d\n", EXIT_FLAG);
        printf("ENTER: ");
        do {
            scanf("%d", &option);
            flush_stdin();
            switch (option) {
                case 1:
                    student_list_menu(db);
                    break;
                case 2:
                    putchar('\n');
                    clear_screen();
                    printFullStudentDetails(student);
                    pause();
                    break;
                case 3:
                case 4:
                    break;
                case EXIT_FLAG:
                    exit = 1;
                    break;
                default:
                    break;
            }
        } while (!option);  // option == 0
        clear_screen();
    } while (!exit); // exit == 0

    free_student(student);
}

int adminLogin() {
    char input[30];

    printf("ENTER THE PASSWORD: ");
    flush_stdin();
    for(int tries = 3; ; tries--){
        fgets(input, ARRAY_SIZE(input), stdin);  // gets is removed from C11
        input[strcspn(input, "\n")] = '\0';  // remove trailing newline from fgets

        if(strcmp(input,"123456789abc") == 0) {
            return EXIT_SUCCESS;
        } else if (tries > 0) {
            printf("PASSWORD INCORRECT\n");
            printf("PLEASE ENTER THE PASSWORD AGAIN(%i): ", tries);
        } else {
            printf("!!!YOU HAVE ENTER THE INCORRECT ANSWER TOO MANY TIMES, PLEASE TRY AGAIN LATER!!!\n");
            printf("================================================================================\n");
            printf("If you have forgotten the password, please contact to look for help:012-879-3965\n");
            printf("OR send us a email:ColejPasarService@gmail.com\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("\n");
            return EXIT_FAILURE;
        }
    }
}

void student(){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char studentID[100];
	SQLStudent* student;

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

    putchar('\n');
	printStudentDetails(student);
	free_student(student);  //free memory
    flush_stdin();
    pause();
}

void printStudentDetails(SQLStudent* student) {

	SQLCourse* course;
	char no[4];

    printf("DETAILS:\n");
    printf("=======================================\n");
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

void printFullStudentDetails(SQLStudent* student) {
    int max_sem;

    printf("DETAILS:\n");
    printf("=============================================\n");
    printf("Student ID: %s\n", student->name);
   printf("Student Name: %s\n", student->student_id);
    printf("Total enrolled courses: %d\n", student->number_of_courses);

    max_sem = get_max_sem(db);
    printStudentCoursesTable(student, max_sem);
}

void printManyChar(char character, int length) {
    for (int i = 0; i < length; i++) {
        putchar(character);
    }
}

void printLineWithManyChar(char character, int length) {
    printManyChar(character, length);
    putchar('\n');
}

void printLineWithManyCharWithSeperators(char character, int length, char seperator, int number_of_times, bool trailing_seperator) {
    for (int i = 0; i < number_of_times; i++) {
        printManyChar(character, length);

        if (i != number_of_times - 1 + trailing_seperator)  // TRUE cancels out -1
            putchar(seperator);
    }
    putchar('\n');
}

void printStudentCoursesTable(SQLStudent* student, int max_sem) {
    const int COLUMN_WIDTH = 15;

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);

    for (int sem = 1; sem <= max_sem; sem++) {
        printf(" Sem %-*d", COLUMN_WIDTH - 5, sem);
        printf("|");
    }
    putchar('\n');

    printLineWithManyCharWithSeperators('-', COLUMN_WIDTH, '|', max_sem, TRUE);

    SQLCourse*** courses_each_sem = calloc(max_sem, sizeof(SQLCourse**));
    for (int i = 0; i < max_sem; i++) {
        courses_each_sem[i] = calloc(student->number_of_courses, sizeof(SQLCourse*));
        filter_sem_courses(i + 1, (Course**)student->pSQLCourses, student->number_of_courses,(Course **) courses_each_sem[i]);
    }
    // line 3++
    for (int row = 0; row < student->number_of_courses; row++) {
        int stop = 1;
        for (int sem_index = 0; sem_index < max_sem; sem_index++) {
            SQLCourse* course = courses_each_sem[sem_index][row];
            if (course != NULL) {
                stop = 0;
                printf(" %s", course->course_code);
                printf(" > ");
                printf("%-*s", COLUMN_WIDTH - 5 - (int)strlen(course->course_code), course->grade);
                printf(" |");
            } else {
                printManyChar(' ', COLUMN_WIDTH);
                putchar('|');
            }
        }
        putchar('\n');
        if (stop) break;
    }

    // free alloc
    for (int i = 0; i < max_sem; i++) {
        free(courses_each_sem[i]);
    }
    free(courses_each_sem);

    printLineWithManyCharWithSeperators('-', COLUMN_WIDTH, '|', max_sem, TRUE);

    for (int sem = 1; sem <= max_sem; sem++) {
        printf("  GPA: %-*.2f", COLUMN_WIDTH - 7, get_gpa_from_courses((Course**)student->pSQLCourses, student->number_of_courses, sem));
        printf("|");
    }
    putchar('\n');

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);

    printf(" CGPA: %-*.2f", COLUMN_WIDTH*max_sem + max_sem - 8, get_cgpa_from_courses((Course**)student->pSQLCourses, student->number_of_courses));
    printf("|\n");

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);
}