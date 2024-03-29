/*
   Thibaut Colar  http://www.colar.net/      2002/05
   This is a dockapp wich shows you the weather at a specific location
   from it's METAR code.
   If this program blew up your computer it isn't my fault :-)
   This use a bit of code borrowed from the wmWeather application(originally).

   To build from src needs libxpm-dev, libx11-dev, libext-dev
 */


/*
 *   Includes
 */
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "xutils.h"
#include "frog.xpm"
#include "frog.xbm"



/*
 *  Delay between refreshes (in microseconds)
 */
#define DEBUG 0
#define VERSION "0.3.1"
// 5 mn in seconds
#define DEFAULT_UPDATEDELAY 300L
// max wind in KNOTS
#define MAX_WIND 50
#define TIME_OFFSET 0
#define METRIC 0

void ParseCMDLine(int argc, char *argv[]);
void ButtonPressEvent(XButtonEvent *);
void KeyPressEvent(XKeyEvent *);
char *StringToUpper(char *);
char *mystrsep(char **stringp, char *delim);
char *GetTempDir(char *suffix);
void UpdateData();
void Repaint();

struct WeatherStruct {
    int intensity;
    char* precip;
    char* desc;
    char* obsc;
    char* misc;
    char* coverage;
    int temp;
    int dew;
    float humidity;
    int hour;
    int min;
    int wind;
    int gust;
    int dir;
} weather;

char StationID[10];
char Label[20];
int Metric = METRIC;
int NeedsUpdate = 1;
int maxWind = MAX_WIND;
int timeOffset = TIME_OFFSET;
long UpdateDelay;
char* folder;
int needsUpdate = 1;

/*
 *   main
 */
int main(int argc, char *argv[]) {
    XEvent event;
    Display *dis;

    dis = XOpenDisplay((char *) 0);

    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);

    if (!folder) {
        folder = GetTempDir(".wmapps");
    }

    initXwindow(argc, argv);

    openXwindow(argc, argv, frog_xpm, frog_bits, frog_width,
            frog_height, "#181818", "#79bdbf", "#ff0000", "#ffbf50", "#c5a6ff");

    XSelectInput(dis, win, ExposureMask);

    // Update the data now .... which calls itself back every x seconds (UpdateDelay) using an alarm
    UpdateData();

    while (1) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose:
                    RedrawWindow();
                    break;
            }
        }
        // check for events 2x seconds to take it easy on cpu
        usleep(500000);
    }
}

