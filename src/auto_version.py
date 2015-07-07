__author__ = 'Lime Microsystems'
from string import Template
import datetime

filename = "version.h"

version_file_template = Template(
"""/**
@author Lime Microsystems
@brief  Automatically generated software version
*/
#pragma once
namespace AutoVersion
{
    static const int year = $year;
    static const int month = $month;
    static const int day = $day;
    static const int hour = $hour;
    static const int minutes = $minutes;
    static const int seconds = $seconds;
    static const int buildsCounter = $buildsCounter;
    static const char branchName[] = "$branchName";
}
"""
)

buildCount = 0
branchName = "";
try:
    fin = open(filename, 'r')
    lines = []
    for line in fin:
        lines.append(line)
        if( line.find("static const int buildsCounter") != -1):
            start = line.find("=")
            if(start != -1):
                buildCount = int( line[start+1:line.find(";", start)])+1
        if( line.find("static const char branchName[]") != -1):
            start = line.find("\"")
            if(start != -1):
                branchName = line[start+1:line.find("\"", start+1)]
    fin.close()
except FileNotFoundError:
    print("old version file not found, creating new file")

timestamp = datetime.datetime.now()

f = open('version.h', "w")
f.write(version_file_template.substitute(
    year=timestamp.year,
    month=timestamp.month,
    day=timestamp.day,
    hour=timestamp.hour,
    minutes=timestamp.minute,
    seconds=timestamp.second,
    buildsCounter = buildCount,
    branchName = branchName)
)
f.close()





