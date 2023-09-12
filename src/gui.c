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
WINDOW* insert_student_win;
FieldData field_data;
InsertFieldData insert_field_data;

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
    curs_set(0);
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
        SQLStudent* stud; // used for details and delete
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
            case '\r':
                def_prog_mode();
                endwin();

                clear_screen();
                stud = get_student(db, field_data.rows[selection].studentID);
                printFullStudentDetails(stud);
                free_student(stud);
                getchar();

                reset_prog_mode();
                refresh();
                break;
            case 'd':
                def_prog_mode();
                endwin();

                clear_screen();
                stud = get_student(db, field_data.rows[selection].studentID);
                deleteStudent(stud);
                free_student(stud);

                free_rows();

                max_students = get_number_of_students(db);
                field_data.rows = calloc(max_students, sizeof(RowData));
                field_data.number_of_rows = max_students;
                init_rows(db);

                update = 1;
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
    raw();
    curs_set(1);

    // inputs
    char *student_id = NULL, *student_name = NULL;
    Student student;
    Course** courses = NULL;
    int win_w = COLS - 4, win_h = LINES - 4, current_row = 0;

    // title
    wprintw_center(stdscr, win_w, _("Inserting Student"));
    wstandout_line(stdscr, 0, 0);
    refresh();
    // fields
    insert_student_win = newpad(100, win_w);

    InsertFieldCoords insertFieldCoords;  // store info
    wprint_initial_insert_student_menu(insert_student_win, &insertFieldCoords);
    prefresh(insert_student_win, current_row, 0, 2, 2, 2+win_h, 2+win_w);

    insert_field_data = (InsertFieldData){
            insertFieldCoords,
            0,
            4,
            current_row,
            win_h,
            win_w
    };

    // declaration for inputing
    char buffer[60];
    int buff_ch, n;
    int exit = 0, number_of_courses = 0;
    int y, x;
    keypad(insert_student_win, TRUE);
    do {
        int current_course = (insert_field_data.current_selection - 2) / insert_field_data.number_of_course_fields;  // starts at 0
        mvwprintw(stdscr, 0, 0, "%d", insert_field_data.current_selection);
        refresh();

        if (current_course == number_of_courses) {  // if no course, make new
            Course **tmp = realloc(courses, (number_of_courses + 1) * sizeof(Course *));
            if (tmp == NULL) {
                log_alloc_error("insert_student_name", "courses");
                free(courses);
                clear();
                endwin();
                return EXIT_FAILURE;
            }
            courses = tmp;

            // build course
            Course *course = malloc(sizeof(Course));
            if (course == NULL) {
                log_alloc_error("insert_student_menu", "course");
                return EXIT_FAILURE;
            }
            courses[current_course] = course;

            // init with default, used to validate later
            courses[current_course]->course_code = NULL;
            courses[current_course]->sem = 0;
            courses[current_course]->credit_hours = 0;
            courses[current_course]->grade = NULL;
            number_of_courses++;
        }

        // accept input
        n = 0;
        move_to_inputting(insert_student_win);
        int stop_input = 0;
        do {
            getyx(insert_student_win, y, x);
            prefresh(insert_student_win, insert_field_data.pad_current_row, 0, 2, 2, 2+insert_field_data.pad_height, 2+insert_field_data.pad_width);
            buff_ch = wgetch(insert_student_win);

            switch (buff_ch) {
                case KEY_ENTER:
                case '\n':
                case '\r':
                    wclrtoeol(insert_student_win);
                    stop_input = 1;
                    break;
                case 3: // ctrl-C
                    stop_input = 1;
                    exit = 1;
                    break;
                case KEY_UP:
                    if (insert_field_data.current_selection > 0) {
                        insert_field_data.current_selection--;
                        stop_input = 1;
                    }
                    break;
                case KEY_DOWN:
                    // allow move down if not last course
                    if ((insert_field_data.current_selection - 2 + 1) / insert_field_data.number_of_course_fields != number_of_courses) {  // current_course formula
                        insert_field_data.current_selection++;
                        stop_input = 1;
                    }
                    break;
                case KEY_LEFT:
                    if (n > 0) {
                        wmove(insert_student_win, y, x - 1);
                        n--;
                    }
                    break;
                case KEY_RIGHT:
                    if (n < strlen(buffer)) {
                        wmove(insert_student_win, y, x + 1);
                        n++;
                    }
                    break;
                case KEY_BACKSPACE:
                    if (n > 0) {
                        wmove(insert_student_win, y, x - 1);
                        waddch(insert_student_win, ' ');
                        wmove(insert_student_win, y, x - 1);
                        n--;
                        buffer[n] = '\0';
                    } else {
                        wclrtoeol(insert_student_win);
                    }
                    break;
                default:
                    waddch(insert_student_win, buff_ch);
                    buffer[n] = (char)buff_ch;
                    n++;

                    if (n >= ARRAY_SIZE(buffer) - 1) stop_input = 1;
            }
        } while(!stop_input);  // only breaks when key_up / down / enter key

        if (buff_ch == KEY_UP || buff_ch == KEY_DOWN) {
            auto_scroll();
            continue;
        }

        buffer[n] = '\0';  // end

        if (!validate_input(buffer)) {
            move_to_inputting(insert_student_win);
            wclrtoeol(insert_student_win);
            continue;
        }

        char* dummy;
        // store input
        switch (insert_field_data.current_selection) {
            case 0:  // id
                if (student_id != NULL) {  // free memory if change
                    free(student_id);
                }
                student_id = strdup(buffer);
                break;
            case 1:  // name
                if (student_name != NULL) {  // free memory if change
                    free(student_name);
                }
                student_name = strdup(buffer);
                break;
            default:  // 2+, is courses
                switch((insert_field_data.current_selection - 2) % 4) {
                    case 0:  // course_code
                        if (courses[current_course]->course_code != NULL) {  // free memory if change
                            free(courses[current_course]->course_code);
                        }
                        courses[current_course]->course_code = strdup(buffer);
                        break;
                    case 1:  // sem
                        courses[current_course]->sem = (int)strtol(buffer, &dummy, 10);
                        break;
                    case 2:  // cred hour
                        courses[current_course]->credit_hours = (int)strtol(buffer, &dummy, 10);
                        break;
                    case 3:  // grade
                        if (courses[current_course]->grade != NULL) {  // free memory if change
                            free(courses[current_course]->grade);
                        }
                        courses[current_course]->grade = strdup(buffer);
                        break;
                    default:
                        fprintf(stderr, "save_input: You shouldn't be here");
                }
        }

        insert_field_data.current_selection++;
        if (insert_field_data.current_selection > 2 &&   // not first course, auto printed already
            (insert_field_data.current_selection - 2) % 4 == 0) {  // course info finish
            // if something is not filled, move them to it.
            if (courses[current_course]->course_code == NULL) {
                insert_field_data.current_selection -= 4;
                continue;
            } else if (courses[current_course]->sem == 0) {
                insert_field_data.current_selection -= 3;
                continue;
            } else if (courses[current_course]->credit_hours == 0) {
                insert_field_data.current_selection -= 2;
                continue;
            } else if (courses[current_course]->grade == NULL) {
                insert_field_data.current_selection -= 1;
                continue;
            }

            if (current_course == number_of_courses - 1) {  // last course
                // prompt if want to add a new course
                wmove(insert_student_win, (insertFieldCoords.courseCodeY - 1) + (insert_field_data.number_of_course_fields + 1 + 1) * (current_course + 1), 0);
                wprintw_center(insert_student_win, COLS - 12, _("Do you want to add another course? "));
                auto_scroll();
                if (yes_or_no_selector(insert_student_win, 1)) {
                    wprint_course_insert_field(insert_student_win, number_of_courses + 1);
                    auto_scroll();
                }
                else
                    exit = 1;
            }
        }
    } while (!exit);

    // ask to save or not
    wmove(insert_student_win, (insertFieldCoords.courseCodeY - 1) + (insert_field_data.number_of_course_fields + 1 + 1) * (number_of_courses ? number_of_courses : 1), 0);
    wprintw_center(insert_student_win, COLS - 8, _("Save record? "));
    if (yes_or_no_selector(insert_student_win, 1)) {  // yes
        student = (Student) {
                student_id,
                student_name,
                courses,
                number_of_courses
        };
        store_student(db, &student);
        store_student_courses(db, &student);
    }  // no, do nothing

    if (courses != NULL) {
        for (int i = 0; i < number_of_courses; i++) {
            if (courses[i] != NULL) {
                free(courses[i]->course_code);
                free(courses[i]->grade);
                free(courses[i]);
            }
        }
        free(courses);
    }
    free(student_name);
    free(student_id);

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
        if (pRow == NULL) {
            log_alloc_error("init_rows", "pRow");
            return EXIT_FAILURE;
        }
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
    mvwchgat(win, y, 0, -1, A_STANDOUT, 0, NULL);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Student ID"));
    getyx(win, insertFieldCoords->studentIdY, insertFieldCoords->studentIdX);
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Student Name"));
    getyx(win, insertFieldCoords->studentNameY, insertFieldCoords->studentNameX);
    getyx(win, y, x);

    wmove(win, y+2, 0);
    wprintw_center(win, COLS, _("Course %d Info"), 1);
    mvwchgat(win, y+2, 0, COLS-4, A_STANDOUT, 1, NULL);
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Course code"));
    getyx(win, insertFieldCoords->courseCodeY, insertFieldCoords->courseCodeX);
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Semester"));
    getyx(win, insertFieldCoords->semY, insertFieldCoords->semX);
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Credit Hours"));
    getyx(win, insertFieldCoords->creditHoursY, insertFieldCoords->creditHoursX);
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, _("%-15s: "), _("Grade"));
    getyx(win, insertFieldCoords->gradeY, insertFieldCoords->gradeX);
}

