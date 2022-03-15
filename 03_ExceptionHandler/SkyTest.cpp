#include "SkyTest.h"
#include "SkyConsole.h"
#include "MultiBoot.h"

//���ͷ�Ʈ �ڵ鷯 �׽�Ʈ
void TestDivideByZero();
void TestBreakPoint();
void TestInvalidOpcode();

void TestInterrupt()
{
	TestDivideByZero();
	//TestBreakPoint();
	//TestInvalidOpcode();
}

int _divider = 0;
int _dividend = 100;
void TestDivideByZero()
{	
	int result = _dividend / _divider;

//����ó���� ���� �Ʒ� �ڵ尡 ����ȴٰ� �ص�
//result ����� �������� ���� �ƴϴ�
//���� ������ ����� ��ɾ�ܿ��� ���� ���� ���̸� 
//�߰��������� ������ �κ��� �����ߴ� �ؼ� �������� ����� ����ϴ� ���� ������.

	if(_divider != 0)
		result = _dividend / _divider;

	SkyConsole::Print("Result is %d, divider : %d\n", result, _divider);
}

void funcBreakPoint(void)
{
	__asm {
		align 4		
		__asm _emit 0xcc
		__asm _emit 0x00
		__asm _emit 0x00
		__asm _emit 0x00
	}
}

void TestBreakPoint()
{
	funcBreakPoint();
}

void TestInvalidOpcode()
{
	__asm {
		align 4
		__asm _emit 0xFF
		__asm _emit 0xFF
		__asm _emit 0xFF
		__asm _emit 0xFF
	}
}
//���ͷ�Ʈ �ڵ鷯 �׽�Ʈ ��




