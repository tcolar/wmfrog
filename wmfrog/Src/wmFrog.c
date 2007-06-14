/**
   Thibaut Colar  http://www.colar.net/      2002/05
   This is a dockapp wich shows you the weather at a specific location
   from it's METAR code.
   If this program blew up your computer it isn't my fault :-)
   This use a bit of code borrowed from the wmWeather application.
 */


/*  
 *   Includes  
 */
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
#define DELAY 100000L
#define VERSION "0.1.6"
// 5 mn
#define DEFAULT_UPDATEDELAY 900L
// max wind in KNOTS
#define MAX_WIND 50
#define TIME_OFFSET 0
#define METRIC 0

void  ParseCMDLine(int argc, char *argv[]);
void  ButtonPressEvent(XButtonEvent *);
void  KeyPressEvent(XKeyEvent *);
char *StringToUpper(char *);
char *mystrsep(char **stringp, char *delim);
char *GetTempDir(char *suffix);

char 		StationID[10];
char		Label[20];
int		UpToDate = 0;
int		Metric = METRIC;
int		ForceUpdate = 1;
int		ForceDownload = 1;
int             maxWind=MAX_WIND;
int             timeOffset=TIME_OFFSET;
long		UpdateDelay;
int             GotFirstClick1, GotDoubleClick1;
int             GotFirstClick2, GotDoubleClick2;
int             GotFirstClick3, GotDoubleClick3;
int             DblClkDelay;
char* folder;

/*  
 *   main  
 */