void Repaint() {
    int i, y, q;
    char chr;

    /*
     * Clear window.
     */
    copyXPMArea(0, 64, 56, 56, 4, 4);

    //clouds
    if (strcmp(weather.coverage, "FEW") == 0) {
        copyXPMArea(0, 183, 56, 15, 4, 4);
    }
    if (strcmp(weather.coverage, "SCT") == 0) {
        copyXPMArea(56, 183, 56, 15, 4, 4);
    }
    if (strcmp(weather.coverage, "BKN") == 0) {
        copyXPMArea(112, 183, 56, 15, 4, 4);
    }
    if (strcmp(weather.coverage, "OVC") == 0 || strcmp(weather.coverage, "VV") == 0) {
        copyXPMArea(168, 183, 56, 15, 4, 4);
    }

    //obstruction
    if (strcmp(weather.obsc, "") != 0) { //fog
        if (strcmp(weather.desc, "BL") == 0)
            copyXPMArea(56, 90, 56, 26, 4, 34);
        else if (strcmp(weather.desc, "FZ") == 0)
            copyXPMArea(112, 90, 56, 26, 4, 34);
        else
            copyXPMArea(168, 64, 56, 26, 4, 34);
    }

    //precipitation
    if (strcmp(weather.precip, "DZ") == 0) {
        copyXPMArea(112, 120, 56, 15, 4, 18);
    }
    if (strcmp(weather.precip, "RA") == 0) {
        if (weather.intensity == 0)
            copyXPMArea(56, 150, 56, 15, 4, 18);
        if (weather.intensity < 0)
            copyXPMArea(0, 150, 56, 15, 4, 18);
        if (weather.intensity > 0)
            copyXPMArea(112, 150, 56, 15, 4, 18);
    }

    if (strcmp(weather.precip, "SN") == 0 || strcmp(weather.precip, "SG") == 0) {
        if (weather.intensity == 0)
            copyXPMArea(56, 135, 56, 15, 4, 18);
        if (weather.intensity > 0)
            copyXPMArea(112, 135, 56, 15, 4, 18);
        if (weather.intensity < 0)
            copyXPMArea(0, 135, 56, 15, 4, 18);
    }

    if (strcmp(weather.precip, "IC") == 0 || strcmp(weather.precip, "IC") == 0 || strcmp(weather.precip, "IC") == 0 || strcmp(weather.precip, "IC") == 0) {
        if (weather.intensity == 0)
            copyXPMArea(56, 166, 56, 16, 4, 18);
        if (weather.intensity > 0)
            copyXPMArea(112, 166, 56, 16, 4, 18);
        if (weather.intensity < 0)
            copyXPMArea(0, 166, 56, 16, 4, 18);
    }

    //descriptor
    if (strcmp(weather.desc, "BL") == 0 && strcmp(weather.obsc, "") == 0) {
        copyXPMArea(56, 64, 56, 26, 4, 34);
    }
    if (strcmp(weather.desc, "FZ") == 0 && strcmp(weather.obsc, "") == 0) {
        copyXPMArea(112, 64, 56, 26, 4, 34);
    }
    if (strcmp(weather.desc, "TS") == 0) {
        if (strcmp(weather.precip, "") != 0)
            copyXPMArea(164, 150, 56, 15, 4, 18);
        else
            copyXPMArea(56, 120, 56, 15, 4, 18);
    }

    //special
    if (strcmp(weather.misc, "") != 0) {
        copyXPMArea(168, 90, 56, 26, 4, 34);
        copyXPMArea(0, 120, 56, 15, 4, 18);
        copyXPMArea(168, 183, 56, 16, 4, 4);

    }

    //writtings:

    //clearing:
    copyXPMArea(170, 20, 25, 17, 29, 44);
    copyXPMArea(220, 0, 3, 60, 4, 4);
    copyXPMArea(220, 0, 3, 60, 57, 4);
    copyXPMArea(220, 0, 3, 60, 4, 4);

    //humidity
    y = (weather.humidity * 58) / 100;
    copyXPMArea(60, 0, 1, y, 4, 60 - y);
    copyXPMArea(60, 0, 1, y, 5, 60 - y);

    //wind & gust

    y = (weather.gust * 58) / maxWind;
    copyXPMArea(62, 0, 1, y, 58, 60 - y);
    copyXPMArea(62, 0, 1, y, 59, 60 - y);
    y = (weather.wind * 58) / maxWind;
    copyXPMArea(61, 0, 1, y, 58, 60 - y);
    copyXPMArea(61, 0, 1, y, 59, 60 - y);

    //station name
    for (i = 0; i != 4; i++) {
        chr = (int) Label[i] - 65;
        copyXPMArea(chr * 5 + 89, 0, 5, 6, 31 + (i * 5), 45);
    }

    //time
    i = weather.hour / 10;
    copyXPMArea(89 + (i * 5), 7, 5, 6, 30, 53);
    i = weather.hour - ((weather.hour / 10)*10);
    copyXPMArea(89 + (i * 5), 7, 5, 6, 35, 53);
    copyXPMArea(89 + (52), 7, 1, 6, 41, 53);
    i = weather.min / 10;
    copyXPMArea(89 + (i * 5), 7, 5, 6, 43, 53);
    i = weather.min - ((weather.min / 10)*10);
    copyXPMArea(89 + (i * 5), 7, 5, 6, 48, 53);


    //temp

    q = 10;
    copyXPMArea(136, 30, 17, 9, q - 1, 22);
    if (weather.temp < 0) {
        copyXPMArea(90, 30, 3, 7, q, 23);
        q += 5;
    }
    /*if(temp>100)
      {
      copyXPMArea(96, 20, 5, 7, q, 23);
      q+=6;
      }
     */
    copyXPMArea(136, 30, 17, 9, (q - 1), 22);

    i = weather.temp;
    if (i < 0)
        i = -i;
    if (i / 100 > 0) {
        copyXPMArea(96, 20, 5, 7, q, 23);
        q += 6;
        i -= 100;
    }
    if ((i == 0 && q > 10) || i / 10 > 0 || q > 10) {
        copyXPMArea(90 + 6 * (i / 10), 20, 5, 7, q, 23);
        q += 6;
    }

    copyXPMArea(90 + 6 * (i - ((i / 10)*10)), 20, 5, 7, q, 23);
    q += 5;
    copyXPMArea(95, 29, 5, 7, q, 23);

    // wind weather.dir
    if ((weather.dir >= 0 && weather.dir <= 22) || (weather.dir <= 360 && weather.dir > 337))
        copyXPMArea(0, 198, 7, 7, 48, 6);
    if (weather.dir > 22 && weather.dir <= 67)
        copyXPMArea(42, 198, 7, 7, 48, 6);
    if (weather.dir > 67 && weather.dir <= 112)
        copyXPMArea(14, 198, 7, 7, 48, 6);
    if (weather.dir > 112 && weather.dir <= 157)
        copyXPMArea(28, 198, 7, 7, 48, 6);
    if (weather.dir > 157 && weather.dir <= 202)
        copyXPMArea(7, 198, 7, 7, 48, 6);
    if (weather.dir > 202 && weather.dir <= 247)
        copyXPMArea(35, 198, 7, 7, 48, 6);
    if (weather.dir > 247 && weather.dir <= 292)
        copyXPMArea(21, 198, 7, 7, 48, 6);
    if (weather.dir > 292 && weather.dir <= 337)
        copyXPMArea(49, 198, 7, 7, 48, 6);

    /*
     * Make changes visible
     */
    RedrawWindow();
}

