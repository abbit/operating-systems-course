#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    putenv("TZ=America/Los_Angeles");
    time_t now;
    struct tm *sp;
    time(&now);
    printf("%s", ctime(&now));
    sp = localtime(&now);
    if (sp->tm_isdst < 0) {
        printf("%d/%d/%02d %d:%02d\n",
               sp->tm_mon + 1, sp->tm_mday,
               sp->tm_year + 1900, sp->tm_hour,
               sp->tm_min);
    }
    else {
        printf("%d/%d/%02d %d:%02d %s\n",
               sp->tm_mon + 1, sp->tm_mday,
               sp->tm_year + 1900, sp->tm_hour,
               sp->tm_min, tzname[sp->tm_isdst]);
    }
    return 0;
}