#include <string.h>
#include <stdlib.h>
#include "TimeParser.h"

int time_parse(char *time){
   if (time == NULL || strlen(time) != 6) {
      return TIME_LEN_ERROR;
   }

   for (int i = 0; i < 6; i++){
        if (time[i] < '0' || time[i] > '9') {
            return TIME_ARRAY_ERROR;
        }
   }

   char temp[7];
    strcpy(temp, time);

    int values[3];
    values[2] = atoi(temp + 4);
    temp[4] = '\0';
    values[1] = atoi(temp + 2);
    temp[2] = '\0'; 
    values[0] = atoi(temp);

    if (values[0] < 0 || values[0] > 23 || values[1] < 0 || values[1] > 59 || values[2] < 0 || values[2] > 59) {
        return TIME_VALUE_ERROR;
    }

    int seconds = values[0] * 3600 + values[1] * 60 + values[2];

    return seconds;
}