/*
 *   ParseCMDLine()
 */
void ParseCMDLine(int argc, char *argv[]) {

    int i;
    void print_usage();

    StationID[0] = '\0';
    UpdateDelay = DEFAULT_UPDATEDELAY;
    for (i = 1; i < argc; i++) {

        if ((!strcmp(argv[i], "-metric")) || (!strcmp(argv[i], "-m"))) {
            Metric = 1;
        } else if (!strcmp(argv[i], "-w")) {
            if ((i + 1 >= argc) || (argv[i + 1][0] == '-')) {

                fprintf(stderr, "You must give a max wind value  with the -w option.\n");
                print_usage();
                exit(-1);
            } else if (sscanf(argv[i + 1], "%d", &maxWind) != 1) {
                fprintf(stderr, "Don't understand the max wind value have entered (%s).\n", argv[i + 1]);
                print_usage();
                exit(-1);
            }
        } else if (!strcmp(argv[i], "-o")) {
            if ((i + 1 >= argc)) {
                fprintf(stderr, "You must give a time offset value  with the -t option.\n");
                print_usage();
                exit(-1);
            } else if (sscanf(argv[i + 1], "%d", &timeOffset) != 1) {
                fprintf(stderr, "Don't understand the time offset value have entered (%s).\n", argv[i + 1]);
                print_usage();
                exit(-1);
            }


        } else if (!strcmp(argv[i], "-tmp")) {
            if ((i + 1 >= argc) || (argv[i + 1][0] == '-')) {
                fprintf(stderr, "-tmp option invalid.\n");
                print_usage();
                exit(-1);
            }
            folder = strdup(argv[++i]);


        } else if ((!strcmp(argv[i], "-station")) || (!strcmp(argv[i], "-s"))) {
            if ((i + 1 >= argc) || (argv[i + 1][0] == '-')) {
                fprintf(stderr, "No METAR station ID found\n");
                print_usage();
                exit(-1);
            }
            strcpy(StationID, StringToUpper(argv[++i]));
            strcpy(Label, StationID);
        } else if (!strcmp(argv[i], "-delay")) {
            if ((i + 1 >= argc) || (argv[i + 1][0] == '-')) {

                fprintf(stderr, "You must give a time with the -delay option.\n");
                print_usage();
                exit(-1);
            } else if (sscanf(argv[i + 1], "%ld", &UpdateDelay) != 1) {

                fprintf(stderr, "Don't understand the delay time you have entered (%s).\n", argv[i + 1]);
                print_usage();
                exit(-1);
            }
            /*
             *  Convert Time to seconds
             */
            UpdateDelay *= 60;
            ++i;
        } else if (!strcmp(argv[i], "-l")) {
            if ((i + 1 >= argc) || (argv[i + 1][0] == '-')) {
                fprintf(stderr, "You must give a station label name.\n");
                print_usage();
                exit(-1);
            }
            strcpy(Label, StringToUpper(argv[++i]));
        }
    }

    if (StationID[0] == '\0') {
        fprintf(stderr, "\nYou must specify a METAR station code\n\n");
        print_usage();
        exit(1);
    }
}

