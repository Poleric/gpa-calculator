#include <string.h>
#include <calculation.h>


float get_grade_point(char* grade) {
	/* returns grade point or -1.0 if it doesnt match any.*/
	if (strcmp(grade, "A") == 0)
		return 4.0f;
	else if (strcmp(grade, "A-") == 0)
		return 3.75f;
	else if (strcmp(grade, "B+") == 0)
		return 3.5f;
	else if (strcmp(grade, "B") == 0)
		return 3.0f;
	else if (strcmp(grade, "B-") == 0)
		return 2.75f;
	else if (strcmp(grade, "C+") == 0)
		return 2.5f;
	else if (strcmp(grade, "C") == 0)
		return 2.0f;
	else if (strcmp(grade, "F") == 0)
		return 0.0f;
	else
		return -1.0f;
}

float get_gpa_from_courses(Course** pCourses, int number_of_courses, int sem) {
    float total_quality_points = 0;
    int total_credit_hours = 0;
    for (int i = 0; i < number_of_courses; i++) {
        if (pCourses[i] != NULL && pCourses[i]->sem == sem) {
            total_quality_points += get_grade_point(pCourses[i]->grade) * pCourses[i]->credit_hours;  // grade_point * credit_hours
            total_credit_hours += (int) pCourses[i]->credit_hours;
        }
    }
    return total_quality_points / total_credit_hours;
}

float get_cgpa_from_courses(Course** pCourses, int number_of_courses) {
    float total_quality_points = 0;
    int total_credit_hours = 0;
    for (int i = 0; i < number_of_courses; i++) {
        if (pCourses[i] != NULL) {
            total_quality_points += get_grade_point(pCourses[i]->grade) * pCourses[i]->credit_hours;  // grade_point * credit_hours
            total_credit_hours += (int) pCourses[i]->credit_hours;
        }
    }
    return total_quality_points / total_credit_hours;
}
