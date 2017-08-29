#include "my_disk_header.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <math.h>

using namespace std;

dir_location Dir;		   //this structure is used for storing sector number and offset from directoy of that sector.
sector_status status;      //this structure stores information about status of sector if its free or not
typecast filentry;


DiskHandle::DiskHandle(int dSize)
{
/*
    Objective: To assign default values to attributes of disk
    Input Parameters: Disk Size and Sector Size
    Return Value: None
*/
    int sSize=pow(2,9);
    disk_info.size_of_disk   = dSize;
    disk_info.size_of_sector = sSize;
    disk_info.total_sector   = (dSize/sSize);
    disk_info.res_sector     = (disk_info.total_sector*4)/(sSize);
    disk_info.unres_sector   = disk_info.total_sector - disk_info.res_sector;
    disk_info.dir_sec		 = disk_info.res_sector;

    cout<<endl<<"Total number of sectors in "<<dSize/pow(2,20)<<" MB file         : "<<disk_info.total_sector<<endl;

	cout<<"Total number of reserved sectors in "<<dSize/pow(2,20)<<" MB file: "<<disk_info.res_sector<<endl;

}

//-----------------------------------------CREATE DISK--------------------------------------
void DiskHandle::Create_Disk(const char file[]){
/*
    Objective: To initialize the disk and store values A, B,..., J in unreserved sectors
               Reserved sectors store the status of unreserved sectors
               Initially every reserved sector stores 0000
	Input parameters: file[]-name of the disk file
*/

	cout<<"\n\t\t   ======= CREATING DISK =======";
	cout<<"\n\t\t  Please wait this will take some time... \n";

	ofstream outfile;
	outfile.open(file,ios::binary|ios::out);

	char buffer[disk_info.size_of_sector];

    //making status of unreserved sectors free i.e. 0 in reserved sectors
	for (int i=0; i <disk_info.res_sector; i++)
    {
        for(int j=0; j<disk_info.size_of_sector; j++)
        {
            status.value=0;
            buffer[j]=status.str[j%4];
        }
        outfile.write(buffer,disk_info.size_of_sector);
	}

    //initialising directory sectors with all '/'
	for (int i=0; i<2; i++)
    {
        for(int j=0;j<disk_info.size_of_sector; j++)

            buffer[j]='/';
        outfile.write(buffer,disk_info.size_of_sector);
	}

    //initializing unreserved sectors with A,B,....J
    for (int i=0; i<disk_info.unres_sector-2; i++)
    {
        for(int j=0;j<disk_info.size_of_sector; j++)
			buffer[j]=static_cast<char>((i%10+65));
        outfile.write(buffer,disk_info.size_of_sector);
		}

    outfile.close();
	cout<<"\n\t\t======= DISK SUCCESSFULLY CREATED! =======\n\n";
}

//------------------update status of sector when sector is written--------------------------
void DiskHandle::update_status(const char file[],int sect_no)
{
/*
    objective: To update status of unreserved sector from empty to busy
    Input Parameters: name of disk file and sector number
    Return Value: None
*/

     if (sect_no>disk_info.total_sector || sect_no<0)
        {
        cout<<endl<<"This Sector is out of range."<<endl;
        }
    else if (sect_no<disk_info.res_sector)
       {
         cout<<"\n Sector "<<sect_no<<"is Reserved. Disk doesnt store it's status.";
       }
    else
    {
        fstream outfile;
        outfile.open(file,ios::out|ios::binary|ios::in);

        char temp[disk_info.size_of_sector];
        status.value=1;

        //let total_status represent total no. of status in 1 sector
        int total_status;
        total_status = (disk_info.size_of_sector/4);
        Dir.base   = ((sect_no-disk_info.res_sector) / total_status);
        Dir.offset = ((sect_no-disk_info.res_sector) % total_status)*4;

        outfile.seekg(Dir.base*disk_info.size_of_sector);
        outfile.read(temp,disk_info.size_of_sector);


        for(int i=Dir.offset;i<4+Dir.offset;i++)
            temp[i]=status.str[3-(i%4)];

        outfile.seekp(Dir.base*disk_info.size_of_sector);
        outfile.write(temp,disk_info.size_of_sector);

        cout<<"\n Status Updated \n";
        outfile.close();
    }

}

