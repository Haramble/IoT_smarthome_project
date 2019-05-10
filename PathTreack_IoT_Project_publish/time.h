/****************************************************************************
 * Definitions
 ****************************************************************************/
#define TIME_STRING_LEN         16
#define DATE_STRING_LEN         16
#define NULL ((void*)0)
#ifndef CONFIG_EXAMPLES_NTPCLIENT_TEST_SERVER_PORT
#define CONFIG_EXAMPLES_NTPCLIENT_TEST_SERVER_PORT   123
#endif

/****************************************************************************
 * Enumeration
 ****************************************************************************/
enum timezone_e {
   TIMEZONE_UTC = 0,
   TIMEZONE_KST,
   TIMEZONE_END
};

/****************************************************************************
 * Structure
 ****************************************************************************/
struct timezone_info_s {
   char str[4];
   int offset;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char g_time_str[TIME_STRING_LEN + 1];
static char g_date_str[DATE_STRING_LEN + 1];
struct timezone_info_s g_timezone[TIMEZONE_END] = {
   {"UTC", 0},
   {"KST", 9}
};
int currTimeS;      // save second

static int dispClock(void) {
   time_t current;
   struct tm *ptm;

   current = time(NULL);         /* UTC time */
    current += (g_timezone[TIMEZONE_KST].offset * 3600);
//    current += (9 * 3600);         //choi 9 hours gap
    ptm = gmtime(&current);
    if (currTimeS != ptm->tm_sec) {
       currTimeS = ptm->tm_sec;
       (void)strftime(g_time_str, TIME_STRING_LEN, "%H:%M:%S", ptm); // current time
       //(void)strftime(g_date_str, DATE_STRING_LEN, "%b %d, %Y", ptm); // current date
       printf("<<%s, %s>>  ", g_time_str,g_timezone[TIMEZONE_KST].str);
    }
}
