
#include "utils_files.h"

void RerobAppDataLog::saveToFileName(const char *fileName, long Tlimit)
{
    FILE *f1;

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char filePath[150];

    sprintf(filePath, "%s/out", homedir);

    if (mkdir(filePath, 0777) == -1)
    {
        PRINT_LOG(5, PRINT_YELLOW "[WARNING]" PRINT_RESET "Folder Create: %s", filePath);
    }

    sprintf(filePath, "%s/out/%s.dat", homedir, fileName);

    f1 = fopen(filePath, "w+");

    if (f1 != nullptr)
    {
        saveHeader(f1);

        for (long i = 0; i <= Tlimit; i++)
        {
            saveLine(f1, i);
        }

        fclose(f1);
        PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "File Saving: %s", filePath);
    }
    else
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "File Create: %s", filePath);
    }
}