void print_usage() {

    printf("\nwmfrog version: %s\n", VERSION);
    printf("\nusage: wmfrog -s <StationID> [-h] [-w <max wind>] [-t <time offset>] [-tmp <temp folder>] [-l <label>]\n");
    printf("                   [-m] [-delay <Time in Minutes>]\n\n");
    printf("\n\t-station <METAR StationID>\n");
    printf("\t-s       <METAR StationID>\tThe 4-character METAR Station ID.\n\n");
    printf("\t-metric\n");
    printf("\t-m\t\t\t\tDisplay Temperature in metric unit: (Celcius) \n\n");
    printf("\t-delay <Time in Minutes>\tOverride time (in minutes) between updates\n");
    printf("\t\t\t\t\tdefault is %ld minutes. \n\n", DEFAULT_UPDATEDELAY / 60);

    printf("\t-w\t\t\t\tSet the maximum wind in KNOTS (for percentage of max / scaling.)\n");
    printf("\t\t\t\t\tdefault is %d minutes). (Times are approximate.)\n\n", MAX_WIND);
    printf("\t-o\t\t\t\tTime offset of location from UTC (7 for calif in summer) \n\n");
    printf("\t-tmp\t\t\t\tSet the temporary folder (default is: %s) \n\n", folder);
    printf("\t-l\t\t\t\tSet a label to replace the station ID \n\n");

    printf("\n\nTo find out more about the METAR/TAF system, look at:\n");
    printf("	 http://nws.noaa.gov/ops2/Surface/metar_taf.htm \n\n");
    printf("To find your city Metar code go to  NOAA's ""Meteorological Station Information\nLookup"" page at:\n\n");
    printf("	 http://www.nws.noaa.gov/tg/siteloc.php\n\n");
    printf("\n Thibaut Colar http://www.colar.net/wmapps/\n\n");

}

/*
 *  Compute the Julian Day number for the given date.
 *  Julian Date is the number of days since noon of Jan 1 4713 B.C.
 */
double jd(ny, nm, nd, UT)
int ny, nm, nd;
double UT;
{
    double A, B, C, D, JD, day;

    day = nd + UT / 24.0;

    if ((nm == 1) || (nm == 2)) {
        ny = ny - 1;
        nm = nm + 12;
    }

    if (((double) ny + nm / 12.0 + day / 365.25) >= (1582.0 + 10.0 / 12.0 + 15.0 / 365.25)) {
        A = ((int) (ny / 100.0));
        B = 2.0 - A + (int) (A / 4.0);
    } else {
        B = 0.0;
    }
    if (ny < 0.0) {
        C = (int) ((365.25 * (double) ny) - 0.75);
    } else {
        C = (int) (365.25 * (double) ny);
    }

    D = (int) (30.6001 * (double) (nm + 1));


    JD = B + C + D + day + 1720994.5;
    return (JD);

}

// Will be called at regular interval to update the weather data (alarm)

