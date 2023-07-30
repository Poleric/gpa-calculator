#include <string.h>
#include <stdlib.h>
#include <calculation.h>
#include <student.h>


float get_grade_point(char* grade) {
	/* returns grade point or -1.0 if it doesnt match any.*/
	if (strcmp(grade, "A") == 0)
		return 4.0;
	else if (strcmp(grade, "A-") == 0)
		return 3.75;
	else if (strcmp(grade, "B+") == 0)
		return 3.5;
	else if (strcmp(grade, "B") == 0)
		return 3.0;
	else if (strcmp(grade, "B-") == 0)
		return 2.75;
	else if (strcmp(grade, "C+") == 0)
		return 2.5;
	else if (strcmp(grade, "C") == 0)
		return 2.0;
	else if (strcmp(grade, "F") == 0)
		return 0.0;
	else
		return -1.0;
}

float get_student_gpa(Student* pStudent, int sem) {
	/* Calculates gpa of a student of a semester */
	float total_quality_points = 0;
	int total_credit_hours = 0;

	for (int i = 0; i < pStudent->number_of_courses; i++) {
		if (pStudent->pCourses[i]->sem == sem) { // for gpa purposes
			total_quality_points += get_grade_point(pStudent->pCourses[i]->grade) * pStudent->pCourses[i]->credit_hours;  // grade_point * credit_hours
			total_credit_hours += pStudent->pCourses[i]->credit_hours;
		}
	}

	return total_quality_points / total_credit_hours;
}

float get_student_cgpa(Student* pStudent) {
	/* Calculates cgpa of a student */
	float total_quality_points = 0;
	int total_credit_hours = 0;

	for (int i = 0; i < pStudent->number_of_courses; i++) {
		total_quality_points += get_grade_point(pStudent->pCourses[i]->grade) * pStudent->pCourses[i]->credit_hours;  // grade_point * credit_hours
		total_credit_hours += pStudent->pCourses[i]->credit_hours;
	}

	return total_quality_points / total_credit_hours;
}