void wprint_course_insert_field(WINDOW* win, int n) {
    int y, x;

    getyx(win, y, x);
    wprintw_center(win, COLS, _("Course %d Info"), n);
    mvwchgat(win, y, 0, COLS-4, A_STANDOUT, n%N_COLOR + 1, NULL);
    wmove(win, y+1, 0);
    wprintw(win, "%-15s: ", _("Course code"));
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, "%-15s: ", _("Semester"));
    getyx(win, y, x);
    wmove(win, y+1, 0);
    wprintw(win, "%-15s: ", _("Credit Hours"));
    getyx(win, y, x);
    wmove(win, y+1, 0);
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

    curs_set(0);
    int input, selection = default_option;
    do {
        mvwchgat(win, y, yes_start_x, no_end_x - yes_start_x, 0, 0, NULL);
        if (selection)
            mvwchgat(win, y, yes_start_x, yes_end_x - yes_start_x, A_STANDOUT, 0, NULL);
        else
            mvwchgat(win, y, no_start_x, no_end_x - no_start_x, A_STANDOUT, 0, NULL);
        prefresh(insert_student_win, insert_field_data.pad_current_row, 0, 2, 2, 2+insert_field_data.pad_height, 2+insert_field_data.pad_width);

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
            case '\r':
                wmove(win, y, 0);
                wclrtoeol(win);

                // revert to before
                curs_set(1);
                return selection;
            default:
                break;
        }
    } while (1);
}

