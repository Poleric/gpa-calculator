#include <stdio.h>	
#include <stdlib.h>
#include <time.h>
#include <string.h>
#pragma warning(disable:4996)

char* get_day(int day);

void main() {

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char input[30];
	int tries = 3;

	printf("          GPA/CGPA CALCULATOR          \n");
	printf("=======================================\n");
	printf("ADMINISTRATOR\n");
	printf("SCHOOL: COLEJ PASAR\n");
	printf("DATE: %s,%d-%02d-%02d\n", get_day(tm.tm_wday), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	printf("ENTER THE PASSWORD: ");
	gets(input);

	if (strcmp(input, "123456789abc") == 0) {
		system("cls");
		printf("Welcome to the admin Screen\n");
	}
	else {
		system("cls");
		printf("PASSWORD INCORRECT\n");
		printf("PLEASE ENTER THE PASSWORD AGAIN(3): ");
		gets(input);
	}
	if (strcmp(input, "123456789abc") == 0) {
		system("cls");
		printf("Welcome to the admin Screen\n");
	}
	else {
		system("cls");
		printf("PASSWORD INCORRECT\n");
		printf("PLEASE ENTER THE PASSWORD AGAIN(2): ");
		gets(input);
	}
	if (strcmp(input, "123456789abc") == 0) {
		system("cls");
		printf("Welcome to the admin Screen\n");
	}
	else {
		system("cls");
		printf("PASSWORD INCORRECT\n");
		printf("PLEASE ENTER THE PASSWORD AGAIN(1): ");
		gets(input);
	}
	if (strcmp(input, "123456789abc") == 0) {
		system("cls");
		printf("Welcome to the admin Screen\n");
	}
	else {
		system("cls");
		printf("!!!YOU HAVE ENTER THE INCORRECT ANSWER TOO MANY TIMES, PLEASE TRY AGAIN LATER!!!\n");
		printf("================================================================================\n");
		printf("If you have forgotten the password, please contact to look for help:012-879-3965\n");
		printf("OR send us a email:ColejPasarService@gmail.com\n");
		printf("--------------------------------------------------------------------------------\n");
		printf("\n");
	}
}

char* get_day(int day) {
    
	switch (day) {
	case 0:
		return "MONDAY";
	case 1:
		return "TUESDAY";
	case 2:
		return "WEDNESDAY";
	case 3:
		return "THURSDAY";
	case 4:
		return "FRIDAY";
	case 5:
		return "SATURDAY";
	case 6:
		return "SUNDAY";
	}
}

