#pragma once

#ifndef GPA_CALCULATOR_GUI_H
#define GPA_CALCULATOR_GUI_H

static inline void clear_screen();
static inline int truncate_str(char* string, size_t len);

int display_students_list(sqlite3* db);

#endif //GPA_CALCULATOR_GUI_H
