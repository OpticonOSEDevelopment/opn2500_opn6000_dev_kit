/*
 * FileSystem.h
 *  
 *      Author: Ronny de Winter
 */
#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

bool fatfs_init(void);

void fatfs_mkfs(void);

void fatfs_uninit(void);

int fatfs_isbusy(void);

size_t fsize(const char *FileName);

#endif // FILESYSTEM_H__