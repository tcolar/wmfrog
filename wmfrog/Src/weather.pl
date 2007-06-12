#!/usr/bin/perl
#Thibaut Colar: http://www.colar.net/ 
#2002/05

#The Weather data can be retrieved through http of ftp.
#Http is faster, however this might get broken if NOAA change their webpage layout
#in wich case you should choose ftp.
$mode="http"; # html || ftp


########################################
# Start:
########################################
($station,$tmpfolder)=@ARGV;

mkdir($tmpfolder); 

$html="http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=${station}";
$ftp="ftp://weather.noaa.gov/data/observations/metar/stations/${station}.TXT";
$url=$html;
if($mode eq "ftp")
{
    $url=$ftp;
}

# change this line if wget isn't in your path:
`wget -q -O ${tmpfolder}/${station}_dat $url`;
$line="";
$file="${tmpfolder}/${station}_dat";
open(DATA,"<$file");
$allGood=0;
if($mode eq "http")
{
    while((! ($line=~m/START BODY OF TEXT HERE/)) && !eof(DATA))
    {
#print ".";
	$line=<DATA>;
    }
    $i=0;
    while($i!=12 && !eof(DATA))
    {
#print "*";
	$line=<DATA>;
	$i++;
    }
}
if(eof(DATA))
{
#print "Unreadable HTML source, no point going any further, stopping\n";
exit(0);
}
$line=<DATA>;
chomp $line;
($crap,$time)=split(/\ /,$line);
($hour,$minute)=split(/:/,$time);
$line="";
if($mode eq "http")
{
    $i=0;
    while($i!=6)
    {
	$line=<DATA>;
	$i++;
    }
}
$newLine=<DATA>;
$line.=$newLine;
$newLine=<DATA>;
$line.=$newLine;
close DATA;

chomp $line;
@args=split(/\ /,$line);

if(debug)
{
	#print "@args\n";
}

$i=0;
$station=@args[$i];
if(($station eq 'METAR') or ($station eq 'SPECI'))
{
	$station=@args[$i+1];
	$i++;
}
$i++;
$i++;#time
$wind=@args[$i];
if($wind eq 'AUTO')
{
	$wind=@args[$i+1];
	$i++;
}
$i++;
$visibility=@args[$i];
$i++;#Visibility
if( ! ($visibility=~m/SM/ || ($visibility eq "CAVOK")) )
{
	#print "Visibility fraction\n";
	$i++; # visibility with fractions, ignoring
}
$next=@args[$i];
if($next=~m/FT/)
{
	#print "Visibility feet\n";
	$i++; # visibility in feet, ignoring
}

$tmp="";
$weather="";
$clouds="";
while((! (@args[$i]=~/\//)) && ($i<@args))
{

    $intensity="";
    $desc="";
    $precip="";
    $obsc="";
    $misc="";
    $j=0;
    $curent=@args[$i];
    $wasCloud="no";

    if($curent=~/^CAVOK/)
       {
	   $clouds.="CAVOK,0;";
	   $wasCloud="yes";
       }
    if($curent=~/^VV/)
    {
	$clouds.="VV,".substr($curent,2,3).";";
	$wasNotCloud="yes";
    }
    if(($curent=~/^CLR/) or ($curent=~/^SCK/) or ($curent=~/^FEW/) or ($curent=~/^SCT/)  or ($curent=~/^BKN/)   or ($curent=~/^OVC/))
       {
	   $clouds.=substr($curent,0,3).",".substr($curent,3,3).";";	   
	   $wasCloud="yes";
       }

if(($wasCloud ne "yes"))
{
	if(length($curent)>0)
	{
# Should be weather
    if($curent=~/^\-/)
    {
	$intensity="-";
	$j=1;
    }
    if($curent=~/^\+/)
    {
	$intensity="+";
        $j=1;	
    }
    if($curent=~/^VC/)
    {
	$j=2;
    }
    $curent=substr($curent,$j);
    if(($curent=~/^MI/) or ($curent=~/^PR/) or ($curent=~/^BC/) or ($curent=~/^DR/) or ($curent=~/^BL/) or ($curent=~/^SH/) or ($curent=~/^TS/) or ($curent=~/^FZ/) )
    {
	$desc=substr($curent,0,2);
	$curent=substr($curent,2);
    }
    if(($curent=~/^DZ/) or ($curent=~/^RA/) or ($curent=~/^SN/) or ($curent=~/^SG/) or ($curent=~/^IC/) or ($curent=~/^PE/) or ($curent=~/^GR/) or ($curent=~/^GX/) or ($curent=~/^UP/))
    {
	$precip=substr($curent,0,2);
	$curent=substr($curent,2);
    }
    if(($curent=~/^BR/) or ($curent=~/^FG/) or ($curent=~/^FU/) or ($curent=~/^VA/) or ($curent=~/^DU/) or ($curent=~/^SA/) or ($curent=~/^HZ/) or ($curent=~/^PY/))
    {
	$obsc=substr($curent,0,2);
	$curent=substr($curent,2);
    }
    if(($curent=~/^PO/) or ($curent=~/^SQ/) or ($curent=~/^FC/) or ($curent=~/^SS/))
    {
	$misc=substr($curent,0,2);
    }
    $weather.="$intensity,$desc,$precip,$obsc,$misc;";
    }

}    
    $i++;
}
($temp,$dew)=split(/\//,@args[$i]);


$dir=substr($wind,0,3);
$speed=substr($wind,3,2);
if($wind=~/G/ )
{
    $gust=substr($wind,6,2);
    $windUnit=substr($wind,8,length($wind))
}
else
{
    $windUnit=substr($wind,5,length($wind));
}
if(length($weather)==0)
{
    $weather=",,,,;";
}
if(length($clouds)==0)
{
    $clouds=",,,,;";
}

$temp=~s/M/-/g;
$dew=~s/M/-/g;


if($hour eq "")
{
	$hour=0;
}
if($minute eq "")
{
	$minute=0;
}
if($speed eq "")
{
	$speed=0;
}
if($dir eq "")
{
	$dir=-1;
}
if($gust eq "")
{
	$gust=0;
}
if($temp eq "")
{
	$temp=0;
	$station="";
}
if($dew eq "")
{
	$dew="-999";#makes humidity 0
	$station="";
}

#check for ok temperature
open(GREP, "grep Temp $tmpfolder/${station} | ");
$templine=<GREP>;
$templine=~m/^Temp:(.*)$/;
{
	$lattemp=$1;
	if($temp>-50 && $temp<150)
	{
		if($lasttemp>-50 && $lasttemp<150)
		{
			if($temp>-2 && $temp<2)
			{
				if($lasttemp>7 || $lasttemp<-7)
				{
					#probably invalid
					$station="";
				}
			}
		}
	}
	else
	{
	#invalid
	$station="";
	}
}
close GREP;

#print "$$station\n";
if(length($station)==4)
{
#print "OK\n";
open(FILE,"> $tmpfolder/${station}");
print FILE "Hour:$hour\n";
print FILE "Minute:$minute\n";
print FILE "Station:$station\n";
print FILE "WindDir:$dir\n";
print FILE "WindSpeed:$speed\n";
print FILE "WindGust:$gust\n";
print FILE "Weather:$weather\n";
print FILE "Clouds:$clouds\n";
print FILE "Temp:$temp\n";
print FILE "Dew:$dew\n";
close FILE
}
else
{
#print "Invalid data, not writing to file, stopping\n";
}