void UpdateData() {
    char command[1024], Line[512], FileName[128];
    int ign;
    char* igns;
    igns = (char*) malloc(512);
    FILE *fp;
    char* weatherStr;
    char* cloudsStr;
    char* tmp1 = NULL;
    char* tmp2 = NULL;
    int keepgoing = 0;
    int weatherFound = 0;
    int cloudsFound = 0;
    char* wtr;
    char* clds;
    int i, z;
    weatherStr = (char*) malloc(512);
    cloudsStr = (char*) malloc(512);

    /*
     *  Check the Current Conditions file every (approx.) several seconds.
     */
    weather.intensity = 0;
    weather.desc = "";
    weather.precip = "";
    weather.obsc = "";
    weather.misc = "";
    weather.coverage = "";
    weather.dir = -1;
    weather.temp = 0;
    weather.dew = 0;
    weather.humidity = 0;
    weather.hour = 0;
    weather.min = 0;
    weather.wind = 0;
    weather.gust = 0;
    /*
     *  Execute Perl script to grab the Latest METAR Report
     */
    snprintf(command, 1024, "/usr/share/wmfrog/weather.pl %s %s", StationID, folder);
    //printf("Retrieveing data\n");
    ign = system(command);
    snprintf(FileName, 128, "%s/%s", folder, StationID);
    //fprintf(stderr,"%s\n\n",FileName);

    if ((fp = fopen(FileName, "r")) != NULL) {
        ign = fscanf(fp, "Hour:%d", &weather.hour);
        igns = fgets(Line, 512, fp); //h
        ign = fscanf(fp, "Minute:%d", &weather.min);
        igns = fgets(Line, 512, fp); //mn
        igns = fgets(igns, 512, fp); //station . ignored
        ign = fscanf(fp, "WindDir:%d", &weather.dir);
        igns = fgets(Line, 512, fp); //dir
        ign = fscanf(fp, "WindSpeed:%d", &weather.wind);
        igns = fgets(Line, 512, fp); //wind
        ign = fscanf(fp, "WindGust:%d", &weather.gust);
        igns = fgets(Line, 512, fp); //gust
        igns = fgets(weatherStr, 512, fp); //weather
        weatherFound = (*igns != '\0');
        igns = fgets(cloudsStr, 512, fp); //clouds
        cloudsFound = (*igns != '\0');
        ign = fscanf(fp, "Temp:%d", &weather.temp);
        igns = fgets(Line, 512, fp); //temp
        ign = fscanf(fp, "Dew:%d", &weather.dew);
        igns = fgets(Line, 512, fp); //dew

        if (DEBUG) {
            printf("hr: %d\n", weather.hour);
            printf("mn: %d\n", weather.min);
            printf("wd: %d\n", weather.dir);
            printf("ws: %d\n", weather.wind);
            printf("wg: %d\n", weather.gust);
            printf("temp: %d\n", weather.temp);
            printf("dew: %d\n", weather.dew);
            printf("w: %s\n", weatherStr);
            printf("c: %s\n", cloudsStr);
        }

        fclose(fp);

        if (weatherFound) {
            wtr = strchr(weatherStr, ':') + 1; // get the part after "Weather:"
            if (DEBUG) printf("weatherLine: %s\n", wtr);
            do {
                tmp1 = mystrsep(&wtr, ";");
                if (tmp1 && strlen(tmp1) > 2) {
                    if (DEBUG) printf("tmp1: %s\n", tmp1);
                    tmp2 = mystrsep(&tmp1, ",");

                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        if (strcmp("-", tmp2) == 0) {
                            weather.intensity = -1;
                        } else if (strcmp("+", tmp2) == 0) {
                            weather.intensity = +1;
                        }
                    }
                    tmp2 = mystrsep(&tmp1, ",");
                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        weather.desc = tmp2;
                    }
                    tmp2 = mystrsep(&tmp1, ",");
                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        weather.precip = tmp2;
                    }
                    tmp2 = mystrsep(&tmp1, ",");
                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        weather.obsc = tmp2;
                    }
                    tmp2 = mystrsep(&tmp1, ",");
                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        weather.misc = tmp2;
                    }
                } else {
                    keepgoing = 0;
                }
                if (DEBUG) printf("Intensity: %d \n", weather.intensity);
            } while (keepgoing);
        }


        if (cloudsFound) {
            clds = strchr(cloudsStr, ':') + 1; // get the part after "Clouds:"
            if (DEBUG) printf("cloudsLine: %s\n", clds);
            do {
                tmp1 = mystrsep(&clds, ";");
                if (DEBUG) printf("tmp1: %s\n", tmp1);
                if (tmp1 && strlen(tmp1) > 2) {
                    tmp2 = mystrsep(&tmp1, ",");

                    if (tmp2 && strcmp(tmp2, "") != 0) {
                        weather.coverage = tmp2;
                    }
                }
                if (DEBUG) printf("Coverage: |%s|\n", weather.coverage);
            } while (tmp1);
        }
        weather.humidity = ((112.0 - 0.1 * weather.temp + weather.dew) / (112.0 + 0.9 * weather.temp));
        z = weather.humidity;
        for (i = 0; i != 8; i++)
            weather.humidity *= z;
        weather.humidity *= 100;
        if (weather.humidity > 100)
            weather.humidity = 100;
        if (weather.humidity < 0)
            weather.humidity = 0;

        weather.hour = weather.hour + timeOffset;
        if (weather.hour < 0)
            weather.hour = 24 + weather.hour;
        if (weather.hour > 24)
            weather.hour = weather.hour - 24;

        if (!Metric)
            weather.temp = (weather.temp * 9) / 5 + 32;
    }

    // set next alarm
    signal(SIGALRM, UpdateData);
    /* Set an alarm to go off in a little while. */
    alarm(UpdateDelay);

    Repaint();
}

char *GetTempDir(char *suffix) {
    uid_t id;
    struct passwd *userEntry;
    static char userHome[128];

    id = getuid();
    userEntry = getpwuid(id);
    snprintf(userHome, 128, "%s/%s", userEntry->pw_dir, suffix);
    return userHome;
}

char *StringToUpper(char *String) {

    int i;

    for (i = 0; i < strlen(String); i++)
        String[i] = toupper(String[i]);

    return String;
}

char *mystrsep(char **stringp, char *delim) {
    char *start = *stringp;
    char *cp;
    char ch;
    if (start == NULL)
        return NULL;
    for (cp = start; (ch = *cp) != 0; cp++) {
        if (strchr(delim, ch)) {
            *cp++ = 0;
            *stringp = cp;
            return start;
        }
    }
    *stringp = NULL;
    return start;
}
