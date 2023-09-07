#pragma once

#include <db.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#include <curses.h>
#else
#include <ncurses.h>
#endif

#ifndef GPA_CALCULATOR_GUI_H
#define GPA_CALCULATOR_GUI_H

typedef struct RowData{
    char *studentID, *studentName;
    float *gpas;
    float cgpa;
} RowData;

typedef struct FieldData {
    int fieldSeperateLen, idFieldLen, gpaFieldLen, cgpaFieldLen, nameFieldLen;
    char *idFieldString, *nameFieldString, *gpaFieldString, *cgpaFieldString;
    int semCols, width, height;
    RowData* rows;
    int number_of_rows;
} FieldData;

int student_list_menu(sqlite3* db);
int update_student_list_window(int current_row);
int write_student_list_window(int current_row);
int init_field_data(int max_width, int max_height, int max_sem);
int init_rows(sqlite3* db);
static inline int truncate_str(char* string, size_t len);
static inline void wprintw_center(WINDOW* win, int width, char* format, ...);
static inline void wprintw_header(WINDOW* win, bool standout);
static inline void wprintw_footer(WINDOW* win, bool standout);
static inline void wstandout_line(WINDOW* win, int row, int color_pair);
void standout_sorted_header(int sort_mode, int color_pair);
int free_row(RowData* pRow);
int free_rows();
#define center_horizontal(length, field_width) ((int)(field_width - length)/2)

int sort_row(int sort_mode);
int compare_id(const void * a, const void * b);
int compare_id_desc(const void * a, const void * b);
int compare_gpa(const void * a, const void * b);
int compare_gpa_desc(const void * a, const void * b);
int compare_cgpa(const void * a, const void * b);
int compare_cgpa_desc(const void * a, const void * b);
int compare_name(const void * a, const void * b);
int compare_name_desc(const void * a, const void * b);

#endif //GPA_CALCULATOR_GUI_H
