#pragma once
#include <db.h>
#include <stdbool.h>

#ifndef ADMIN_H
#define ADMIN_H

void admin();
int adminLogin();
void student();

void printStudentDetails(SQLStudent* student);
void printFullStudentDetails(SQLStudent* student);

void printManyChar(char character, int length);
void printLineWithManyChar(char character, int length);
void printLineWithManyCharWithSeperators(char character, int length, char seperator, int number_of_times, bool trailing_seperator);
void printStudentCoursesTable(SQLStudent* student, int max_sem);

#endif