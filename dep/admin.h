#pragma once
#include <db.h>

#ifndef ADMIN_H
#define ADMIN_H

void admin();
int admin_login();
void student();

void printStudentDetails(SQLStudent* student);

#endif