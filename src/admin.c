#include <stdio.h>	
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <utils.h>

#define PASSWORD "123456789abc"
#define NUMBER_OF_ATTEMPTS 3
#define AVAILABLE_OPTIONS {'V', 'S', 'U', 'Q'}

#define MAX_LEN_INPUT 32
#define VALID_GRADES { "A", "A-", "B+", "B", "B-", "C+", "C", "F"}
#define LEN(x) sizeof(x)/sizeof(*x)

char* get_day(unsigned int day);
int authenticate_user();
void print_home_screen();
void print_admin_screen();
void print_wrong_password_screen(unsigned int attempts_left);
void print_cannot_login_screen();
char ask_for_option();

int start_admin_screen() {
	/* 
	Returns:
	0   - OK
	-1  - Error / End early.
	*/

	clear_screen();
	print_home_screen();

	if (authenticate_user() == -1) {  // wrong password
		return -1;  // return early
	}

	// only way to reach here is getting right password
	clear_screen();
	print_admin_screen(); // welcome to admin blah blah (then ask what they want to do)

	char option = ask_for_option(); 
	clear_screen();
	switch (option) {
		case 'V':  // View record
			break; 
		case 'S':  // Store student
			break;
		case 'U':  // Update student
			break;
		case 'Q':  // Exit application
			printf("Have a great day!\n");
			exit(1);
		default:  // error
			fprintf(stderr, "You shouldn't be here...");
	}
	return 1;
}

char* get_day(unsigned int day) {
	/* Convert day in integer (0 - 6) to day name (MONDAY - SUNDAY)*/
    
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
	default:
		return -1;
	}
}

int authenticate_user() {
	char input[MAX_LEN_INPUT];
	int attempts_left = NUMBER_OF_ATTEMPTS;

	printf("PLEASE ENTER THE PASSWORD: ");  // this is outside coz we use different string for asking password AGAIN
	while (1) {  // infinite loop
		// asks for password
		gets_s(input, MAX_LEN_INPUT);

		if (strcmp(input, PASSWORD) == 0) {
			break;
		}
		else if (attempts_left > 0) {
			clear_screen();
			print_wrong_password_screen(attempts_left);
		}
		else {  // no attempts left
			clear_screen();
			print_cannot_login_screen();
			return -1;  // exit program
		}

		attempts_left--;
	}
	return 0;
}

void print_home_screen() {
	printf("          GPA/CGPA CALCULATOR\n");
	printf("=======================================\n");
	printf("         ADMINISTRATOR CONSOLE\n");
	printf("SCHOOL: COLEJ PASAR\n");

	struct tm now_tm;
	time_t now = time(NULL);  // returns unix time (in seconds)
	localtime_s(&now_tm, &now);  // process the seconds and put into now_tm
	printf("DATE: %s, %d-%02d-%02d\n", get_day(now_tm.tm_wday), now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
}

void print_admin_screen() {
	printf("Welcome to the Administrator Interface.\n");
	printf("=======================================\n");
}

void print_wrong_password_screen(unsigned int attempts_left) {
	printf("PASSWORD INCORRECT\n");
	printf("PLEASE ENTER THE PASSWORD AGAIN(%d): ", attempts_left);
}

void print_cannot_login_screen() {
	printf("!!!YOU HAVE ENTER THE INCORRECT ANSWER TOO MANY TIMES, PLEASE TRY AGAIN LATER!!!\n");
	printf("================================================================================\n");
	printf("If you have forgotten the password, please contact for help:\n");
	printf(" - Phone: 012-879-3965\n");
	printf(" - Email: ColejPasarService@gmail.com\n");
	printf("--------------------------------------------------------------------------------\n");
}

char ask_for_option() {
	/* Ask for user input and returns their input as an uppercase character
	Returns: 'V' | 'S' | 'U' | 'Q'
	*/
	char input;

	printf("Would you like to (V)iew, (S)tore, (U)pdate from the current records, or (Q)uit?\n");

	rewind(stdin);
	input = toupper(getchar());

	char options[] = AVAILABLE_OPTIONS;
	if (is_char_in(input, options, LEN(options)) == -1) { // if invalid option
		clear_screen();
		printf("Invalid option!\n");
		printf("\n");
		return ask_for_option();
	}

	return input;
}

