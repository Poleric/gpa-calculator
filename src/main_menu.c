#include <gui.h>
#include <time.h>
#include <string.h>
#include <calculation.h>
#include <utils.h>
#include <main_menu.h>

#include <locale.h>
#include <libintl.h>
#define _(String) gettext(String)
#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

#ifdef LOCALE_DIR
#else
#define LOCALE_DIR "/usr/local/share/locale"
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#include <Windows.h>
// windows no setenv
int setenv(const char* name, const char* value, int overwrite)
{
    int errcode = 0;
    if (!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if (errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#endif

#define EXIT_FLAG (-1)

// language defines
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define ENGLISH_UK "English_UnitedKingdom"
#define ENGLISH_US "English_UnitedStates"
#define CHINESE_CN "Chinese-Simplified"
#define MALAY_MY "Malay"
#else
#define ENGLISH_UK "en_GB"
#define ENGLISH_US "en_US"
#define CHINESE_CN "zh_CN"
#define MALAY_MY "ms_MY"
#endif
#define SUPPORTED_LANGUAGES_TEXT {"English (United Kingdom)", "English (United State)", "简体中文", "Bahasa Melayu"}
#define SUPPORTED_LANGUAGES_CODES {ENGLISH_UK, ENGLISH_US, CHINESE_CN, MALAY_MY}

sqlite3* db;

int main() {
    // initialize locales
	int id, exit = 0;
    char* locale;

    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    SetConsoleOutputCP(CP_UTF8)
    SetConsoleCP(CP_UTF8)
    #endif

	sqlite3_open("students.db",&db);  //connect database
    init_student_db(db);

    clear_screen();
    locale = promptLanguage();
    locale = setLocale(locale);
    if (locale == NULL) {
        fprintf(stderr, _("Failed to set language. Does your system does not support this language?"));
        putchar('\n');
        pause();
    }

    do {
        clear_screen();
        printf("=======================================\n");
        printf("          %s          \n", _("GPA/CGPA CALCULATOR"));  // 10 padded left right, 39 wide field
        printf("=======================================\n");
        printf(_("ADMIN ENTER 1\n"));
        printf(_("STUDENT ENTER 2\n"));
        printf("\n");
        printf(_("EXIT ENTER %d\n"), EXIT_FLAG);
        printf(_("ENTER AS ADMIN/STUDENT : "));
        do {
            scanf("%d", &id);
            flush_stdin();

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
                    printf(_("PLEASE CHOOSE THE NUMBER AGAIN: "));
                    id = 0;
            }
        } while (!id);  // id != 0
        clear_screen();
    } while(!exit);
	sqlite3_close(db);  //close database
}

char* promptLanguage() {
    /* Returns the corresponding locale code the user selected */
    char* supported_langs[] = SUPPORTED_LANGUAGES_TEXT;
    char* supported_langs_code[] = SUPPORTED_LANGUAGES_CODES;
    int input_num;

    printf("======================================\n");
    printf(_("Language\n"));
    printf("======================================\n");
    for (int i = 0; i < ARRAY_SIZE(supported_langs); i++)
        printf("[%d] %s\n", i + 1, supported_langs[i]);

    putchar('\n');

    do {
        printf(_("Enter a number > "));
        scanf("%d", &input_num);
        flush_stdin();

        if (input_num > ARRAY_SIZE(supported_langs))
            printf(_("%d is not a valid option. Please input again.\n"), input_num);
        else
            break;
    } while (1);

    return supported_langs_code[input_num - 1];
}

char* setLocale(char* lang_code) {
    char* set_locale;

    setenv("LANG", lang_code, 1);
    setenv("LANGUAGE", lang_code, 1);
    set_locale = setlocale(LC_ALL, "");
    if (set_locale == NULL) {  // add .UTF-8 if fail
        char lang_utf[strlen(lang_code) + 1 + 6];

        strcpy(lang_utf, lang_code);
        strcat(lang_utf, ".UTF-8");

        set_locale = setlocale(LC_ALL, lang_utf);
    }
    bindtextdomain("gpa-calculator", LOCALE_DIR);
    textdomain("gpa-calculator");
    return set_locale;
}


void admin(){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("=======================================\n");
	printf("==        %s        ==\n", _("GPA/CGPA CALCULATOR")); // 10 padded left right, 39 wide field.
	printf("=======================================\n");
	printf(_("ADMINISTRATOR\n"));
	printf(_("SCHOOL: KOLEJ PASAR\n"));
	printf(_("DATE: %s,%d-%02d-%02d\n"), get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	switch (adminLogin()) {
        case EXIT_SUCCESS:  // success login
            break;
        case EXIT_FAILURE:  // fail login
            pause();
            return;
    }

    int option, exit = 0;
    do {
        clear_screen();
        printf("=======================================\n");
        printf("     %s\n", _("WELCOME TO THE ADMIN SCREEN"));  // 5 space padded left, 39 wide field
        printf("=======================================\n");
        printf(_("VIEW student list ENTER 1\n"));
        printf(_("GET a student details ENTER 2\n"));
        printf(_("STORE student ENTER 3\n"));
        printf(_("DELETE student ENTER 4\n"));
        printf("\n");
        printf(_("EXIT ENTER %d\n"), EXIT_FLAG);
        printf(_("ENTER: "));
        do {
            scanf("%d", &option);
            flush_stdin();
            switch (option) {
                case 1:
                    student_list_menu(db);
                    break;
                case 2:
                    clear_screen();
                    getStudentDetailsScreen();
                    break;
                case 3:
                    clear_screen();
                    insert_student_menu(db);
                    break;
                case 4:
                    clear_screen();
                    promptDeletion();
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
}

int adminLogin() {
    char input[30];

    printf(_("ENTER THE PASSWORD: "));
    for(int tries = 3; ; tries--){
        fgets(input, ARRAY_SIZE(input), stdin);  // gets is removed from C11
        input[strcspn(input, "\n")] = '\0';  // remove trailing newline from fgets

        if(strcmp(input,"123456789abc") == 0) {
            return EXIT_SUCCESS;
        } else if (tries > 0) {
            printf(_("PASSWORD INCORRECT\n"));
            printf(_("PLEASE ENTER THE PASSWORD AGAIN(%i): "), tries);
        } else {
            printf(_("!!!YOU HAVE ENTER THE INCORRECT ANSWER TOO MANY TIMES, PLEASE TRY AGAIN LATER!!!\n"));
            printf("================================================================================\n");
            printf(_("If you have forgotten the password, please contact to look for help: %s\n"), "012-879 3965");
            printf(_("OR send us a email: %s\n"), "ColejPasarService@gmail.com");
            printf("--------------------------------------------------------------------------------\n");
            printf("\n");
            return EXIT_FAILURE;
        }
    }
}

void student(){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("=======================================\n");
	printf("==        %s        ==\n", _("GPA/CGPA CALCULATOR"));
	printf("=======================================\n");
	printf(_("STUDENT\n"));
	printf(_("SCHOOL: KOLEJ PASAR\n"));
	printf(_("DATE: %s,%d-%02d-%02d\n"), get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	getStudentDetailsScreen();
}

void getStudentDetailsScreen() {
    SQLStudent *student;

    student = promptStudentID();

    if (student == NULL) {  // user select exit
        return;
    }

    putchar('\n');
    printFullStudentDetails(student);

    free_student(student);  //free memory
    pause();  // pause screen to see
}

SQLStudent* promptStudentID() {
    char studentID[15];
    const char* EXIT_STR = "-1";
    SQLStudent *student;

    do {
        printf(_("PLEASE ENTER THE STUDENT ID: "));
        fgets(studentID, ARRAY_SIZE(studentID), stdin);  // gets is removed from C11
        studentID[strcspn(studentID, "\n")] = '\0';  // remove trailing newline from fgets

        if (strcmp(studentID, EXIT_STR) == 0) {
            return NULL;
        }

        student = get_student(db, studentID);  //read student from database

        if (student == NULL) {
            printf(_("You entered a wrong ID (Enter %s to exit)\n"), EXIT_STR);
            continue;
        }

        return student;
    } while(1);
}

void promptDeletion() {
    SQLStudent* student;
    int input;

    student = promptStudentID();

    if (student == NULL) {  // user select exit
        return;
    }

    putchar('\n');
    printFullStudentDetails(student);

    putchar('\n');
    puts(_("Are you sure you want to delete this entry?"));
    puts(_("ENTER 0 to CANCEL"));
    puts(_("ENTER 1 to DELETE"));
    putchar('\n');

    do {
        printf(_("> "));  // this is a number input

        scanf("%d", &input);
        flush_stdin();

        switch (input) {
            case 0:
                break;
            case 1:
                delete_sql_student_courses(db, student);
                delete_sql_student(db, student);
                input = 0;
                break;
            default:
                puts(_("The number you've entered is invalid. Please enter again."));
        }
    } while(input);  // input != 0

    free_student(student);
}

void printFullStudentDetails(SQLStudent* student) {
    printf(_("DETAILS:\n"));
    printf("================================================\n");
    printf(_("Student ID: %s\n"), student->student_id);
    printf(_("Student Name: %s\n"), student->name);
    printf(_("Total enrolled courses: %d\n"), student->number_of_courses);

    printStudentCoursesTable(student);
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

void printStudentCoursesTable(SQLStudent* student) {
    const int COLUMN_WIDTH = 15;
    int max_sem = get_max_sem(db);

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);

    for (int sem = 1; sem <= max_sem; sem++) {
        printf(" ");
        printf(_("Sem %-*d"), COLUMN_WIDTH - 5, sem);
        printf("|");
    }
    putchar('\n');

    printLineWithManyCharWithSeperators('-', COLUMN_WIDTH, '|', max_sem, TRUE);

    SQLCourse*** courses_each_sem = calloc(max_sem, sizeof(SQLCourse**));
    if (courses_each_sem == NULL) {
        log_alloc_error("printStudentCoursesTable", "courses_each_sem");
        return;
    }

    for (int i = 0; i < max_sem; i++) {
        SQLCourse** courses = calloc(student->number_of_courses, sizeof(SQLCourse*));
        if (courses == NULL) {
            log_alloc_error("printStudentCoursesTable", "courses");
            return;
        }

        courses_each_sem[i] = courses;
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
        printf("  ");
        printf(_("GPA: %-*.2f"), COLUMN_WIDTH - 7, get_gpa_from_courses((Course**)student->pSQLCourses, student->number_of_courses, sem));
        printf("|");
    }
    putchar('\n');

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);

    printf(" ");
    printf(_("CGPA: %-*.2f"), COLUMN_WIDTH*max_sem + max_sem - 8, get_cgpa_from_courses((Course**)student->pSQLCourses, student->number_of_courses));
    printf("|\n");

    printLineWithManyChar('=', (COLUMN_WIDTH+1)*max_sem);
}