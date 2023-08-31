#pragma once

#ifndef GPA_CALCULATOR_GUI_H
#define GPA_CALCULATOR_GUI_H

typedef struct FieldData {
    int FIELD_SEPERATE_LEN, ID_FIELD_LEN, GPA_FIELD_LEN, CGPA_FIELD_LEN, NAME_FIELD_LEN;
    char* ID_FIELD_STRING;
    char* NAME_FIELD_STRING;
    char* GPA_FIELD_STRING;
    char* CGPA_FIELD_STRING;
    int ID_FIELD_OFFSET, NAME_FIELD_OFFSET, GPA_FIELD_OFFSET, CGPA_FIELD_OFFSET;
} FieldData;

typedef struct StudentListRow {
    char* student_id;
    char* student_name;
    float* gpa;
    float cgpa;
} StudentListRow;

int admin_menu();
int write_student_list_window(WINDOW* win, sqlite3* db, FieldData* field_data);
int init_field_data(FieldData* field_data, int max_width);
static inline int truncate_str(char* string, size_t len);
static inline void wprint_center(WINDOW* win, int width, char* string);
static inline void print_header(FieldData* field_data, bool standout);
#define print_center(width, string) wprint_center(stdscr, width, string)
#define center_horizontal(length, field_width) ((int)(field_width - length)/2)


#endif //GPA_CALCULATOR_GUI_H
