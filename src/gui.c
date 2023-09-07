#include <stdlib.h>
#include <db.h>
#include <string.h>
#include <calculation.h>
#include <utils.h>
#include <stdarg.h>
#include <admin.h>
#include <gui.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define strdup _strdup  //  warning C4996: 'strdup': The POSIX name for this item is deprecated.
#define _(String) String
#else
#include <libintl.h>
#define _(String) gettext(String)
#endif

#define FIELD_SEPERATE_LEN 2

#define ID_FIELD_LEN 10
#define MINIMUM_NAME_FIELD_LEN 12
#define GPA_FIELD_LEN 8
#define CGPA_FIELD_LEN 8

#define ID_FIELD_STRING _("Student ID")
#define NAME_FIELD_STRING _("Student Name")
#define GPA_FIELD_STRING _("GPA (S%d)")
#define CGPA_FIELD_STRING _("CGPA")

// used by gui functions.
WINDOW* student_list_win;
FieldData field_data;
int sort_gpa;  // only used in compare_gpa

int student_list_menu(sqlite3* db) {
    // ncurses
    initscr();      // start
    if(has_colors() == FALSE) {
        endwin();
        sqlite3_close(db);
        printf("Your terminal does not support color\n");
        return EXIT_FAILURE;
    }
    start_color();
    cbreak();       // get input for each character
    noecho();       // no display input

    int max_sem = get_max_sem(db);
    init_field_data(COLS, LINES-3, max_sem);

    if (field_data.width > COLS) {
        endwin();
        fprintf(stderr, "Screen is too small, resize to be atleast %d wide.\n", field_data.width);
        return EXIT_FAILURE;
    }


    wprintw_center(stdscr, COLS, "Kolej Pasar Students");
    move(1, 0);        // print table header below title
    wprintw_header(stdscr, TRUE);
    move(LINES-1, 2);  // print footer at the bottom, move right 2
    wprintw_footer(stdscr, TRUE);
    refresh();

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    int max_students = get_number_of_students(db);
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
                printStudentDetails(stud);
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

int update_student_list_window(int current_row) {
    wclrtobot(student_list_win);
    write_student_list_window(current_row);
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

int init_field_data(int max_width, int max_height, int max_sem) {
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
    return EXIT_SUCCESS;
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
            fprintf(stderr, "get_rows: Allocation error when allocating gpas. Out of memory.");
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

int sort_row(int sort_mode) {
    // 1 - id, 2 - name, 3+ - gpa, ..., cgpa
    if (sort_mode == 1)
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_id);
    else if (sort_mode == 2)
        qsort(field_data.rows, field_data.number_of_rows, sizeof(RowData), compare_name);
    else if (sort_mode >= 3 && sort_mode < 3 + field_data.semCols) {
        sort_gpa = sort_mode - 3;
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

    wprintw(win, "[q] %s\t[Enter] %s", _("Quit"), _("View Details"));
    if (standout)
        wstandout_line(win, crow, 0);  // other color pair dont work for some reason
}

static inline void wstandout_line(WINDOW* win, int row, int color_pair) {
    mvwchgat(win, row, 0, -1, A_STANDOUT, color_pair, NULL);  // highlight header
}

void standout_sorted_header(int sort_mode, int color_pair) {
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

int compare_id_desc(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    return strcmp(r2->studentID, r1->studentID);
}

int compare_gpa(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    float fa = r1->gpas[sort_gpa];
    float fb = r2->gpas[sort_gpa];
    return (fa > fb) - (fa < fb);
}

int compare_gpa_desc(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    float fa = r1->gpas[sort_gpa];
    float fb = r2->gpas[sort_gpa];
    return (fa < fb) - (fa > fb);
}

int compare_cgpa(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    float fa = r1->cgpa;
    float fb = r2->cgpa;
    return (fa > fb) - (fa < fb);
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

int compare_name_desc(const void * a, const void * b) {
    RowData *r1 = (RowData*)a;
    RowData *r2 = (RowData*)b;

    return strcmp(r2->studentName, r1->studentName);
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
