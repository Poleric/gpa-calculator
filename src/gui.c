#include <stdlib.h>
#include <db.h>
#include <string.h>
#include <calculation.h>
#include <utils.h>
#include <stdarg.h>
#include <main_menu.h>
#include <gui.h>

#include <locale.h>
#include <libintl.h>

#define _(String) gettext(String)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define strdup _strdup  //  warning C4996: 'strdup': The POSIX name for this item is deprecated.
#endif

// this entire module is a mistake

#define FIELD_SEPERATE_LEN 2

#define ID_FIELD_LEN 10
#define MINIMUM_NAME_FIELD_LEN 12
#define GPA_FIELD_LEN 8
#define CGPA_FIELD_LEN 8

#define ID_FIELD_STRING _("Student ID")
#define NAME_FIELD_STRING _("Student Name")
#define GPA_FIELD_STRING _("GPA (S%d)")
#define CGPA_FIELD_STRING _("CGPA")

#define wstandout_line(win, row, color_pair) mvwchgat(win, row, 0, -1, A_STANDOUT, color_pair, NULL)

// used by student_list_menu
WINDOW* student_list_win;
FieldData field_data;

int sort_gpa_sem;  // workaround for compare_gpa by semester.
const int N_COLOR = 3;

void init_color_pairs() {
    /* Initialize preset colors pairs
     * 1 - Green background
     * 2 - Cyan background
     * 3 - Yellow background
     */
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}

int student_list_menu(sqlite3* db) {
    // ncurses
    initscr();      // start
    if(has_colors() == FALSE) {
        endwin();
        printf(_("Your terminal does not support color\n"));
        return EXIT_FAILURE;
    }
    start_color(); init_color_pairs();
    cbreak();       // get input for each character
    noecho();       // no display input

    // initialize stuff that might be used
    int max_students = get_number_of_students(db);
    int max_sem = get_max_sem(db);

    init_field_data(COLS, LINES-3, max_sem);
    if (field_data.width > COLS) {
        endwin();
        fprintf(stderr, _("Screen is too small, resize to be atleast %d wide.\n"), field_data.width);
        return EXIT_FAILURE;
    }

    // print title, header, footer
    wprintw_center(stdscr, COLS, _("Kolej Pasar Students"));
    move(1, 0);        // print table header below title
    wprintw_header(stdscr, TRUE);
    move(LINES-1, 2);  // print footer at the bottom, move right 2
    wprintw_footer(stdscr, TRUE);
    refresh();

    field_data.rows = calloc(max_students, sizeof(RowData));
    field_data.number_of_rows = max_students;
    init_rows(db);

    int current_row = 0, selection = 0, update = 1;
    int input;
    int sort_mode = 1; // 1 - id, 2 - name, 3+ - gpa, ..., cgpa

    student_list_win = newwin(field_data.height, field_data.width, 2, 0);
    keypad(student_list_win, TRUE);
    leaveok(student_list_win, TRUE);
    do {
        if (update) {
            wstandout_line(stdscr, 1, 1);
            standout_sorted_header(sort_mode, 2);
            refresh();

            update_student_list_window(current_row);

            int selected_row = current_row < max_students - field_data.height ? 0 : selection - current_row;
            wstandout_line(student_list_win, selected_row, 2);
            wrefresh(student_list_win);

            wmove(student_list_win, 0, 0);
            update = 0;
        }

        input = wgetch(student_list_win);
        switch(input) {
            // scroll up and down
            case KEY_UP:
                if (current_row > 0 && selection < max_students - field_data.height + 1) {  // magic numbers
                    current_row--;
                    update = 1;
                }
                if (selection > 0) {
                    selection--;
                    update = 1;
                }
                break;
            case KEY_DOWN:
                if (current_row < max_students - field_data.height) {
                    current_row++;
                    update = 1;
                }
                if (selection < max_students - 1) {
                    selection++;
                    update = 1;
                }
                break;
            // change sorting option
            case KEY_LEFT:
                if (sort_mode > 1) {
                    sort_mode--;
                    sort_row(sort_mode);
                    update = 1;
                }
                break;
            case KEY_RIGHT:
                if (sort_mode < 3 + field_data.semCols) {
                    sort_mode++;
                    sort_row(sort_mode);
                    update = 1;
                }
                break;
            case KEY_ENTER:
            case '\n':
                def_prog_mode();
                endwin();

                clear_screen();
                SQLStudent* stud = get_student(db, field_data.rows[selection].studentID);
                printFullStudentDetails(stud);
                free_student(stud);
                getchar();

                reset_prog_mode();
                refresh();
                break;
            default:
                break;
        }
    } while (input != 'q');
    free_rows();
    wclear(student_list_win); clear();
    delwin(student_list_win); endwin();
    return EXIT_SUCCESS;
}

