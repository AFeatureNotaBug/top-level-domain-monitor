/*
 * Jake Haakanson - 2407682h, SP Exercise 1a
 * This is my own work as defined in the Academic Ethics Agreenment I have signed0
 */

#include <stdlib.h>
#include <string.h>
#include "date.h"


#define COMP(x, y) (x > y) ? 1 : (x < y) ? -1 : 0   //Compares two ints and returns <0 if x < y, >0 if x > y, and 0 if x == y


struct date {
     /**********************************************************************************\
     * Since dates are only used for comparison we can use an integer representation:	*
     *   "Date value" dValue = (year * 10000) + (month * 100) + day					*
     *																					*
     * Incorrectly assumes there are 31 days in each month but works for comparisons	*
     \**********************************************************************************/
    int dValue;
};


/*Create a new date and return it, returns NULL upon failure*/
Date *date_create(char *datestr) {
    if (datestr[2] == '/' && datestr[5] == '/') {   //Check format is correct, slash expected at indexes 2 and 5 of datestr - dd/mm/yyyy
        Date *d;
        
        if ((d = (struct date *)malloc(sizeof(struct date))) != NULL) {
            int day, month, year;
            
            if (!((day   = atoi(strtok(datestr, "/"))) > 0) ||  //Get value of day, month, year tokens and check they are ints
                !((month = atoi(strtok(NULL, "/")))    > 0) ||
                !((year  = atoi(strtok(NULL, "/")))    > 0)) {
                
                free(d);		//If the values are not integers, free memory allocated to d
                return NULL;	//and return NULL (Date is not a correct date)
            }
            d->dValue = (year * 10000) + (month * 100) + day;  //Calculate dValue for date comparisons
        }
        return d;
        
    } else {
        return NULL;    //Date format incorrect
		
    }
}


/*Duplicates date d and returns the new date*/
Date *date_duplicate(Date *d) {
	if (d != NULL) {	//Check that d is a date and not NULL
		Date *newDate;
		
		if ((newDate = (struct date *)malloc(sizeof(struct date))) != NULL) {
			newDate->dValue = d->dValue;
		}
    
		return newDate;
		
	} else {
		return NULL;
		
	}
}


/*Compares date1 and date2 returning <0 if d1 < d2, 0 if d1 == d2, and >0 if d1 > d2*/
int date_compare(Date *date1, Date *date2) {
    return COMP(date1->dValue, date2->dValue);	//Assumes it will only be used by tldlist, NULL checks are performed before calling date_compare
}


/*Destroy date d*/
void date_destroy(Date *d) {
    free(d);
}