int main(int argc, char *argv[]) {
int		n, s, m, i, dt1, dt2, dt3    ;
XEvent		event;
char		command[1024], Line[512], FileName[10];
FILE		*fp;
 char*          Weather;
 char*          Clouds;
 char* tmp1;
 char* tmp2;
 int intensity=0;
 char* precip;
 char* desc;
 char* obsc;
  char* misc;
  char* coverage;
  int keepgoing=0;
  int weatherFound=0;
  int cloudsFound=0;
  char chr;
  int temp=0;
  int dew=0;
  float humidity=0;	  
  float z;
  int y;
  int wind=0;
  int gust=0;
  int min=0;
  int hour=0;
  int dir=-1;
  int q;

	folder=GetTempDir(".wmapps");
	//fprintf(stderr,"User dir: %s\n",folder);

    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);
	   
    initXwindow(argc, argv);
    openXwindow(argc, argv, frog_xpm, frog_bits, frog_width, 
	frog_height, "#181818", "#79bdbf", "#ff0000", "#ffbf50", "#c5a6ff");
	   
    /*
     *  Loop until we die
     */
    n = 32000;
    s = 32000;
    m = 32000;
    dt1 = 32000;
    dt2 = 32000;
    dt3 = 32000;
    DblClkDelay = 32000;
    UpToDate = 0;
    while(1) {




	/*
	 *  Keep track of # of seconds
	 */
	if (m > 1000000/DELAY){
	
	    m = 0;
	    ++dt1;
	    ++dt2;
	    ++dt3;
	
	} else {
	
	    /*
	     *  Increment counter
	     */
	    ++m;
	
	}






        /*
         *  Double Click Delays
         *  Keep track of click events. If Delay too long, set GotFirstClick's to False.
         */
        if (DblClkDelay > 15) {

            DblClkDelay = 0;
            GotFirstClick1 = 0; GotDoubleClick1 = 0;
            GotFirstClick2 = 0; GotDoubleClick2 = 0;
            GotFirstClick3 = 0; GotDoubleClick3 = 0;

        } else {

            ++DblClkDelay;

        }



	/* 
	 *   Process any pending X events.
	 */
        while(XPending(display)){
            XNextEvent(display, &event);
            switch(event.type){
                case Expose:
                        RedrawWindow();
                        break;
                case ButtonPress:
                        ButtonPressEvent(&event.xbutton);
                        break;
                case KeyPress:
                        KeyPressEvent(&event.xkey);
                        break;
                case ButtonRelease:
                        break;
                case EnterNotify:
                        XSetInputFocus(display, PointerRoot, RevertToParent, CurrentTime);
                        break;
                case LeaveNotify:
                        XSetInputFocus(display, PointerRoot, RevertToParent, CurrentTime);
                        break;

            }
        }
	


	/*
	 *  Check the Current Conditions file every (approx.) several seconds.
	 *  Can significantly reduce this frequency later. But its
	 *  easier to debug this way...
	 *  Do this before trying to download again! The file may be there and it
	 *  may be Up-To-Date!
	 */
	intensity=0;
	    desc="";
	    precip="";
	    obsc="";
	    misc="";
	    coverage="";
	    dir=-1;

	if ((dt2 > 15)||(ForceUpdate)){
	    
	    dt2 = 0;

	    sprintf(FileName, "%s/%s", folder, StationID);
		fprintf(stderr,"%s\n\n",FileName);
    	    if ((fp = fopen(FileName, "r")) != NULL){
	      fscanf(fp, "Hour:%d", &hour);
	      fgets(Line, 512, fp);//h
	      fscanf(fp, "Minute:%d", &min);
	      fgets(Line, 512 , fp);//station
	      fgets(Line, 512, fp);//wdir
	      fscanf(fp, "WindDir:%d", &dir);
	      fgets(Line, 512, fp);//wspeed
	      fscanf(fp, "WindSpeed:%d", &wind);
	      fgets(Line, 512, fp);//wgust
	      fscanf(fp, "WindGust:%d", &gust);
	      fgets(Line, 512, fp);//weat
	      weatherFound=fscanf(fp, "Weather:%as", &Weather);
	      fgets(Line, 512, fp);//cld
	      cloudsFound=fscanf(fp, "Clouds:%as", &Clouds);
	      fgets(Line, 512, fp);//temp
	      fscanf(fp, "Temp:%d", &temp);
	      fgets(Line, 512, fp);//dew
	      fscanf(fp,"Dew:%d", &dew);
	      fclose(fp);
	      keepgoing=1;
		    {

		do{

		    
		    tmp1=mystrsep(&Weather,";");
		    if(tmp1)
		      {

			  tmp2=mystrsep(&tmp1,",");
			    
			      if(tmp2 && strcmp(tmp2,"")!=0)
			      {
				if(strcmp("-",tmp2)==0)
				{
				  intensity=-1;
				}
				else if(strcmp("+",tmp2)==0)
				{
				  intensity=+1;
				}
			      }   
			  tmp2=mystrsep(&tmp1,",");
			  if(tmp2 && strcmp(tmp2,"")!=0)
			    {
			      desc=tmp2;
			    }
			  tmp2=mystrsep(&tmp1,",");
			  if(tmp2 && strcmp(tmp2,"")!=0)
			    {
			      precip=tmp2;
			    }
			  tmp2=mystrsep(&tmp1,",");
			  if(tmp2 && strcmp(tmp2,"")!=0)
			    {
			      obsc=tmp2;
			    }
			  tmp2=mystrsep(&tmp1,",");
			  if(tmp2 && strcmp(tmp2,"")!=0)
			    {
			      misc=tmp2;
			    }
			
		      }
		    else
		      {
			keepgoing=0;
		      }
		    
		}
		while(keepgoing);
		    }

		do{
		    tmp1=mystrsep(&Clouds,";");
		    if(tmp1)
		      {
			  tmp2=mystrsep(&tmp1,",");
			    
			      if(tmp2 && strcmp(tmp2,"")!=0)
			      {
				  coverage=tmp2;;
			      }
		      }
	    }
	    while(tmp1);

		    humidity=((112.0-0.1*temp+dew)/(112.0+0.9*temp));
		    z=humidity;
		    for(i=0;i!=8;i++)
		      humidity*=z;
		    humidity*=100;
			if(humidity>100)
				humidity=100;
			if(humidity<0)
				humidity=0;


		    hour=hour+timeOffset;
		    if(hour<0)
		      {
			hour=24+hour;
		      }
		    if(hour>24)
		      {
			hour=hour-24;
		      }

		    if(!Metric)
		      {
			temp=(temp*9)/5+32;
		      }
		    
	}



	}


	/*
	 *  Draw window.
	 */
	if ( (dt3 > 15) || ForceUpdate){

	    dt3 = 0;
	    


	    /*
	     * Clear window.
	     */
	    copyXPMArea(0,64,56,56,4,4);


	    //clouds
	    if(strcmp(coverage,"FEW")==0)
	    {
	      copyXPMArea(0, 183, 56, 15, 4, 4); 
	    }
	    if(strcmp(coverage,"SCT")==0)
	    {
	      copyXPMArea(56, 183, 56, 15, 4, 4); 
	    }
	    if(strcmp(coverage,"BKN")==0)
	    {
	      copyXPMArea(112, 183, 56, 15, 4, 4); 
	    }
	    if(strcmp(coverage,"OVC")==0 || strcmp(coverage,"VV")==0)
	    {
	      copyXPMArea(168, 183, 56, 15, 4, 4); 
	    }

	    //obstruction
	    if(strcmp(obsc,"")!=0)
	      { //fog
	      if(strcmp(desc,"BL")==0)
	      copyXPMArea(56, 90, 56, 26, 4, 34);
	      else
	      if(strcmp(desc,"FZ")==0)
	      copyXPMArea(112, 90, 56, 26, 4, 34); 
	      else
	      copyXPMArea(168, 64, 56, 26, 4, 34); 
	    }

	    //precipitation
	    if(strcmp(precip,"DZ")==0)
	    {
	      copyXPMArea(112, 120, 56, 15, 4, 18);
	    }
	    if(strcmp(precip,"RA")==0)
	    {
	    if(intensity==0)
	      copyXPMArea(56, 150, 56, 15, 4, 18);
	    if(intensity<0)
	      copyXPMArea(0, 150, 56, 15, 4, 18);
	    if(intensity>0)
	      copyXPMArea(112, 150, 56, 15, 4, 18);
	    }

	    if(strcmp(precip,"SN")==0 || strcmp(precip,"SG")==0)
	    {
	    if(intensity==0)
	      copyXPMArea(56, 135, 56, 15, 4, 18);
	    if(intensity>0)
	      copyXPMArea(112, 135, 56, 15, 4, 18);
	    if(intensity<0)
	      copyXPMArea(0, 135, 56, 15, 4, 18);
	    }

	    if(strcmp(precip,"IC")==0 || strcmp(precip,"IC")==0 || strcmp(precip,"IC")==0 || strcmp(precip,"IC")==0)
	    {
	    if(intensity==0)
	      copyXPMArea(56, 166, 56, 16, 4, 18);
	    if(intensity>0)
	      copyXPMArea(112, 166, 56, 16, 4, 18);
	    if(intensity<0)
	      copyXPMArea(0, 166, 56, 16, 4, 18);
	    }

	    //descriptor
 	    if(strcmp(desc,"BL")==0 && strcmp(obsc,"")==0)
	    {
	      copyXPMArea(56, 64, 56, 26, 4, 34);
	    }
 	    if(strcmp(desc,"FZ")==0 && strcmp(obsc,"")==0)
	    {
	      copyXPMArea(112, 64, 56, 26, 4, 34);
	    }
 	    if(strcmp(desc,"TS")==0)
	    {
	      if(strcmp(precip,"")!=0)	      
		copyXPMArea(164, 150, 56, 15, 4, 18);	      
	      else
		copyXPMArea(56, 120, 56, 15, 4, 18);	      
	    }
	    
	    //special
	    if(strcmp(misc,"")!=0)
	      {
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
	    y=(humidity*58)/100;
 	    copyXPMArea(60, 0, 1, y, 4, 60-y);
 	    copyXPMArea(60, 0, 1, y, 5, 60-y);
	    
	    //wind & gust

	    y=(gust*58)/maxWind;
 	    copyXPMArea(62, 0, 1, y, 58, 60-y);
 	    copyXPMArea(62, 0, 1, y, 59, 60-y);
	    y=(wind*58)/maxWind;
 	    copyXPMArea(61, 0, 1, y, 58, 60-y);
 	    copyXPMArea(61, 0, 1, y, 59, 60-y);

	    //station name
	    for(i=0;i!=4;i++)
	      {
	     chr = (int)Label[i] - 65; 
	     copyXPMArea(chr*5+89, 0, 5, 6, 31+(i*5), 45); 
	      }

	    //time
	    i=hour/10;
	     copyXPMArea(89+(i*5), 7, 5, 6, 30, 53); 
	     i=hour-((hour/10)*10);
	     copyXPMArea(89+(i*5), 7, 5, 6, 35, 53); 
	     copyXPMArea(89+(52), 7, 1, 6, 41, 53); 
	    i=min/10;
	     copyXPMArea(89+(i*5), 7, 5, 6, 43, 53); 
	     i=min-((min/10)*10);
	     copyXPMArea(89+(i*5), 7, 5, 6, 48, 53); 
	    

	    //temp

	    q=10;
	    copyXPMArea(136, 30, 17, 9, q-1, 22); 
	    if(temp<0)
	      {
		copyXPMArea(90, 30, 3, 7, q, 23); 
		q+=5;
	      }
	    /*if(temp>100)
	      {
	      copyXPMArea(96, 20, 5, 7, q, 23);
	      q+=6;
	      }
*/
	    copyXPMArea(136, 30, 17, 9, (q-1), 22); 


	    i=temp;
		if(i<0)
		  i=-i;
	    if(i/100>0)
	      {
	      copyXPMArea(96, 20, 5, 7, q, 23);
	      q+=6;
	      i-=100;
	      }
	    if((i==0 && q>10) || i/10>0 || q > 10)
	      {
	      copyXPMArea(90+6*(i/10), 20, 5, 7, q, 23);
	      q+=6;
	      }
	    
	      copyXPMArea(90+6*(i-((i/10)*10)), 20, 5, 7, q, 23);
	      q+=5;
	      copyXPMArea(95, 29, 5, 7, q, 23);

	      // wind dir
	      if((dir>=0 && dir<=22) || (dir<=360 && dir>337))
	      copyXPMArea(0, 198, 7, 7, 48, 6);
		 if(dir>22 && dir<=67)
		 copyXPMArea(42, 198, 7, 7, 48, 6);
		 if(dir>67 && dir<=112)
		 copyXPMArea(14, 198, 7, 7, 48, 6);
		 if(dir>112 && dir<=157)
		 copyXPMArea(28, 198, 7, 7, 48, 6);
		 if(dir>157 && dir<=202)
		 copyXPMArea(7, 198, 7, 7, 48, 6);
		 if(dir>202 && dir<=247)
		 copyXPMArea(35, 198, 7, 7, 48, 6);
		 if(dir>247 && dir<=292)
		 copyXPMArea(21, 198, 7, 7, 48, 6);
	         if(dir>292 && dir<=337)
		 copyXPMArea(49, 198, 7, 7, 48, 6);
		 
	    /*
	     * Make changes visible
	     */
	    RedrawWindow();


	



	/*
	 *  Reset "force update" flag
	 */
	ForceUpdate = 0;
	}



	/*
	 *  Check every 5 min if the values are not up to date...
	 */
/*
 *  We still need to add a flashing LED to warn about
 *  times that are out of date. Also need to determine if it is uptodate...
 */
UpToDate = 0;
	if (((!UpToDate)&&(dt1 > UpdateDelay)) || ForceDownload){

	    dt1 = 0;

	    /*
	     *  Execute Perl script to grab the Latest METAR Report
	     */
	    sprintf(command, "/usr/lib/wmfrog/weather.pl %s %s &", StationID, folder);
	    //printf("Retrieveing data\n");
	    system(command);
	    ForceDownload = 0;
	    ForceUpdate = 1;
	}




	/* 
	 *  Wait for next update 
	 */
	usleep(DELAY);
    }

}

