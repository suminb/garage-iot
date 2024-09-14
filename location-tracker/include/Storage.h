#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <FS.h>
#include <SD_MMC.h>

void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

void createDir(fs::FS &fs, const char *path);

void removeDir(fs::FS &fs, const char *path);

void readFile(fs::FS &fs, const char *path);

void writeFile(fs::FS &fs, const char *path, const char *message);

void appendFile(fs::FS &fs, const char *path, const char *message);

void renameFile(fs::FS &fs, const char *path1, const char *path2);

void deleteFile(fs::FS &fs, const char *path);

void init_sd();

#endif // __STORAGE_H__