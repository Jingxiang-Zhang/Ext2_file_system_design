/*
���̻���

*/
#include "Disk.h"

class DiskB {
private:
	DiskPhysical DP;
public:
	DiskB(){}
	char* read(unsigned int block_num);  //�����̿��һ������
	void write(unsigned int block_num, char* content);//дһ���̿飬����
	unsigned int findEmpty();  //ע�����ﷵ�صĿ����32λ��ÿ��4kb��������16Tb�ռ�
	void addToOccupy(unsigned int BlockNumber);  //��һ���̿�����ռ��
	void delFromOccupy(unsigned int BlockNumber); //ɾ��һ���̿�ռ��
	unsigned int getLastDiskNum();
	DiskInformation getDiskInfo(int diskNum);

};

DiskInformation DiskB::getDiskInfo(int diskNum) {
	return DP.getDiskInfo(diskNum);
}


char* DiskB::read(unsigned int block_num) {
	return DP.ReadDisk(block_num);
}

void DiskB::write(unsigned int block_num, char* content) {
	DP.WriteDisk(block_num, content);
}

unsigned int DiskB::findEmpty() {
	unsigned int emp = DP.SearchEmpty();
	if (emp == 0) {
		DP.CreateDisk();
		emp = DP.SearchEmpty();
	}
	DP.AddToOccupy(emp);
	return emp;
}
void DiskB::addToOccupy(unsigned int BlockNumber) {
	DP.AddToOccupy(BlockNumber);
}
void DiskB::delFromOccupy(unsigned int BlockNumber) {
	DP.DelFromOccupy(BlockNumber);

}
unsigned int DiskB::getLastDiskNum() {
	return DP.GetLastDiskNum();
}