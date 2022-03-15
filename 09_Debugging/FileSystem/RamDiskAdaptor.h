#pragma once
#include "FileSysAdaptor.h"
#include "MintFileSystem.h"

#pragma pack( push, 1 )

// ��Ű���� �ñ׳�ó
#define PACKAGESIGNATURE    "MINT64OSPACKAGE "

// ���� �̸��� �ִ� ����, Ŀ���� FILESYSTEM_MAXFILENAMELENGTH�� ����
#define MAXFILENAMELENGTH   24

// �Ķ���͸� ó���ϱ����� ������ �����ϴ� �ڷᱸ��
typedef struct kParameterListStruct
{
	// �Ķ���� ������ ��巹��
	const char* pcBuffer;
	// �Ķ������ ����
	int iLength;
	// ���� ó���� �Ķ���Ͱ� �����ϴ� ��ġ
	int iCurrentPosition;
} PARAMETERLIST;

// ��Ű�� ��� ������ �� ���� ������ �����ϴ� �ڷᱸ��
typedef struct PackageItemStruct
{
	// ���� �̸�
	char vcFileName[MAXFILENAMELENGTH];

	// ������ ũ��
	DWORD dwFileLength;
} PACKAGEITEM;

// ��Ű�� ��� �ڷᱸ��
typedef struct PackageHeaderStruct
{
	// MINT64 OS�� ��Ű�� ������ ��Ÿ���� �ñ׳�ó
	char vcSignature[16];

	// ��Ű�� ����� ��ü ũ��
	DWORD dwHeaderSize;
	
} PACKAGEHEADER;

#pragma pack( pop )

class RamDiskAdaptor : public FileSysAdaptor
{
public:
	RamDiskAdaptor(char* deviceName, DWORD deviceID);
	~RamDiskAdaptor();
	
	virtual bool Initialize() override;
	virtual int GetCount() override;
	virtual int Read(PFILE file, unsigned char* buffer, unsigned int size, int count) override;
	virtual bool Close(PFILE file)  override;
	virtual PFILE Open(const char* FileName, const char *mode)  override;
	virtual size_t Write(PFILE file, unsigned char* buffer, unsigned int size, int count) override;

	bool InstallPackage();

private:
	void PrintRamDiskInfo();	
	PACKAGEHEADER* FindPackageSignature(UINT32 startAddress, UINT32 endAddress);

private:
	HDDINFORMATION* m_pInformation;
};