void move_to_inputting(WINDOW* win) {
    int y, x;
    int current_course = (insert_field_data.current_selection - 2) / insert_field_data.number_of_course_fields;  // starts at 0

    switch (insert_field_data.current_selection) {
        case 0:  // id
            y = insert_field_data.insertFieldCoords.studentIdY;
            x = insert_field_data.insertFieldCoords.studentIdX;
            break;
        case 1:  // name
            y = insert_field_data.insertFieldCoords.studentNameY;
            x = insert_field_data.insertFieldCoords.studentNameX;
            break;
        default:  // 2+, is courses
            switch((insert_field_data.current_selection - 2) % 4) {
                case 0:  // course_code
                    y = insert_field_data.insertFieldCoords.courseCodeY + (insert_field_data.number_of_course_fields + 2) * current_course;
                    x = insert_field_data.insertFieldCoords.courseCodeX;
                    break;
                case 1:  // sem
                    y = insert_field_data.insertFieldCoords.semY + (insert_field_data.number_of_course_fields + 2) * current_course;
                    x = insert_field_data.insertFieldCoords.semX;
                    break;
                case 2:  // cred hour
                    y = insert_field_data.insertFieldCoords.creditHoursY + (insert_field_data.number_of_course_fields + 2) * current_course;
                    x = insert_field_data.insertFieldCoords.creditHoursX;
                    break;
                case 3:  // grade
                    y = insert_field_data.insertFieldCoords.gradeY + (insert_field_data.number_of_course_fields + 2) * current_course;
                    x = insert_field_data.insertFieldCoords.gradeX;
                    break;
                default:
                    fprintf(stderr, "move_to_inputting: You shouldn't be here");
            }
    }
    wmove(win, y, x);
}

int validate_input(char * input) {
    long integer_input;
    char *buff;

    switch (insert_field_data.current_selection) {
        case 0:  // id
        case 1:  // name
            if (input[0] == '\0') return 0;
            else return 1;
        default:  // 2+, is courses
            switch((insert_field_data.current_selection - 2) % 4) {
                case 0:  // course_code
                    if (input[0] == '\0') return 0;
                    else return 1;
                case 1:  // sem
                case 2:  // cred hour
                    integer_input = strtol(input, &buff, 10);
                    if (integer_input <= 0) return 0;
                    return 1;
                case 3:  // grade
                    if (!is_valid_grade(input)) return 0;
                    return 1;
                default:
                    fprintf(stderr, "validate_input: You shouldn't be here");
                    return 0;
            }
    }
}

void auto_scroll() {
    int y, x;
    getyx(insert_student_win, y, x);
    if (y > insert_field_data.pad_current_row + insert_field_data.pad_height)
        insert_field_data.pad_current_row = y - insert_field_data.pad_height;
    else if (y <= insert_field_data.pad_current_row) {
        insert_field_data.pad_current_row = y - 1;
    }
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

    wprintw(win, "[q] %s\t", _("Quit"));
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
    wprintw(win, "[Enter] %s\t[d] %s\t", _("View Details"), _("Delete"));
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
