#ifndef UTILS_FILES_H
#define UTILS_FILES_H

#include "utils_printlog.h"

#include <iostream>
#include <unistd.h>
#include <pwd.h>

#include <sys/stat.h>
#include <sys/types.h>

class RerobAppDataLog
{
protected:
    long _size;

public:
    virtual void saveHeader(FILE *file){}
    virtual void setSize(long size) {}
    virtual void saveLine(FILE *file, long idx) {}
    void saveToFileName(const char *fileName, long Tlimit);
};

#endif