//-----------------------------------check status of sector-------------------------------
int DiskHandle::check_status(const char file[],int sect_no)
{
/*
    objective: To check status of a sector if its free or not
    Input Parameter: name of disk file and sector number
    Return Value: 0 - if status of sector is 0000 i.e. free
                  1 - if status of sector is not 0000 i.e. not free
                  2 - if it is a reserved sector
*/
		fstream infile;
        infile.open(file,ios::out|ios::binary|ios::in);


    if (sect_no<(disk_info.res_sector+2))
       {

         return 2;
       }
    else
    {

        infile.seekg(0,ios::beg);

        char buffer1[disk_info.size_of_sector];

        //let total_status represent total no. of status in 1 sector
        int total_status;
        total_status = (disk_info.size_of_sector/4);
        Dir.base   = ((sect_no-disk_info.res_sector) / total_status);
        infile.seekg(Dir.base);
        infile.read(buffer1,disk_info.size_of_sector);
        Dir.offset = ((sect_no-disk_info.res_sector) % total_status)*4;

        infile.seekg(Dir.base*disk_info.size_of_sector + Dir.offset);

        for(int i=0;i<4;i++)
            {
                status.str[3-i]=buffer1[Dir.offset+i];
            }

        if (status.value==0)
            {
                return 0;       //sector is free

            }
        else if(status.value==1)
        {
            return 1;           //sector is not free
        }
        else{
            return status.value;
        }


    }
       infile.close();
}

//-----------------------------------WRITE SECTOR-----------------------------------
void DiskHandle::write_sector(const char file[],int sect_no,char ch)
{
/*
    Objective: To write in a given sector number and update its status
    Input Parameter: name of disk file and sector number
    Return Value: None
*/
    int check=check_status(file,sect_no);

    fstream outfile;
    outfile.open(file,ios::binary|ios::out|ios::in);

    if(check==1 or check==2)
    {
        cout<<"\n ERRRRRRRRR.......This Sector is Busy. Can't write here.\n";
    }
    else if(check==0)
    {

	    outfile.seekp(sect_no*disk_info.size_of_sector);

	    char buffer[disk_info.size_of_sector];

	    for(int i=0;i<disk_info.size_of_sector;i++)
	        {
	            buffer[i]=ch;
	        }

	    outfile.write(buffer,disk_info.size_of_sector);
	    update_status(file,sect_no);
	}

	outfile.close();
}

//-----------------------------------READ SECTOR-----------------------------------
void DiskHandle::read_sector(const char file[],int sect_no)
{
/*
    Objective: To read a sector and show its contents byte by byte
    Input Parameter: name of disk-file and sector number user want to read from
    Return Value: None
*/

    ifstream infile;
    infile.open(file,ios::binary|ios::in); //opening disk file

    char buffer[disk_info.size_of_sector];

    infile.seekg(sect_no*disk_info.size_of_sector); //taking file pointer to the start of given sector number
	infile.read(buffer,disk_info.size_of_sector);  //reading contents of sector into buffer

    if (sect_no<disk_info.res_sector)
        {

            for (int i=0; i<disk_info.size_of_sector; i++)
                cout<<static_cast<int>(buffer[i]);
        }
    else if(disk_info.res_sector<sect_no<(disk_info.res_sector+2))
        {

            for (int i=0; i<disk_info.size_of_sector; i++)
                cout<<static_cast<char>(buffer[i]);
        }
    else
        {

            for (int i=0; i<disk_info.size_of_sector; i++)
                cout<<static_cast<char>(buffer[i]);
        }
    cout<<endl;
    infile.close();

}

//------------------------------location of status-------------------------
void DiskHandle::location(const char file[],int sect_no)
{
        if (sect_no>disk_info.total_sector || sect_no<0)
            cout<<endl<<" This sector is out of range"<<endl;
        else if (sect_no<(disk_info.res_sector))
            cout<<"\n This is a reserved sector.Its status is not stored. \n";
        else if (disk_info.res_sector < sect_no <(disk_info.res_sector+2))
            cout<<"\n This is a directory sector.Its status is not stored. \n";
        else
        {
            int byte_no,sec_no;
            int total_status = (disk_info.size_of_sector/4);
	        byte_no=((sect_no-disk_info.res_sector) % total_status)*4;
	        sec_no = ((sect_no-disk_info.res_sector) / total_status);
            cout << "status of sector " <<sect_no<<" is stored at sector number "<<sec_no<<" byte number "<<byte_no<<"-"<<(byte_no+3)<<endl;
        }

}