int insert_student_menu(sqlite3* db) {  // this is so horrible lmao
    // ncurses
    initscr();      // start
    if(has_colors() == FALSE) {
        endwin();
        printf(_("Your terminal does not support color\n"));
        return EXIT_FAILURE;
    }
    start_color(); init_color_pairs();
    noecho();
    scrollok(stdscr, TRUE);

    Student student;
    Course** courses = NULL;
    InsertFieldCoords insertFieldCoords;

    // inputs
    char student_id[20], student_name[64];
    char course_code[15], grade[3];
    int sem, credit_hours;

    // title
    wprintw_center(stdscr, COLS, _("Inserting Student"));
    wstandout_line(stdscr, 0, 0);
    // fields
    move(2, 0);
    wprint_initial_insert_student_menu(stdscr, &insertFieldCoords);
    refresh();

    echo();  // enable back display typed characters
    // accept student info
    do {
        move(insertFieldCoords.studentIdY, insertFieldCoords.studentIdX);
        clrtoeol();
        getnstr(student_id, ARRAY_SIZE(student_id));
    } while(student_id[0] == '\0');
    do {
        move(insertFieldCoords.studentNameY, insertFieldCoords.studentNameX);
        clrtoeol();
        getnstr(student_name, ARRAY_SIZE(student_name));
    } while(student_name[0] == '\0');

    const int FIELD_HEIGHT = 5;
    void * tmp;  // for checking alloc ok or not
    // accept course(s) info
    int number_of_courses = 0, selection = 0;
    do {  // might be multiple courses
        if (number_of_courses != 0) {
            move((insertFieldCoords.courseCodeY - 1) + (FIELD_HEIGHT + 1) * number_of_courses, 0);

            wprintw_center(stdscr, COLS - 8, _("Do you want to add another course? "));
            if (!yes_or_no_selector(stdscr, 1))
                break;

            wprint_course_insert_field(stdscr, number_of_courses + 2);  // +1 to compensate the ++ later, and +1 to compensate for the initial run
        }

        number_of_courses++;

        // realloc thing
        tmp = realloc(courses, number_of_courses*sizeof(Course*));
        if (tmp == NULL) {
            log_alloc_error("insert_student_name", "courses");
            free(courses);
            clear();
            endwin();
            return EXIT_FAILURE;
        }
        courses = tmp;

        // accept input from course fields
        do {
            move(insertFieldCoords.courseCodeY + (FIELD_HEIGHT + 1) * (number_of_courses - 1), insertFieldCoords.courseCodeX);
            clrtoeol();
            getnstr(course_code, ARRAY_SIZE(course_code));
        } while(course_code[0] == '\0');
        do {
            move(insertFieldCoords.semY + (FIELD_HEIGHT + 1) * (number_of_courses - 1), insertFieldCoords.semX);
            clrtoeol();
            scanw("%d", &sem);
        } while(sem <= 0);
        do {
            move(insertFieldCoords.creditHoursY + (FIELD_HEIGHT + 1) * (number_of_courses - 1), insertFieldCoords.creditHoursX);
            clrtoeol();
            scanw("%d", &credit_hours);
        } while(credit_hours <= 0);
        do {
            move(insertFieldCoords.gradeY + (FIELD_HEIGHT + 1) * (number_of_courses - 1), insertFieldCoords.gradeX);
            clrtoeol();
            getnstr(grade, ARRAY_SIZE(grade));
        } while (!is_valid_grade(grade));

        // build course
        Course* course = malloc(sizeof(Course));
        if (course == NULL) {
            log_alloc_error("insert_student_menu", "course");
            return EXIT_FAILURE;
        }
        course->course_code = strdup(course_code);  // TODO: check allocation == NULL
        course->sem = sem;
        course->credit_hours = credit_hours;
        course->grade = strdup(grade);

        courses[number_of_courses - 1] = course;
    } while (1);

    // ask to save or not
    wprintw_center(stdscr, COLS - 8, _("Save record? "));
    if (yes_or_no_selector(stdscr, 1)) {  // yes
        student = (Student) {
                student_id,
                student_name,
                courses,
                number_of_courses
        };
        store_student(db, &student);
        store_student_courses(db, &student);
    }  // no, do nothing

    for (int i = 0; i < number_of_courses; i++) {
        free(courses[i]->course_code);
        free(courses[i]->grade);
        free(courses[i]);
    }
    free(courses);

    clear();
    endwin();
    return EXIT_SUCCESS;
}

