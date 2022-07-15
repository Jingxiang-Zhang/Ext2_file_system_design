---
lng_pair: UN3_OS
title: Ext2 File System Design
author: Jingxiang Zhang
category: Course Project
tags: [C Language, C++]
img: ":UN_3_OS/ext2.png"
date: 2019-12-30 00:00:00
---

### Introduction

After learning operation system course, the file management system intrigue me so much. Therefore, I decide to spend a few weeks to design and program this file system. I hope readers could have a better understanding of file system after read my code. Due to the complexity of the file system, <!-- outline-start -->This project is programmed by C++. I design a seven layer architecture to realize all the functions<!-- outline-end --> Here is the architecture diagram

![architecture ](:UN_3_OS/ext2.png){:data-align="center"}

### Architectural Analysis

#### 1. Physical Layer

In this layer, all the data store in Windows file system as files. Because all of this is based on Windows OS, I must follow it rules. Therefore, it just like a virtual disk. In order to achieve a powerful file system, my file system can be dynamically changed in length. Instead of using memory, I use disk to store the data.

![architecture ](:UN_3_OS/physical_disk.png){:data-align="center"}

Each large disk block occupies 16MB (16 x 1024=16384) of space. If the memory space exceeds the threshold, a large disk block is dynamically created (as shown in the following figure). Those 16MB of space contain totally 4096 small blocks (each small block is 4KB), and those small block is the basic space to store data.

In a 16MB block, the first small block contains the basic information about the disk (such as creation time). The second block contains the disk block usage (bitmap table), and only the first 500 bytes (4000 bits) in the second small block is in used, because 4096 stand for all available small blocks in one large 16MB block. If the large 16MB block is an integer multiple of 4000, let's say, 0000000 or 0004000, then the third small block in this large block indicates the usage of the 4000 large disks (bitmap table). And similarly, the first 512bytes is in used. When a large disk block is full, the system will create a extra large block to enlarge virtual disk space. 

Here is the diagram of this structure

![virtual disk structure](:UN_3_OS/physical_structure.png){:data-align="center"}

#### 2. Disk Driver Layer

This layer used to operate (CRUD, which include Create, Read, Update, Delete) the virtual disk. It provides a interface to operate disk by block number, and can auto enlarge the virtual disk. Some code segment of this class:

```c++
class DiskPhysical {
private:
	DiskInformation __CreateDiskInformation(); 	// return disk information
	int __FindEmptyFromMap(char* DiskMapBlock);  	// find a empty small disk block by bitmap
	char* __GetDiskName(int number); 		// transform block number into real block path
	int __SearchNotExistName();  			// find a name that not exist in the disk block
public:
	DiskPhysical() {}
	char* ReadDisk(unsigned int block_num);  	// read virtual disk by block number
	void WriteDisk(unsigned int block_num, char* content);	// overwrite a block (overwrite)
	void CreateDisk();          			// create a new disk block
	unsigned int SearchEmpty();  			// search a empty disk block
	void AddToOccupy(unsigned int BlockNumber);  	// set occupy flag into bitmap
	void DelFromOccupy(unsigned int BlockNumber);//delete a occupy flag
	unsigned int GetLastDiskNum();
	DiskInformation getDiskInfo(int diskNum);
};
```

#### 3. Buffer Layer

This layer is a buffer to reduce the frequency of disk access. It has the same function as disk driver layer, but it will cache the data. When access the same disk block, disk buffer will use the cache data, instead of read the disk.

#### 4. FCB Layer

The structure of ext2 file system. It record inode and FCB information, and it provides interface to read FCD.

Here is the data structure of ext2 inode