//----------------------------find free sectors for writing file------------------------------------
void DiskHandle::free_sector(const char file[],int sec){
    int count=0,start=disk_info.res_sector+2;

    while(count!=sec){
        int stat=check_status(file,start);
        if(stat==0){
            free_sec[count]=start;
            count++;
        }
        start++;

    }

    free_sec[sec]=1;
}

//-----------------------------------DIRECTORY ENTRY----------------------------------------
void DiskHandle::dir_ent(const char file1[],const char file[],int start,int parent,int size,char i_dir){

    char bufferin[disk_info.size_of_sector];
    char bufferout[disk_info.size_of_sector];
    char bytebuffer[32];
    fstream outfile;
    outfile.open(file1,ios::out|ios::binary|ios::in);


    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++){

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(filentry.file_info.is_dir=='/'){
                    strcpy(filentry.file_info.filename, file);
                    filentry.file_info.startsector    =start;
                    filentry.file_info.parentdirectory=parent;
                    filentry.file_info.filesize       =size;
                    filentry.file_info.is_dir         =i_dir;
                        for(int k=0;k<32;k++)
                        {
                            bufferin[32*j+k]=filentry.filebuffer[k];
                        }
                    outfile.seekp((disk_info.dir_sec+i)*disk_info.size_of_sector);
                    outfile.write(bufferin,disk_info.size_of_sector);
                    outfile.close();
                    return;
                }

            }
                }
cout<<"\n No free space found.\n";
outfile.close();

    }

//---------------------------update status of sectors when file is written---------------------
void DiskHandle::update_status_2(const char file[],int sect_no,int val2){
	fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    char temp[disk_info.size_of_sector];
    status.value=val2;

	 //let total_status represent total no. of status in 1 sector
    int total_status;
    total_status = (disk_info.size_of_sector/4);
    Dir.base   = ((sect_no-disk_info.res_sector) / total_status);
    Dir.offset = ((sect_no-disk_info.res_sector) % total_status)*4;

    outfile.seekg(Dir.base*disk_info.size_of_sector);
    outfile.read(temp,disk_info.size_of_sector);


    for(int i=Dir.offset;i<4+Dir.offset;i++)
        temp[i]=status.str[3-(i%4)];

    outfile.seekp(Dir.base*disk_info.size_of_sector);
    outfile.write(temp,disk_info.size_of_sector);

    outfile.close();

}

//--------------------------------WRITE FILE-----------------------------------------------
void DiskHandle::write_sector_2(const char file[],const char f2[],char ch)
{

    //checking if filename is unique
    char bufferin[disk_info.size_of_sector];
    fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++)
            {

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(strcmp(filentry.file_info.filename , f2)==0)
                    {
                        cout<<"\n FILE NAME ALREADY EXIST!! \n";
                        return;
                    }
            }
    }

    outfile.close();

    //writing file if file name is unique
    int fsize,n_sec;
    char no='N';

    cout<<" Size of file:  ";
    cin>>fsize;
    char buffer[disk_info.size_of_sector];

    n_sec=((fsize-1)/disk_info.size_of_sector) + 1;

    int l_sec= fsize-(disk_info.size_of_sector*(n_sec-1));

    free_sector(file,n_sec);

    dir_ent(file,f2,free_sec[0],0,fsize,no);


    outfile.open(file,ios::binary|ios::out|ios::in);

    for(int i=0;i<n_sec-1;i++){
        outfile.seekp(free_sec[i]*disk_info.size_of_sector);
        for(int j=0;j<disk_info.size_of_sector;j++)
            {
                buffer[j]=ch;
            }

        outfile.write(buffer,disk_info.size_of_sector);
        update_status_2(file,free_sec[i],free_sec[i+1]);
    }

    outfile.seekp(free_sec[n_sec-1]*disk_info.size_of_sector);

    for(int i=0;i<l_sec;i++)
        {
            buffer[i]=ch;
        }
    for(int i=l_sec;i<disk_info.size_of_sector;i++){
            buffer[i]=' ';
    }

    outfile.write(buffer,disk_info.size_of_sector);
    update_status_2(file,free_sec[n_sec-1],free_sec[n_sec]);


	outfile.close();
    cout<<endl<<" File is successfully written!"<<endl;
}

