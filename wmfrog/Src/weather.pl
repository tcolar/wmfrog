#!/usr/bin/perl
#
#   Copyright
#
#       Copyright (C) 2002,2010 Thibaut Colar <thibautc@pcf.com>
#
#   License
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   Description
#
#       See http://www.colar.net
#
#       The Weather data can be retrieved through http of ftp. Http is
#       faster, however this might get broken if NOAA change their
#       webpage layout in wich case you should choose ftp.

use Geo::METAR;

$mode="ftp"; # html || ftp

$debug = 0; # turn On/Off debugging

sub debug ($)
{
    my ($str) = @_;

    if ( $debug )
    {
	print "$str\n";
    }
}


########################################
# Start:
########################################

($station, $tmpfolder) = @ARGV;

debug("station: $station ; tmpfolder: $tmpfolder");

mkdir $tmpfolder;

$html = "http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=${station}";
$ftp  = "ftp://weather.noaa.gov/data/observations/metar/stations/${station}.TXT";
$ftp  = "ftp://tgftp.nws.noaa.gov/data/observations/metar/stations/${station}.TXT";

$url  = $html;

if ( $mode eq "ftp" )
{
    $url = $ftp;
}

# Change this line if wget isn't in your path:
system "wget -q -O $tmpfolder/${station}_dat $url";

$line = "";
$file = "$tmpfolder/${station}_dat";

open DATA, "<", $file;
# Not used: $allGood = 0;

if ( $mode eq "http" )
{
    while ( (! ($line =~ m/START BODY OF TEXT HERE/))  and  not eof DATA)
    {
	$line = <DATA>;
    }

    $i = 0;

    while ( $i != 21  and  not eof DATA )
    {
	$line = <DATA>;
	$i++;
    }
}

if ( eof DATA )
{
    debug("Unreadable HTML source, no point going any further, stopping\n");
    exit 0;
}

$line = <DATA>;
chomp $line;

$time = (split /\ /, $line)[1];
($hour, $minute) = split /:/, $time;
$line = "";

if ( $mode eq "http" )
{
    $i = 0;

    while ( $i != 8 )
    {
	$line = <DATA>;
	$i++;
    }
}

$newLine = <DATA>;
$line   .= $newLine;
chomp $line;

close DATA;

debug("line: $line");

my $m = Geo::METAR->new;

$m->metar ($line);

$wind = $m->{wind};

debug("wind: $wind");

$visibility = $m->{visibility};

debug("vis: $visibility");

$weather = "";
$clouds = " ";

if ($m->{sky}[0] eq "CAVOK")
{
    $clouds .= "CAVOK,0;";
}
else
{
    if (!@{$m->{sky}})
    {
        if (my ($vv) = grep { s/^VV(\d+)/$1/ } split /\s+/, $line)
        {
            $clouds .= "VV,$vv;";
        }
    }
    else
    {
        $clouds .= join (",", m/(.{3})(.{3})/) . ";" for @{$m->{sky}};
    }

    for my $curent (@{$m->{weather}})
    {
        $intensity = "";
        $desc      = "";
        $precip    = "";
        $obsc      = "";
        $misc      = "";
        $j         = 0;

        if ( $curent =~ /^\-/ )
        {
            $intensity = "-";
            $j = 1;
        }

        if ( $curent =~ /^\+/ )
        {
            $intensity="+";
            $j = 1;
        }

        if ( $curent =~ /^VC/ )
        {
            $j = 2;
        }

        $curent = substr $curent, $j;

        if ( $curent =~ /^MI/ or
             $curent =~ /^PR/ or
             $curent =~ /^BC/ or
             $curent =~ /^DR/ or
             $curent =~ /^BL/ or
             $curent =~ /^SH/ or
             $curent =~ /^TS/ or
             $curent =~ /^FZ/ )
        {
            $desc   = substr $curent, 0, 2;
            $curent = substr $curent, 2;
        }

        if ( $curent=~/^DZ/ or
             $curent=~/^RA/ or
             $curent=~/^SN/ or
             $curent=~/^SG/ or
             $curent=~/^IC/ or
             $curent=~/^PE/ or
             $curent=~/^GR/ or
             $curent=~/^GX/ or
             $curent=~/^UP/ )
        {
            $precip = substr $curent, 0, 2;
            $curent = substr $curent, 2;
        }

        if ( $curent=~/^BR/ or
             $curent=~/^FG/ or
             $curent=~/^FU/ or
             $curent=~/^VA/ or
             $curent=~/^DU/ or
             $curent=~/^SA/ or
             $curent=~/^HZ/ or
             $curent=~/^PY/ )
        {
            $obsc   = substr $curent, 0, 2;
            $curent = substr $curent, 2;
        }

        if ( $curent=~/^PO/ or
             $curent=~/^SQ/ or
             $curent=~/^FC/ or
             $curent=~/^SS/ )
        {
            $misc = substr $curent, 0, 2;
        }

        $weather .= "$intensity,$desc,$precip,$obsc,$misc;" ;
    }
}

($temp, $dew) = split /\//, $m->{temp_dew};

$dir   = substr $wind, 0, 3;
$speed = substr $wind, 3, 2;

if ( $wind =~ /G/ )
{
    $gust     = substr $wind, 6, 2;
    $windUnit = substr $wind, 8, length $wind;
}
else
{
    $windUnit = substr $wind, 5, length $wind;
}

if ( length $weather == 0 )
{
    $weather = ",,,,;" ;
}

if ( length $clouds == 0 )
{
    $clouds = ",,,,;" ;
}

$temp =~ s/M/-/g;
$dew  =~ s/M/-/g;


if ( $hour eq "" )
{
    $hour = 0;
}

if ( $minute eq "" )
{
    $minute = 0;
}

if ( $speed eq "" )
{
    $speed = 0;
}

if ( $dir eq "" )
{
    $dir = -1;
}

if ( $gust eq "" )
{
    $gust = 0;
}

if ( $temp eq "" )
{
    $temp    = 0;
    $station = "";
}

if ( $dew eq "" )
{
    $dew     = "-999";			# makes humidity 0
    $station = "";
}

debug("Hour:$hour");
debug("Minute:$minute");
debug("Station:$station");
debug("WindDir:$dir");
debug("WindSpeed:$speed");
debug("WindGust:$gust");
debug("Weather:$weather");
debug("Clouds:$clouds");
debug("Temp:$temp");
debug("Dew:$dew");


# Check for ok temperature

if ( -f "$tmpfolder/$station" )
{
    open GREP, "grep Temp $tmpfolder/${station} | ";
    $templine = <GREP>;

    $templine = ~m/^Temp:(.*)$/;

    {
	if ( $temp > -50  and  $temp < 150 )
	{
	    if ( $lasttemp > -50  and  $lasttemp < 150 )
	    {
		if ( $temp > -2  and  $temp < 2 )
		{
		    if ( $lasttemp > 7  or  $lasttemp < -7 )
		    {
			$station = "";	# probably invalid
		    }
		}
	    }
	}
	else
	{
	    $station = "";
	}
    }

    close GREP;
}

debug("Station: $station");

if ( length $station == 4 )
{
    debug("Data OK");

    open FILE, ">", "$tmpfolder/${station}";

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
    debug("Invalid data, not writing to file, stopping\n");
}

# End of file
