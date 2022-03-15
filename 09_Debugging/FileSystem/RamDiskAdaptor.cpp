#include "RamDiskAdaptor.h"
#include "SkyConsole.h"
#include "MultiBoot.h"

RamDiskAdaptor::RamDiskAdaptor(char* deviceName, DWORD deviceID)
	: FileSysAdaptor(deviceName, deviceID)
{
	m_pInformation = new HDDINFORMATION;
}

RamDiskAdaptor::~RamDiskAdaptor()
{
	delete m_pInformation;
}

bool RamDiskAdaptor::Initialize()
{
	bool result = kInitializeRDDFileSystem();	

	if (result == true)
	{
		kGetHDDInformation(m_pInformation);

		PrintRamDiskInfo();		

		return true;
	}

	return false;
}

int RamDiskAdaptor::GetCount()
{	
	return 1;
}


void RamDiskAdaptor::PrintRamDiskInfo()
{
	SkyConsole::Print("RamDisk Info\n");
	SkyConsole::Print("Total Sectors : %d\n", m_pInformation->dwTotalSectors);
	SkyConsole::Print("Serial Number : %s\n", m_pInformation->vwSerialNumber);
	SkyConsole::Print("Model Number : %s\n", m_pInformation->vwModelNumber);	 
}

int RamDiskAdaptor::Read(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (file == nullptr)
		return false;

	return kReadFile(buffer, size, count, (MFILE*)file->_id);
}

bool RamDiskAdaptor::Close(PFILE file)
{
	if (file == nullptr)
		return false;

	return (-1 != kCloseFile((MFILE*)file->_id));	
}

PFILE RamDiskAdaptor::Open(const char* fileName, const char *mode)
{
	MFILE* pMintFile = kOpenFile(fileName, mode);

	if (pMintFile)
	{
		PFILE file = new FILE;
		file->_deviceID = 'C';
		strcpy(file->_name, fileName);
		file->_id = (DWORD)pMintFile;
		return file;
	}

	return nullptr;
}

size_t RamDiskAdaptor::Write(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (file == nullptr)
		return 0;
	
	return kWriteFile(buffer, size, count, (MFILE*)file->_id);
}

PACKAGEHEADER* RamDiskAdaptor::FindPackageSignature(UINT32 startAddress, UINT32 endAddress)
{
	// ��ũ �̹����� 0x10000 ��巹���� �ε��ǹǷ� �̸� ��������
	// Ŀ�� ���� ����ŭ ������ ���� ��Ű�� ����� ����
	PACKAGEHEADER* pstHeader = nullptr;
	
	for (UINT32 addr = startAddress; addr < endAddress; addr += 512)
	{
		pstHeader = (PACKAGEHEADER*)(addr);
		// �ñ׳�ó�� Ȯ��
		if (memcmp(pstHeader->vcSignature, PACKAGESIGNATURE, sizeof(pstHeader->vcSignature)) == 0)
		{			
			return pstHeader;
		}				
	}		
	return nullptr;
}

bool RamDiskAdaptor::InstallPackage()
{	
	FILE* fp;
	PACKAGEITEM* pstItem = nullptr;
	UINT32 dwDataAddress = 0;

	SkyConsole::Print("Package Install Start...\n");

	PACKAGEHEADER* pstHeader = FindPackageSignature(KERNEL_LOAD_ADDRESS, KERNEL_END_ADDRESS);

	if(pstHeader == nullptr)
	{
		SkyConsole::Print("Package Signature Fail\n");
		return false;
	}	
	//--------------------------------------------------------------------------
	// ��Ű�� ���� ��� ������ ã�Ƽ� �� ��ũ�� ����
	//--------------------------------------------------------------------------
	// ��Ű�� �����Ͱ� �����ϴ� ��巹��
	dwDataAddress = (UINT32)(((char*)pstHeader) + sizeof(PackageHeaderStruct));		
	// ��Ű�� ����� ù ��° ���� ������
	pstItem = (PACKAGEITEM*)dwDataAddress;

	// ��Ű���� ���Ե� ��� ������ ã�Ƽ� ����
	for (DWORD i = 0; i < pstHeader->dwHeaderSize / sizeof(PACKAGEITEM); i++)
	{
		SkyConsole::Print("[%d] file: %s, size: %d Byte\n", i + 1, pstItem[i].vcFileName, (int)(pstItem[i].dwFileLength));

		// ��Ű���� ���Ե� ���� �̸����� ������ ����
		fp = fopen(pstItem[i].vcFileName, "w");
		if (fp == NULL)
		{
			SkyConsole::Print("%s File Create Fail\n");
			return false;
		}
		
		// ��Ű�� ������ �κп� ���Ե� ���� ������ �� ��ũ�� ����
		if (fwrite((BYTE*)dwDataAddress, 1, pstItem[i].dwFileLength, fp) != pstItem[i].dwFileLength)
		{
			SkyConsole::Print("Ram Disk Write Fail\n");

			// ������ �ݰ� ���� �ý��� ĳ�ø� ������
			fclose(fp);

			return false;
		}		

		// ������ ����        
		fclose(fp);

		// ���� ������ ����� ��ġ�� �̵�
		dwDataAddress += pstItem[i].dwFileLength;
	}

	SkyConsole::Print("Package Install Complete\n");
	
	return true;
}