int write_student_list_window(int current_row) {
    int crow, ccol;  // current_row, current_column
    RowData row;
    for (
        int i = current_row;
        i < field_data.number_of_rows && i < current_row + field_data.height;
        i++, wmove(student_list_win, crow+1, 0)
    )
    {
        row = field_data.rows[i];

        // print student_id
        getyx(student_list_win, crow, ccol);
        wprintw(student_list_win, "%s", row.studentID);
        wmove(student_list_win, crow, ccol + field_data.idFieldLen + field_data.fieldSeperateLen);

        // print student_name
        getyx(student_list_win, crow, ccol);
        char* name = row.studentName;
        if (strlen(name) > field_data.nameFieldLen) {
            truncate_str(name, field_data.nameFieldLen);
        }
        wprintw(student_list_win, "%s", name);
        wmove(student_list_win, crow, ccol + field_data.nameFieldLen + field_data.fieldSeperateLen);

        // print gpas
        for (int j = 0; j < field_data.semCols; j++) {
            getyx(student_list_win, crow, ccol);
            wprintw_center(student_list_win, field_data.gpaFieldLen, "%.2f", row.gpas[j]);
            wmove(student_list_win, crow, ccol + field_data.gpaFieldLen + field_data.fieldSeperateLen);
        }

        // print cgpa
        wprintw_center(student_list_win, field_data.cgpaFieldLen, "%.2f", row.cgpa);
    }
    wmove(student_list_win, 0, 0);
    return EXIT_SUCCESS;
}

void update_student_list_window(int current_row) {
    wclrtobot(student_list_win);
    write_student_list_window(current_row);
}

void init_field_data(int max_width, int max_height, int max_sem) {
    // field lengths
    field_data.fieldSeperateLen = FIELD_SEPERATE_LEN; field_data.idFieldLen = ID_FIELD_LEN; field_data.gpaFieldLen = GPA_FIELD_LEN; field_data.cgpaFieldLen = CGPA_FIELD_LEN;
    field_data.nameFieldLen = max_width - ID_FIELD_LEN - max_sem*(GPA_FIELD_LEN + FIELD_SEPERATE_LEN) - CGPA_FIELD_LEN - 2*FIELD_SEPERATE_LEN;

    if (field_data.nameFieldLen < MINIMUM_NAME_FIELD_LEN) {
        field_data.width = max_width + MINIMUM_NAME_FIELD_LEN - field_data.nameFieldLen;  // math magic
        field_data.nameFieldLen = MINIMUM_NAME_FIELD_LEN;
    } else
        field_data.width = max_width;
    
    // field strings
    field_data.idFieldString = ID_FIELD_STRING; field_data.nameFieldString = NAME_FIELD_STRING; field_data.gpaFieldString = GPA_FIELD_STRING; field_data.cgpaFieldString = CGPA_FIELD_STRING;

    field_data.height = max_height;
    field_data.semCols = max_sem;
}

int init_rows(sqlite3* db) {
    sqlite3_stmt* stmt = get_all_students_stmt(db);
    RowData* pRow;

    for (
        int i = 0, ret = sqlite3_step(stmt);
        i < field_data.number_of_rows && ret == SQLITE_ROW;
        i++, ret = sqlite3_step(stmt)
    )
    {
        char* stud_id = strdup((const char*)sqlite3_column_text(stmt, 0));
        if (cannot_alloc(stud_id)) {
            log_alloc_error("get_rows", "stud_id");
            return EXIT_FAILURE;
        }

        char* stud_name = strdup((const char*)sqlite3_column_text(stmt, 1));
        if (cannot_alloc(stud_name)) {
            free(stud_id);
            log_alloc_error("get_rows", "stud_name");
            return EXIT_FAILURE;
        }

        int number_of_courses = get_number_of_courses(db, (char *)stud_id);
        SQLCourse** pSQLCourses = malloc( number_of_courses * sizeof(SQLCourse*));
        get_student_courses(db, (char*) stud_id, pSQLCourses, number_of_courses);

        float* gpas = calloc(field_data.semCols, sizeof(float));

        if (gpas == NULL) {
            log_alloc_error("init_rows", "gpas");
            free(stud_id);
            free(stud_name);
            return EXIT_FAILURE;
        }

        get_gpas_from_courses((Course**)pSQLCourses, number_of_courses, gpas, field_data.semCols);

        float cgpa = get_cgpa_from_courses((Course**)pSQLCourses, number_of_courses);

        free_courses(pSQLCourses, number_of_courses);

        pRow = malloc(sizeof(RowData));
        pRow->studentID = stud_id;
        pRow->studentName = stud_name;
        pRow->gpas = gpas;
        pRow->cgpa = cgpa;

        field_data.rows[i] = *pRow;
        free(pRow);
    }
    sqlite3_finalize(stmt);
    return EXIT_SUCCESS;
}

