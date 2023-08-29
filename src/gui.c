#include <stdio.h>
#include <stdlib.h>
#include <db.h>
#include <sqlite3.h>
#include <gui.h>

int display_students_list(sqlite3* db) {
    sqlite3_stmt* stmt = get_students_stmt(db, NULL);
    printf("Student ID  Name\n");
    printf("----------  ------------------------------\n");
    for (
        int ret = sqlite3_step(stmt);  // 1. run the sql, and assign the return value
        ret == SQLITE_ROW;  // 2. break if no more sqlite results
        ret = sqlite3_step(stmt)  // 3. go next row and continue at 2.
    )
    {
        printf("%-10s  %-30s\n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    return EXIT_SUCCESS;
}

int display_student_info(sqlite3* db) {

}

int input_student_details(sqlite3* db) {

}

int update_student_details(sqlite3* db) {

}

// helper functions
static inline void clear_screen() {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    system("cls");
    #else
    system("clear"); // assume POSIX
    #endif
}

static inline int truncate_str(char* string, size_t len) {
    string[len] = '\0';
    return EXIT_SUCCESS;
}