/*
 *   ParseCMDLine()
 */
void ParseCMDLine(int argc, char *argv[]) {

    int  i;
    void print_usage();
 
    StationID[0] = '\0';
    UpdateDelay = DEFAULT_UPDATEDELAY;
    for (i = 1; i < argc; i++) {

        if ((!strcmp(argv[i], "-metric"))||(!strcmp(argv[i], "-m"))){

	    Metric = 1;

        } else if (!strcmp(argv[i], "-w")){

	    if( (i+1 >= argc)||(argv[i+1][0] == '-')) {

		fprintf(stderr,"You must give a max wind value  with the -w option.\n");
		print_usage();
		exit(-1);

	    } else if(sscanf(argv[i+1], "%d", &maxWind) != 1) {

		fprintf(stderr,"Dont understand the max wind value have entered (%s).\n", argv[i+1]);
		print_usage();
		exit(-1);

	    } 

        } else if (!strcmp(argv[i], "-o")){
	    if( (i+1 >= argc)) {

		fprintf(stderr,"You must give a time offset value  with the -t option.\n");
		print_usage();
		exit(-1);

	    } else if(sscanf(argv[i+1], "%d", &timeOffset) != 1) {

		fprintf(stderr,"Dont understand the time offset value have entered (%s).\n", argv[i+1]);
		print_usage();
		exit(-1);

	    } 


        } else if (!strcmp(argv[i], "-tmp")){
	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "-tmp option invalid.\n");
		print_usage();
		exit(-1);
	    }
            strcpy(folder, argv[++i]);

 
        } else if ((!strcmp(argv[i], "-station"))||(!strcmp(argv[i], "-s"))){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "No METAR station ID found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(StationID, StringToUpper(argv[++i]));
            strcpy(Label, StationID);

        } else if (!strcmp(argv[i], "-delay")) {

	    if( (i+1 >= argc)||(argv[i+1][0] == '-')) {

		fprintf(stderr,"You must give a time with the -delay option.\n");
		print_usage();
		exit(-1);
		}
        	else if(sscanf(argv[i+1], "%ld", &UpdateDelay) != 1) {

		fprintf(stderr,"Dont understand the delay time you have entered (%s).\n", argv[i+1]);
		print_usage();
		exit(-1);

	    } 
	    /*
	     *  Convert Time to seconds
	     */
	    UpdateDelay *= 60;
	    ++i;
	    
        }
	 else if (!strcmp(argv[i], "-l")) {

	    if( (i+1 >= argc)||(argv[i+1][0] == '-')) {

		fprintf(stderr,"You must give a station label name.\n");
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


    void print_usage(){

    printf("\nwmfrog version: %s\n", VERSION);
    printf("\nusage: wmfrog -s <StationID> [-h] [-w <max wind>] [-t <time offset>] [-tmp <temp folder>] [-l <label>]\n");
    printf("                   [-m] [-delay <Time in Minutes>]\n\n");
    printf("\n\t-station <METAR StationID>\n");
    printf("\t-s       <METAR StationID>\tThe 4-character METAR Station ID.\n\n");
    printf("\t-metric\n");
    printf("\t-m\t\t\t\tDisplay Temperature in metric unit: (Celcius) \n\n");
    printf("\t-delay <Time in Minutes>\tOverride time (in minutes) between updates\n");
    printf("\t\t\t\t\tdefault is %ld minutes. \n\n", DEFAULT_UPDATEDELAY/60);

    printf("\t-w\t\t\t\tSet the maximum wind in KNOTS (for percentage of max / scaling.)\n");
    printf("\t\t\t\t\tdefault is %d minutes). (Times are approximate.)\n\n", MAX_WIND);
    printf("\t-o\t\t\t\tTime offset of location from UTC (7 for calif in summer) \n\n");
    printf("\t-tmp\t\t\t\tSet the temporary folder (default is: %s) \n\n",folder);
    printf("\t-l\t\t\t\tSet a label to replace the station ID \n\n");

    printf("\n\nTo find out more about the METAR/TAF system, look at:\n");
    printf("	 http://www.nws.noaa.gov/oso/oso1/oso12/metar.htm \n\n");
    printf("To find your city Metar code go to  NOAA's ""Meteorological Station Information\nLookup"" page at:\n\n");
    printf("	 http://www.nws.noaa.gov/oso/siteloc.shtml\n\n");
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

        day = nd + UT/24.0;


        if ((nm == 1) || (nm == 2)){
                ny = ny - 1;
                nm = nm + 12;
        }

        if (((double)ny+nm/12.0+day/365.25)>=(1582.0+10.0/12.0+15.0/365.25)){
                        A = ((int)(ny / 100.0));
                        B = 2.0 - A + (int)(A/4.0);
        }
        else{
                        B = 0.0;
        }
        if (ny < 0.0){
                C = (int)((365.25*(double)ny) - 0.75);
        }
        else{
                C = (int)(365.25*(double)ny);
        }

        D = (int)(30.6001*(double)(nm+1));


        JD = B + C + D + day + 1720994.5;
        return(JD);

}



/*
 *  This routine handles button presses. 
 *
 *	- Left Mouse single click toggles Deg F/C for temperatures.
 *	- Some other click event should display the full METAR report -- lots of
 *        juicy stuff in there... Should bring up a separate window...
 *
 *
 */
void ButtonPressEvent(XButtonEvent *xev){

    /*
     *  Process single clicks.
     */
    DblClkDelay = 0;
    if ((xev->button == Button1) && (xev->type == ButtonPress)){

	if (GotFirstClick1) GotDoubleClick1 = 1;
	else GotFirstClick1 = 1;

    } else if ((xev->button == Button2) && (xev->type == ButtonPress)){ 

	if (GotFirstClick2) GotDoubleClick2 = 1;
	else GotFirstClick2 = 1;

    } else if ((xev->button == Button3) && (xev->type == ButtonPress)){

	if (GotFirstClick3) GotDoubleClick3 = 1;
	else GotFirstClick3 = 1;

    }




    /*
     *  We got a double click on Mouse Button1 (i.e. the left one)
     */
    if (GotDoubleClick1) {
	GotFirstClick1 = 0;
	GotDoubleClick1 = 0;
	// change from metric to us
	Metric = !Metric;
	ForceUpdate = 1;
    }

   return;


}




/*
 *  This routine handles key presses.
 *
 */
void KeyPressEvent(XKeyEvent *xev){

   return;

}

char *GetTempDir(char *suffix) 
{
	uid_t id;
	struct passwd *userEntry;
	char * userHome;
	
	id=getuid();
	userEntry=getpwuid(id);
	userHome=userEntry->pw_dir;
	sprintf(userHome,"%s/%s",userHome,suffix);
	return userHome;
}

char *StringToUpper(char *String) {

    int    i;

    for (i = 0; i < strlen(String); i++)
	String[i] = toupper(String[i]);

    return String;
}

char *mystrsep(char **stringp, char *delim)
{
	char *start = *stringp;
	char *cp;
	char ch;     
	if (start == NULL)
		return NULL; 
	for (cp = start; (ch = *cp)!=0; cp++) 
	{
		if (strchr(delim, ch)) 
		{
			*cp++ = 0;
			*stringp = cp;
			return start;
		}
	}
	*stringp = NULL;
	return start;


}