void wprint_initial_insert_student_menu(WINDOW* win, InsertFieldCoords* insertFieldCoords) {
    int y, x;

    getyx(win, y, x);
    wprintw_center(win, COLS, _("Student Info"));
    mvwchgat(win, y, 2, COLS-4, A_STANDOUT, 0, NULL);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Student ID"));
    getyx(win, insertFieldCoords->studentIdY, insertFieldCoords->studentIdX);
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Student Name"));
    getyx(win, insertFieldCoords->studentNameY, insertFieldCoords->studentNameX);
    getyx(win, y, x);

    wmove(win, y+2, 0);
    wprintw_center(win, COLS, _("Course %d Info"), 1);
    mvwchgat(win, y+2, 2, COLS-4, A_STANDOUT, 1, NULL);
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Course code"));
    getyx(win, insertFieldCoords->courseCodeY, insertFieldCoords->courseCodeX);
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Semester"));
    getyx(win, insertFieldCoords->semY, insertFieldCoords->semX);
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Credit Hours"));
    getyx(win, insertFieldCoords->creditHoursY, insertFieldCoords->creditHoursX);
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, _("%-15s: "), _("Grade"));
    getyx(win, insertFieldCoords->gradeY, insertFieldCoords->gradeX);
}

void wprint_course_insert_field(WINDOW* win, int n) {
    int y, x;

    getyx(win, y, x);
    wprintw_center(win, COLS, _("Course %d Info"), n);
    mvwchgat(win, y, 2, COLS-4, A_STANDOUT, n%N_COLOR + 1, NULL);
    wmove(win, y+1, 2);
    wprintw(win, "%-15s: ", _("Course code"));
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, "%-15s: ", _("Semester"));
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, "%-15s: ", _("Credit Hours"));
    getyx(win, y, x);
    wmove(win, y+1, 2);
    wprintw(win, "%-15s: ", _("Grade"));
}

int yes_or_no_selector(WINDOW* win, int default_option) {
    /* prints directly on the cursor position*/

    int y, x, yes_start_x, yes_end_x, no_start_x, no_end_x;  // Continue?  YES  ||  NO
    getyx(win, y, yes_start_x);
    wprintw(win, _("YES"));
    getyx(win, y, yes_end_x);

    waddstr(win, "  ");

    getyx(win, y, no_start_x);
    wprintw(win, _("NO"));
    getyx(win, y, no_end_x);

    noecho();
    keypad(win, TRUE);
    curs_set(0);

    int input, selection = default_option;
    do {
        mvwchgat(win, y, yes_start_x, no_end_x - yes_start_x, 0, 0, NULL);
        if (selection)
            mvwchgat(win, y, yes_start_x, yes_end_x - yes_start_x, A_STANDOUT, 0, NULL);
        else
            mvwchgat(win, y, no_start_x, no_end_x - no_start_x, A_STANDOUT, 0, NULL);
        wrefresh(win);

        input = wgetch(win);
        switch (input) {
            case KEY_LEFT:
                if (!selection)
                    selection = 1;
                break;
            case KEY_RIGHT:
                if (selection)
                    selection = 0;
                break;
            case KEY_ENTER:
            case '\n':
                wmove(win, y, 0);
                wclrtoeol(win);

                // revert to before
                echo();
                keypad(win, FALSE);
                curs_set(1);
                return selection;
            default:
                break;
        }
    } while (1);
}

/* Helpers */