//---------------------------------------READ FILE------------------------------------
void DiskHandle::read_file(const char file[],const char f2[]){
    char bufferin[disk_info.size_of_sector];
    fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++)
            {

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(strcmp(filentry.file_info.filename , f2)==0)
                    {
                        if(filentry.file_info.startsector<0){
                            cout<<"FILE DELETED"<<endl;
                            return;
                        }
                        cout<<"\n FILE FOUND!! \n";
                        int sec_no=filentry.file_info.startsector;
                        while(sec_no!=1){
                            read_sector(file,sec_no);
                            sec_no=check_status(file,sec_no);

                        }


                        outfile.close();
                        cout<<endl<<"FILE READ"<<endl;
                        return;

                    }


            }
    }
    cout<<endl<<"FILE NOT FOUND"<<endl;

    outfile.close();

}

//--------------------------FILE INFORMATION--------------------------
int DiskHandle::get_info(const char file[],const char f2[]){
    char bufferin[disk_info.size_of_sector];
    fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++)
            {

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(strcmp(filentry.file_info.filename , f2)==0)
                { if (filentry.file_info.startsector<0)
                        {cout<<"\n This file is deleted!! \n";
                        return filentry.file_info.startsector;
                        }
                  else
                    {
                        cout<<"\n \t\t FILENAME          : "<<filentry.file_info.filename<<endl;
                        cout<<"\t\t FILE SIZE         : "<<filentry.file_info.filesize<<endl;
                        cout<<"\t\t START SECTOR      : "<<filentry.file_info.startsector<<endl;
                        cout<<"\t\t PARENT DIRECTORY  : "<<filentry.file_info.parentdirectory<<endl;
                        cout<<"\t\t IS IT A DIRECTORY : "<<filentry.file_info.is_dir<<endl;

                        return filentry.file_info.startsector;

                    }
                }

            }
    }
    cout<<endl<<"FILE NOT FOUND"<<endl;
    return 0;

    outfile.close();

}

//------------------------------DELETE FILE----------------------------------------
void DiskHandle::delete_file(const char file[],const char f2[]){
    char bufferin[disk_info.size_of_sector];
    fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++)
            {

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(strcmp(filentry.file_info.filename , f2)==0)
                    {
                        cout<<"\n FILE FOUND!! \n";
                        filentry.file_info.startsector=-(filentry.file_info.startsector);
                        for(int l=0;l<32;l++)
                        {
                            bufferin[32*j+l]=filentry.filebuffer[l];
                        }
                        outfile.seekp((disk_info.dir_sec+i)*disk_info.size_of_sector);
                        outfile.write(bufferin,disk_info.size_of_sector);
                        outfile.close();
                        cout<<endl<<"FILE DELETED"<<endl;
                        return;

                    }


            }
    }
    cout<<endl<<"FILE NOT FOUND"<<endl;

    outfile.close();



}

//-------------------------------DELETE SECTOR---------------------------------
void DiskHandle::delete_sec(const char file[],int sect_no){
	char bufferin[disk_info.size_of_sector];
	int ch=sect_no-(disk_info.res_sector+2);

	fstream f2;
	f2.open(file,ios::in|ios::out|ios::binary);
	f2.seekg(sect_no*disk_info.size_of_sector);
	f2.read(bufferin,disk_info.size_of_sector);

	for(int i=0;i<disk_info.size_of_sector;i++){
		bufferin[i]=static_cast<char>((ch%10+65));
	}

	f2.seekp(sect_no*disk_info.size_of_sector);
	f2.write(bufferin,disk_info.size_of_sector);

	f2.close();
}

