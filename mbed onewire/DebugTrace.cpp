/*
* DebugTrace. Allows dumping debug messages/values to serial or
* to file.
*
* Copyright (C) <2009> Petras Saduikis <petras@petras.co.uk>
*
* This file is part of DebugTrace.
*
* DebugTrace is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* DebugTrace is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with DebugTrace.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DebugTrace.h"
#include <mbed.h>
#include <stdarg.h>
#include <string.h>

Serial logSerial(USBTX, USBRX);
LocalFileSystem local("local");

const char* FILE_PATH = "/local/";
const char* EXTN = ".bak";

DebugTrace::DebugTrace(eLog on, eLogTarget mode, const char* fileName, int maxSize) :
    enabled(on), logMode(mode), maxFileSize(maxSize), currentFileSize(0),
    logFileStatus(0)
{
    // allocate memory for file name strings
    int str_size = (strlen(fileName) + strlen(FILE_PATH) + strlen(EXTN) + 1) * sizeof(char); 
    logFile = (char*)malloc(str_size);
    logFileBackup = (char*)malloc(str_size);
    
    // add path to log file name
    strcpy(logFile, FILE_PATH);
    strcat(logFile, fileName);
    
    // create backup file name
    strcpy(logFileBackup, logFile);
    strcpy(logFileBackup, strtok(logFileBackup, "."));
    strcat(logFileBackup, EXTN);
}

DebugTrace::~DebugTrace()
{
    // dust to dust, ashes to ashes
    if (logFile != NULL) free(logFile);
    if (logFileBackup != NULL) free(logFileBackup);
}

void DebugTrace::clear()
{
    // don't care about whether these fail
    remove(logFile);    
    remove(logFileBackup);
}

void DebugTrace::backupLog()
{
    // delete previous backup file
    if (remove(logFileBackup))
    {
        // standard copy stuff
        char ch;
        FILE* to = fopen(logFileBackup, "wb");
        if (NULL != to)
        {
            FILE* from = fopen(logFile, "rb");   
            if (NULL != from)
            {
                while(!feof(from))
                {
                    ch = fgetc(from);
                    if (ferror(from)) break;
                    
                    if(!feof(from)) fputc(ch, to);
                    if (ferror(to)) break;
                }
            }
            
            if (NULL != from) fclose(from);
            if (NULL != to) fclose(to);
        }
    }

    // now delete the log file, so we are ready to start again
    // even if backup creation failed - the show must go on!
    logFileStatus = remove(logFile);
}

void DebugTrace::traceOut(const char* fmt, ...)
{
    if (enabled)
    {
        va_list ap;            // argument list pointer
        va_start(ap, fmt);
        
        if (TO_SERIAL == logMode)
        {
            vfprintf(logSerial, fmt, ap);
        }
        else    // TO_FILE
        {
            if (0 == logFileStatus)    // otherwise we failed to remove a full log file
            {
                // Write data to file. Note the file size may go over limit
                // as we check total size afterwards, using the size written to file.
                // This is not a big issue, as this mechanism is only here
                // to stop the file growing unchecked. Just remember log file sizes may
                // be some what over (as apposed to some what under), so don't push it 
                // with the max file size.
                FILE* fp = fopen(logFile, "a");
                if (NULL == fp)
                {
                    va_end(ap);
                    return;
                }
                int size_written = vfprintf(fp, fmt, ap);
                fclose(fp);
                
                // check if we are over the max file size
                // if so backup file and start again
                currentFileSize += size_written;
                if (currentFileSize >= maxFileSize)
                {
                    backupLog();
                    currentFileSize = 0;
                }
            }
        }
        
        va_end(ap);
    }
}
