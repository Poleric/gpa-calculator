#include <ncurses.h>
#include <stdlib.h>
#include <db.h>
#include <sqlite3.h>
#include <gui.h>
#include <string.h>
#include <calculation.h>
//#include <menu.h>

#define MINIMUM_NAME_FIELD_LEN 10
#define MAX_SEM 3
#define DEBUG 1

//int display_students_list(sqlite3* db) {
//    sqlite3_stmt* stmt = get_students_stmt(db, NULL);
//    printf("Student ID  Name\n");
//    printf("----------  ------------------------------\n");
//    for (
//        int ret = sqlite3_step(stmt);  // 1. run the sql, and assign the return value
//        ret == SQLITE_ROW;  // 2. break if no more sqlite results
//        ret = sqlite3_step(stmt)  // 3. go next row and continue at 2.
//    )
//    {
//        printf("%-10s  %-30s\n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1));
//    }
//    sqlite3_finalize(stmt);
//
//    return EXIT_SUCCESS;
//}




int admin_menu() {
    sqlite3* db;
    FieldData field_data;
    WINDOW* stud_list_win;

    sqlite3_open("./students.db", &db);
    init_student_db(db);

    // ncurses
    initscr();      // start
    cbreak();       // get input for each character
    noecho();       // no display input

    // title
    print_center(COLS, "Kolej Pasar Students");

    /* GUI idea
     * Student ID....<---Student Name--->....GPA (S1)....<-CGPA->
     *
     */
    init_field_data(&field_data, COLS-2);
    if (field_data.NAME_FIELD_LEN < MINIMUM_NAME_FIELD_LEN) {
        endwin();
        fprintf(stderr, "Screen too small. Requires at least %d character wide", field_data.ID_FIELD_LEN + MINIMUM_NAME_FIELD_LEN + field_data.GPA_FIELD_LEN + field_data.CGPA_FIELD_LEN + 3*field_data.FIELD_SEPERATE_LEN);  // TODO: Adjust the screen display to accomadate small screen sizes.
        exit(1);
    }

    // write headers
    print_header(&field_data, TRUE);
    refresh();


    int max_students = get_number_of_students(db);
    int pad_row = 0, selection = 1;
    int input;
    int win_h = LINES - 3, win_w = COLS;
    int crow, ccol;

    stud_list_win = newpad(max_students, COLS);
    keypad(stud_list_win, TRUE);
    write_student_list_window(stud_list_win, db, &field_data);
    do {
        getyx(stud_list_win, crow, ccol);
        prefresh(stud_list_win, pad_row, 0, 2, 0, win_h, win_w);
        input = wgetch(stud_list_win);
        switch(input) {
            case KEY_UP:
                if (pad_row > 0 && selection <= max_students - win_h + 2) {
                    pad_row--;
                }
                if (selection > 1) {
                    selection--;
                    wmove(stud_list_win, crow-1, 0);
                }
                break;
            case KEY_DOWN:
                if (pad_row <= max_students - win_h)
                    pad_row++;
                if (selection < max_students) {
                    wmove(stud_list_win, crow+1, 0);
                    selection++;
                }
                break;
            default:
                break;
        }

        if (DEBUG) {
            mvprintw(0, 0, "%d", pad_row);
            mvprintw(0, 4, "%d", max_students - win_h);
            mvprintw(0, 8, "%d", selection);
            refresh();
        }
    } while (input != 'q');

    wclear(stud_list_win); clear();
    delwin(stud_list_win);
    endwin();
    sqlite3_close(db);
    return EXIT_SUCCESS;
}

