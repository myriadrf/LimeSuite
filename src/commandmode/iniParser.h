/**
@file	iniParser.h
@author	Lime Microsystems
@brief	Parser for saving and loading ini files
*/
#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <string.h>
using namespace std;

class iniParser
{
public:
    iniParser()
    {
        m_currentFilename = "";
        m_currentSectionName = "";
    }
    ~iniParser(){};

    bool Open(const char* filename)
    {
        const int bufSize = 1024;
        ifstream fin;
        fin.open(filename, ios::in);
        m_currentFilename = filename;

        char cline[bufSize];
        memset(cline, 0, bufSize);
        char *poss = 0;
        char *pose = 0;

        char key[bufSize/2];
        char value[bufSize/2];

        bool sectionDetected = false;

        while(!fin.eof() && fin.good())
        {
            if(!sectionDetected)
            {
                memset(cline, 0, bufSize);
                fin.getline(cline, bufSize-1);
                sectionDetected = false;
            }
            poss = strstr(cline, "[");
            pose = strstr(cline, "]");
            if( poss != NULL && pose != NULL) //section detected
            {
                sectionDetected = false;
                char secName[bufSize];
                memset(secName, 0, bufSize);
                strncpy(secName, poss+1, pose-poss-1);
                SelectSection(secName);
                while(!fin.eof())
                {
                    memset(cline, 0, bufSize);
                    memset(key, 0, bufSize/2);
                    memset(value, 0, bufSize/2);
                    fin.getline(cline, bufSize-1);
                    if( strlen(cline) == 0)
                        continue; //skip empty lines

                    if( strstr(cline, "[") != NULL )
                    {
                        sectionDetected = true;
                        break; // next section detected
                    }

                    poss = cline;
                    pose = strstr(cline, "=");
                    if(pose == NULL)
                        continue; //no value pair found

                    //copy key value
                    memset(key, 0, bufSize/2);
                    strncpy(key, poss, pose-poss);

                    poss = pose+1;
                    if(strlen(poss) == 0)
                        continue; //only key found, but no value

                    strncpy(value, poss, strlen(poss));
                    m_currentSection[key] = value;
                }
            }
        }
        fin.close();
        return true;
    }

    bool Save(const char* filename = "")
    {
        if(m_currentSection.size() > 0)
        {
            sections[m_currentSectionName] = m_currentSection;
        }
        ofstream fout;
        if( strlen(filename) > 0)
            fout.open(filename, ios::out | ios::binary);
        else
            fout.open(m_currentFilename.c_str(), ios::out | ios::binary);

        map<string , map<string, string> >::iterator iterSections;
        for(iterSections = sections.begin(); iterSections != sections.end(); ++iterSections)
        {
            fout << "[" << iterSections->first << "]" << endl;
            map<string , string >::iterator iterPairs;
            for(iterPairs = iterSections->second.begin(); iterPairs != iterSections->second.end(); ++iterPairs)
            {
                fout << iterPairs->first << "=" << iterPairs->second << "\n";
            }
        }
        fout.close();
        return true;
    }

    bool SelectSection(const char* secName)
    {
        if(m_currentSection.size() > 0)
        {
            sections[m_currentSectionName] = m_currentSection;
        }
        m_currentSectionName = secName;
        m_currentSection.clear();
        m_currentSection = sections[secName];
        return true;
    }

    template <typename dataType>
    dataType Get(const char* variable, const dataType &defValue)
    {
        string retVal = "";
        try
        {
            retVal = m_currentSection.at(variable);
        }
        catch(const std::out_of_range &oor)
        {
            return defValue;
        }
        stringstream ss;
        ss.str(retVal);
        dataType valOut = defValue;
        ss >> valOut;
        return valOut;
    }


    string Get(const char* variable, const char* defValue)
    {
        string retVal = "";
        try
        {
            retVal = m_currentSection.at(variable);
        }
        catch(const std::out_of_range &oor)
        {
            return defValue;
        }
        return retVal;
    }

    string Get(const char* variable, const string &defValue)
    {
        string retVal = "";
        try
        {
            retVal = m_currentSection.at(variable);
        }
        catch(const std::out_of_range &oor)
        {
            return defValue;
        }
        return retVal;
    }

    map<string, string> GetCurrentSection()
    {
        return m_currentSection;
    }

    void Set(const char* variable, const string &value);
    template <typename dataType>
    void Set(const char* variable, const dataType &value)
    {
        stringstream ss;
        ss << value;
        m_currentSection[variable] = ss.str();
    }

    void Set(const char* variable, const char *value)
    {
        stringstream ss;
        ss.str(value);
        m_currentSection[variable] = ss.str();
    }

protected:
    string m_currentFilename;
    string m_currentSectionName;
    map<string, string> m_currentSection;
    map<string, map<string, string> > sections;
};

#endif


