#include <string.h>
#include <stdlib.h>
#include <calculation.h>


float get_grade_point(char* grade) {
	/* returns grade point or -1.0 if it doesn't match any.*/
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
            total_credit_hours += pCourses[i]->credit_hours;
        }
    }
    return total_quality_points / total_credit_hours;
}

void get_gpas_from_courses(Course** pCourses, int number_of_courses, float* buff, int buff_len) {
    float* total_quality_points = calloc(buff_len, sizeof(float));
    int* total_credit_hours = calloc(buff_len, sizeof(int));
    for (int i = 0; i < number_of_courses; i++) {
        if (pCourses[i] != NULL) {
            total_quality_points[pCourses[i]->sem - 1] += get_grade_point(pCourses[i]->grade) * pCourses[i]->credit_hours;  // grade_point * credit_hours
            total_credit_hours[pCourses[i]->sem - 1] += pCourses[i]->credit_hours;
        }
    }

    for (int sem = 0; sem < buff_len; sem++)
        buff[sem] = total_quality_points[sem] / total_credit_hours[sem];

    free(total_quality_points);
    free(total_credit_hours);
}

float get_cgpa_from_courses(Course** pCourses, int number_of_courses) {
    float total_quality_points = 0;
    int total_credit_hours = 0;
    for (int i = 0; i < number_of_courses; i++) {
        if (pCourses[i] != NULL) {
            total_quality_points += get_grade_point(pCourses[i]->grade) * pCourses[i]->credit_hours;  // grade_point * credit_hours
            total_credit_hours += pCourses[i]->credit_hours;
        }
    }
    return total_quality_points / total_credit_hours;
}

float get_student_gpa(Student* pStudent, int sem) {
    return get_gpa_from_courses(pStudent->pCourses, pStudent->number_of_courses, sem);
}

void get_student_gpas(Student* pStudent, float* buff, int buff_len) {
    return get_gpas_from_courses(pStudent->pCourses, pStudent->number_of_courses, buff, buff_len);
}

float get_student_cgpa(Student* pStudent) {
    return get_cgpa_from_courses(pStudent->pCourses, pStudent->number_of_courses);
}