static inline int sort_row(int sort_mode) {
    // 1 - id, 2 - name, 3+ - gpa, ..., cgpa
    if (sort_mode == 1)
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_id);
    else if (sort_mode == 2)
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_name);
    else if (sort_mode >= 3 && sort_mode < 3 + field_data.semCols) {
        sort_gpa_sem = sort_mode - 3;
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_gpa_desc);
    } else
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_cgpa_desc);
    return EXIT_SUCCESS;
}

// helper functions
static inline int truncate_str(char* string, size_t len) {
    string[len-2] = '.';
    string[len-1] = '.';
    string[len] = '\0';
    return EXIT_SUCCESS;
}

static inline void wprintw_center(WINDOW* win, int width, char* format, ...) {
    va_list arglist;
    va_start(arglist, format);

    int y, x;
    getyx(win, y, x);
    wmove(win, y, x + center_horizontal(strlen(format), width));
    vw_printw(win, format, arglist);
}

static inline void wprintw_header(WINDOW* win, bool standout) {
    int crow, ccol;
    getyx(win, crow, ccol);
    wprintw(win, "%s", field_data.idFieldString);
    wmove(win, crow, ccol + field_data.idFieldLen + field_data.fieldSeperateLen);

    getyx(win, crow, ccol);
    wprintw(win, "%s", field_data.nameFieldString);
    wmove(win, crow, ccol + field_data.nameFieldLen + field_data.fieldSeperateLen);

    for (int i = 1; i <= field_data.semCols; i++) {
        getyx(win, crow, ccol);
        wprintw(win, field_data.gpaFieldString, i);
        wmove(win, crow, ccol + field_data.gpaFieldLen + field_data.fieldSeperateLen);
    }

    getyx(win, crow, ccol);
    wprintw_center(win, field_data.cgpaFieldLen, field_data.cgpaFieldString);
    wmove(win, crow, ccol + field_data.gpaFieldLen + field_data.fieldSeperateLen);
    if (standout)
        wstandout_line(win, crow, 0);
}

static inline void wprintw_footer(WINDOW* win, bool standout) {
    int crow, ccol;
    getmaxyx(win, crow, ccol);

    wprintw(win, "[q] %s\t[Enter] %s\t", _("Quit"), _("View Details"));
    waddch(win, '[');
    waddch(win, ACS_LARROW);
    waddch(win, ACS_RARROW);
    waddch(win, ']');
    wprintw(win, " %s\t", _("Change Sort Mode"));
    waddch(win, '[');
    waddch(win, ACS_UARROW);
    waddch(win, ACS_DARROW);
    waddch(win, ']');
    wprintw(win, " %s\t", _("Scroll"));
    if (standout)
        wstandout_line(win, crow, 0);  // other color pair don't work for some reason, make line invisible until terminal resized.
}

static inline void standout_sorted_header(int sort_mode, int color_pair) {
    int start = 0, end = 0;
    if (sort_mode >= 1)
        end += field_data.idFieldLen;
    if (sort_mode >= 2) {
        end += field_data.fieldSeperateLen;
        start = end;
        end += field_data.nameFieldLen;
    }
    if (sort_mode >= 3) {
        for (int i = 3; i <= sort_mode && i < 3 + field_data.semCols; i++) {
            end += field_data.fieldSeperateLen;
            start = end;
            end += field_data.gpaFieldLen;
        }
    }
    if (sort_mode >= 3 + field_data.semCols) {
        end += field_data.fieldSeperateLen;
        start = end;
        end += field_data.cgpaFieldLen;
    }
    mvwchgat(stdscr, 1, start, end-start, A_STANDOUT, color_pair, NULL);
}

int compare_id(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    return strcmp(r1->studentID, r2->studentID);
}

int compare_gpa_desc(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    float fa = r1->gpas[sort_gpa_sem];
    float fb = r2->gpas[sort_gpa_sem];
    return (fa < fb) - (fa > fb);
}

int compare_cgpa_desc(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    float fa = r1->cgpa;
    float fb = r2->cgpa;
    return (fa < fb) - (fa > fb);
}

int compare_name(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    return strcmp(r1->studentName, r2->studentName);
}

int free_row(RowData* pRow) {
    free(pRow->studentID);
    free(pRow->studentName);
    free(pRow->gpas);
    return EXIT_SUCCESS;
}

int free_rows() {
    for (int i = 0; i < field_data.number_of_rows; i++)
        free_row(&field_data.rows[i]);
    free(field_data.rows);
    return EXIT_SUCCESS;
}
