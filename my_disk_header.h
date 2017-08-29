#ifndef MY_DISK_HEADER_H_INCLUDED
#define MY_DISK_HEADER_H_INCLUDED

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <math.h>

using namespace std;

struct sector_status{
    union{
            int value;
            char str[4];
            };
};

typedef struct{
        char filename[19];
        int startsector;
        char is_dir;
        int filesize;
        int parentdirectory;
    }fileinfo;

typedef struct{
    union{
        fileinfo file_info;
        char filebuffer[32];
        };
}typecast;




typedef struct{
    int base;		//sector number
	int offset;		//number of bytes apart from base sector number.
}dir_location;

class DiskHandle{
private:
    typedef struct{
        int size_of_disk;
        int size_of_sector;
        int total_sector;
        int res_sector;     //number of reserved sectors
        int unres_sector;   //number of unreserved sectors
        int dir_sec;
    }disk_attributes;

public:
    int free_sec[100];
    disk_attributes disk_info;  //for storing information like disk size,sector size,total sectors etc
    DiskHandle(int dSize);
    void Create_Disk(const char file[]);
    void read_sector(const char file[],int sect_no);
    int check_status(const char file[],int sect_no);
    void location(const char file[],int sect_no);
    void update_status(const char file[],int sect_no);
    void update_status_2(const char file[],int sect_no,int val2);
    void dir_ent(const char file1[],const char file[],int start,int parent,int size,char i_dir);
    void read_file(const char file[],const char f2[]);
    int get_info(const char file[],const char f2[]);
    void delete_file(const char file[],const char f2[]);
    void delete_sec(const char file[],int sect_no);
    void free_dir(const char file[],const char f2[]);
    void per_delete_file(const char file[],const char f2[]);
    void free_sector(const char file[],int sec);
    void write_sector_2(const char file[],const char f2[],char ch);
    void write_sector(const char file[],int sect_no,char ch);
    void file_restore(const char file[],const char f2[]);
};
#endif // MY_DISK_HEADER_H_INCLUDED