//----------------------------------FREE DIRECTORY ENTRY-----------------------------
void DiskHandle::free_dir(const char file[],const char f5[]){
	char bufferin[disk_info.size_of_sector];
	char bytebuffer[32];


	fstream f2;
	f2.open(file,ios::in|ios::out|ios::binary);


	for(int j=0;j<2;j++)
	{
		f2.seekg((disk_info.res_sector+j)*disk_info.size_of_sector);
		f2.read(bufferin,disk_info.size_of_sector);
		for(int k=0;k<16;k++)
		{
			for(int i=0;i<32;i++)
				{
					filentry.filebuffer[i]=bufferin[32*k+i];
				}

			if(strcmp(f5,filentry.file_info.filename)==0)
			{
				for(int i=0;i<32;i++)
					{
						bytebuffer[i]='/';
					}
				f2.seekp((disk_info.res_sector+j+(32*k))*disk_info.size_of_sector);
				f2.write(bytebuffer,32);
				f2.close();
				return;
			}
		}
	}

	f2.close();
}

//-------------------------PERMANENT DELETE FILE--------------------------------------------
void DiskHandle::per_delete_file(const char file[],const char f2[]){
	int chc,sect,sect2;
	int fstat=get_info(file,f2);
	if(fstat==0)
        {
		cout<<endl<<"FILE NOT FOUND. \n";
		return;
        }
	else if(fstat<0)
        {
		sect=sect2=-fstat;
		cout<<endl<<"FILE ALREADY DELETED. PRESS 1 TO PERMANENTLY DELETE THE FILE \n";
		cin>>chc;
		if(chc==1)
            {
			while(sect2!=1)
                {
				delete_sec(file,sect2);
				sect2=check_status(file,sect);
				update_status_2(file,sect,0);
				sect=sect2;

                }
			free_dir(file,f2);
			cout<<"FILE DELETED PERMANENTLY.\n";
			return;
            }
		else
            {
			cout<<"FILE NOT DELETED PERMANENTLY.\n";
			return;
            }
        }
	else
        {
		sect=sect2=fstat;
		cout<<endl<<"FILE FOUND! PRESS 1 TO PERMANENTLY DELETE THE FILE ";
		cin>>chc;
		if(chc==1)
			{
				while(sect2!=1)
					{
						delete_sec(file,sect2);
						sect2=check_status(file,sect);
						update_status_2(file,sect,0);
						sect=sect2;
					}
				free_dir(file,f2);
				cout<<"FILE DELETED PERMANENTLY.\n";
				return;
			}
		else{
			cout<<"FILE NOT DELETED PERMANENTLY.\n";
			return;}

        }
}

//-----------------------------------RESTORE FILE------------------------------------
void DiskHandle::file_restore(const char file[],const char f2[]){

    char bufferin[disk_info.size_of_sector];
    fstream outfile;
    outfile.open(file,ios::out|ios::binary|ios::in);

    for(int i=0;i<2;i++)
    {
        outfile.seekg((disk_info.dir_sec+i)*disk_info.size_of_sector);
        outfile.read(bufferin,disk_info.size_of_sector);
        for(int j=0;j<16;j++)
            {

                for(int k=0;k<32;k++)
                {
                    filentry.filebuffer[k]=bufferin[32*j+k];
                }
                if(strcmp(filentry.file_info.filename , f2)==0)
                    {

                        filentry.file_info.startsector=-(filentry.file_info.startsector);
                        for(int l=0;l<32;l++)
                        {
                            bufferin[32*j+l]=filentry.filebuffer[l];
                        }
                        outfile.seekp((disk_info.dir_sec+i)*disk_info.size_of_sector);
                        outfile.write(bufferin,disk_info.size_of_sector);
                        outfile.close();
                        cout<<endl<<" FILE RESTORED"<<endl;
                        return;

                    }
            }
    }
    cout<<endl<<"FILE NOT FOUND"<<endl;

    outfile.close();
}