int write_student_list_window(WINDOW* win, sqlite3* db, FieldData* field_data) {
    // write list
    sqlite3_stmt* stmt = get_all_students_stmt(db);

    if (can_change_color()) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);  // selected color
        start_color();
    } else
        mvwprintw(win, 10, 10, "Cant start Color");

    int crow, ccol;  // current_row, current_column
    for (
        int ret = sqlite3_step(stmt);
        ret == SQLITE_ROW;
        ret = sqlite3_step(stmt), wmove(win, crow+1, 0)
    )
    {
        getyx(win, crow, ccol);

        // print student_id
        const unsigned char* stud_id = sqlite3_column_text(stmt, 0);
        wprintw(win, "%s", (const char*)stud_id);

        // print student_name
        int name_len = sqlite3_column_bytes(stmt, 1);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        if (name_len > field_data->NAME_FIELD_LEN) {
            char trunc_name[field_data->NAME_FIELD_LEN + 1];
            for (int j = 0; j < field_data->NAME_FIELD_LEN - 3; j++) {
                trunc_name[j] = name[j];
            }
            truncate_str(trunc_name, field_data->NAME_FIELD_LEN);

            mvwprintw(win, crow, field_data->NAME_FIELD_OFFSET, "%s", trunc_name);
        } else {
            mvwprintw(win, crow, field_data->NAME_FIELD_OFFSET, "%s", name);
        }

        // print gpa & cgpa
        int number_of_courses = get_number_of_courses(db, (char *)stud_id);
        SQLCourse** pSQLCourses = malloc( number_of_courses * sizeof(SQLCourse*));
        get_student_courses(db, (char*) stud_id, pSQLCourses, number_of_courses);

        mvwprintw(win, crow, field_data->GPA_FIELD_OFFSET + center_horizontal(4, field_data->GPA_FIELD_LEN), "%.2f",get_gpa_from_courses((Course**)pSQLCourses, number_of_courses, 1));
        mvwprintw(win, crow, field_data->CGPA_FIELD_OFFSET + center_horizontal(4, field_data->CGPA_FIELD_LEN), "%.2f", get_cgpa_from_courses((Course**)pSQLCourses, number_of_courses));

        free_courses(pSQLCourses, number_of_courses);

//        if (i+1 == current_row)
//            mvwchgat(win, crow, 0, -1, A_BLINK, 1, NULL);  // TODO: Text is not visible.
    }
    sqlite3_finalize(stmt);
    wmove(win, 0, 0);
    return EXIT_SUCCESS;
}

int init_field_data(FieldData* field_data, int max_width) {
    field_data->FIELD_SEPERATE_LEN = 2;
    field_data->ID_FIELD_LEN = 10;
    field_data->GPA_FIELD_LEN = 8;
    field_data->CGPA_FIELD_LEN = 8;
    field_data->NAME_FIELD_LEN = max_width - field_data->ID_FIELD_LEN - field_data->GPA_FIELD_LEN - field_data->CGPA_FIELD_LEN - 3*field_data->FIELD_SEPERATE_LEN;
    field_data->ID_FIELD_STRING = "Student ID";
    field_data->NAME_FIELD_STRING = "Student Name";
    field_data->GPA_FIELD_STRING = "GPA (S%d)";
    field_data->CGPA_FIELD_STRING = "CGPA";
    field_data->ID_FIELD_OFFSET = 0;
    field_data->NAME_FIELD_OFFSET = field_data->ID_FIELD_LEN + field_data->FIELD_SEPERATE_LEN;
    field_data->GPA_FIELD_OFFSET = field_data->NAME_FIELD_OFFSET + field_data->NAME_FIELD_LEN + field_data->FIELD_SEPERATE_LEN;
    field_data->CGPA_FIELD_OFFSET = field_data->GPA_FIELD_OFFSET + field_data->GPA_FIELD_LEN + field_data->FIELD_SEPERATE_LEN;

    return EXIT_SUCCESS;
}

//int display_student_info(sqlite3* db) {
//
//}
//
//int input_student_details(sqlite3* db) {
//
//}
//
//int update_student_details(sqlite3* db) {
//
//}

// helper functions
static inline int truncate_str(char* string, size_t len) {
    string[len-2] = '.';
    string[len-1] = '.';
    string[len] = '\0';
    return EXIT_SUCCESS;
}

static inline void wprint_center(WINDOW* win, int width, char* string) {
    int y, x;
    getyx(win, y, x);
    mvwprintw(win, y, x + center_horizontal(strlen(string), width), string);
}

static inline void print_header(FieldData* field_data, bool standout) {
    mvprintw(1, field_data->ID_FIELD_OFFSET, "%s", field_data->ID_FIELD_STRING);
    mvprintw(1, field_data->NAME_FIELD_OFFSET, "%s", field_data->NAME_FIELD_STRING);
    mvprintw(1, field_data->GPA_FIELD_OFFSET, field_data->GPA_FIELD_STRING, 1);
    move(1, field_data->CGPA_FIELD_OFFSET);
    print_center(field_data->CGPA_FIELD_LEN, field_data->CGPA_FIELD_STRING);
    if (standout)
        mvchgat(1, 0, -1, A_STANDOUT, 0, NULL);  // highlight header
}

int main() {
    admin_menu();
}
