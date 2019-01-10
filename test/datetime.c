#include <masc/datetime.h>
#include <masc/macro.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    // Get current time
    DateTime *dt = new(DateTime, NULL);
    print("Current time: %O\n", dt);
    datetime_from_unix(dt, 0);
    print("datetime_from_unix(0): %O\n", dt);
    // Parse date
    char *dates[] = {"1981-06-19", "1979-1-1 00:00", "2019-01-10 15:45:44",
        "1234-12-01", "2000-07-31 9:12:34.123456"
    };
    for (int i = 0; i < ARRAY_LEN(dates); i++) {
        DateTime *new_dt = new(DateTime, dates[i]);
        print("parse '%s': unix: %li, ", dates[i], datetime_to_unix(new_dt));
        if (datetime_is_valid(new_dt)) {
            put(new_dt);
        } else {
            puts("failed!");
        }
        delete(new_dt);
    }
    delete(dt);
    return 0;
}