```c++
struct ext2_inode_physical { 		// each inode occupy 30 int size (120 bytes)
	unsigned int i_ABMS;      	// this 32 bits is:
	//0~2 bits: file mode. 0 is empty, 1 is directory, 1 is txt file 
	//3~11bits: authority. same as linux
	unsigned int i_uid;      	// file user id
	unsigned int i_size;      	// file size (only support maximum 4GB for a single file)
	unsigned int i_ctime;     	// create time
	unsigned int i_mtime;     	// change time 
	unsigned int blocks;	// the amount of iblock used
	char i_name[44];
	// filename, maximum for 38 bytes
	// 40 byte: whether last node is son or brother
	// 41 byte: for the last son or brother
	// 42 byte: for inter son pointer
	// 43 byte: for inter brother pointer
	unsigned int i_block[10];  	// data pointer, point to the data block
	unsigned int i_son_outer;   	// child inode location
	unsigned int i_brother_outer;	// brother inode location 
	unsigned int i_last_outer;   	// link back to it parent or right brother
};
```

Each inode need 120 bytes, and each inode use left-child right-sibling tree. Because each small block is 4096 bytes, it only have 30 FCB information. Each time FCB numbers greate than integer multiple of 30, it will needs a new small block to contain FCB. Here is the diagram of how FCB contain physical disk block. There is a root FCB information in the first disk block, it has the first (root) FCB block location. Through this, it will finds the first FCB in disk block, and all the file in the system can be found one by one by left-child right-sibling tree.

![FCB top layer](:UN_3_OS/FCB_top_layer.png){:data-align="center"}

This is the inode structure. It has two type of pointer, one is the inside pointer, the other is outside pointer. For FCB inside the 4096 bytes block, the inside pointer will be in used, otherwise the outside pointer will be in used. Note that those 30 FCB in one block must be in one directory, in order to guarantee the correction.

![FCB link](:UN_3_OS/FCB_link.png){:data-align="center"}

For the data block of the file, please view the diagram below.

![data block in inode](:UN_3_OS/data_block.png){:data-align="center"}

#### 5. File Driver Layer

In this layer, it provide the interface that can operate the file by file name.

Some public function name of this class:

```c++
class FileStream {
public:
	FileStream(int userid);
	FileStream();
	fileInfo getRoot();
	
	int formatDisk();
	int getBatchSon(fileInfo* info, int number);
	int getBatchSon(int num, int offset, fileInfo* info, int length);
	void getPWD();
	int cdDirectory(const char* name);
	void setUid(int uid);
	int getUid();
	void depthDeduce();
	void depthToOne();
	void getNodeByName(const char* name, unsigned int* num, unsigned int* offset, i_FileMode mode);
	void showopen();

	int mk(char* name, int aut,i_FileMode mode);
	void openFile(char* name, fileOpenMode openMode);
	void closeFile(char* name);
	void writeFile(char* name, char* content, int length);
	char* readFile(char* name,int *length);
	void flush();
	void seekg(int pos, seek_mode mode);
	void seekp(int pos, seek_mode mode);
	void read(char *content, int length);

	void delFile(char* name);
	void moveFile(char* From, char* To);
};
```

#### 6. OS Layer

This layer provide the interface that can read and execute user's command. Some basic command like "ls", "makir" can be execute here.

Some code segment of this class:

```c++
class Command {
private:
	FileStream filestream;
	void __changeColor(i_FileMode mode);
	int uid;
public:
	Command();
	int login();
	int execute(char *command);
	void showPWD();
	int list(const char* method);
	int mkdir(char *name, const char *authority);
	int cd(const char *name);
	int del(const char* name);
	int open(const char*mode, const char* name);
	int write(const char* name, const char* content);
	int close(const char* name);
	int read(const char* name);
	int showopen();
};
```

#### 7. Top Layer

Main function layer, waiting for user's input, and invoke the command execute function.

### Project Show

Login the system (uid and pwd is root)

![login](:UN_3_OS/login.png){:data-align="center"}

Initiate the system (create empty virtual disk)

![initiation](:UN_3_OS/init.png){:data-align="center"}

Execute some basic command

![command 1](:UN_3_OS/command.png){:data-align="center"}

To download this project, please click [here](https://github.com/Jingxiang-Zhang/Vivado_MIPS_CPU).