//==============================================================================================================
//---------------------------------------MAIN-------------------------------------------------------------------
//==============================================================================================================
int main()
{
	/*
	Objective:Creating a Disk and performing disk related functions like
			   Creation of a file of a given size by the user
				having operations read sector wrie sector and read status
	*/


	char data,ch,diskfile[20],f_name[19];
	int choice;
	int disksize=1,sector;
	cout<<"\t\t+-------------------------------------------------------+\n";
    cout<<"\t\t+                                                       +\n";
    cout<<"\t\t+                      DISK MANAGEMENT                  +\n";
    cout<<"\t\t+                                                       +\n";
    cout<<"\t\t+-------------------------------------------------------+\n";
	cout<<endl<<endl;
	cout<<"Enter the name of the Disk file to be created:   ";
	cin>>diskfile;
	ifstream f;
	f.open(diskfile,ios::binary|ios::in);
	bool create_new_file=(f.peek()==ifstream::traits_type::eof());
	f.close();
	if (create_new_file)
	{

		cout<<"\n Creating disk named "<<diskfile<<endl;

		cout<<"Enter 1 for hardcoded disk size else 0 for default :  ";
		cin>>choice;
		cout<<endl;
		if(choice==1)
		{
			cout<<"Enter the size of disk in MB :  ";
			cin>>disksize;
			cout<<endl;
		}
	}


	else{
		cout<<"\n ERRRRRRRRRR.....File already exists...."<<endl<<endl;

		ifstream fp;
		fp.open(diskfile,ios::in|ios::binary);
		fp.seekg(0,ios::end);
		int size=fp.tellg();
		disksize=size/pow(2,20);
		fp.close();
	}

	DiskHandle mydisk(disksize*pow(2,20));

	if (create_new_file)
		mydisk.Create_Disk(diskfile);


	do
	{	cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +                    MENU                       + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +     1.READ SECTOR    ++     2.READ STATUS     + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +     3.WRITE SECTOR   ++      4.LOCATION       + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +     5.WRITE FILE     ++     6. DELETE FILE    + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +     7.FILE INFO      ++      8. READ FILE     + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +           9.PERMANENT DELETE FILE             + ";
        cout<<"\n\t\t +-----------------------------------------------+ ";
        cout<<"\n\t\t +           10.RESTORE DELETED FILE             + ";
        cout<<"\n\t\t +-----------------------------------------------+ \n";


		cout<<"Enter choice :   ";
		cin>>choice;
		cout<<endl;
		switch(choice)
		{	case 1:	cout<<"\n Enter the sector number from which you want to read:   ";
					cin>>sector;
					mydisk.read_sector(diskfile,sector);
					break;
			case 2: {cout<<"\n Enter the sector number for which you want to get the status:   ";
					cin>>sector;
					int chk=mydisk.check_status(diskfile,sector);
					cout<<chk<<endl;
					if(chk==0)
						{cout<<"\n\t\t This Sector is free.\n";}
					else if (chk==1)
						{cout<<"\n\t\t This Sector is busy.\n";}
					else if (chk==2)
						{cout<<"\n\t\t This Sector is reserved or directory sector.\n";}
					else
						{cout<<"\n\t\t STATUS IS  :   "<<chk;}
					break;}
			case 3: cout<<"\n Enter the sector number in which you want to write:  ";
					cin>>sector;
					cout<<"\n Enter the character to be written:  ";
					cin>>data;
					mydisk.write_sector(diskfile,sector,data);
					break;
			case 4: cout<<"\n Enter the sector number for which you want to get the location of status:   ";
					cin>>sector;
					mydisk.location(diskfile,sector);
					break;
			case 5: cout<<"\n Enter name of the file:   ";
                    cin>>f_name;
                    cout<<"\n Enter the character to be written:   ";
					cin>>data;
					mydisk.write_sector_2(diskfile,f_name,data);
                    break;
            case 6: cout<<"\n Enter name of the file:   ";
                    cin>>f_name;
                    mydisk.delete_file(diskfile,f_name);
                    break;
            case 7: cout<<"\n Enter name of the file:   ";
                    cin>>f_name;
                    mydisk.get_info(diskfile,f_name);
                    break;
            case 8: cout<<"\n Enter name of the file:   ";
                    cin>>f_name;
                    mydisk.read_file(diskfile,f_name);
                    break;
            case 9: cout<<"\n Enter name of the file:   ";
                    cin>>f_name;
                    mydisk.per_delete_file(diskfile,f_name);
                    break;
            case 10: cout<<"Enter name of the file to be restored ";
                    cin>>f_name;
                    mydisk.file_restore(diskfile,f_name);
                    break;
			default: cout<<"\n Wrong choice!!! ";
		}

	cout<<"\n\n Do you want to continue(y/n): ";
	cin>>ch;
	cout<<endl;
	}while(ch=='y'||ch=='Y');
}
