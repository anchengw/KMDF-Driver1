#include <ntddk.h>

#define DEVICE_NAME L"\\Device\\link_mydevice"
#define SYMBOL_LINK_NAME L"\\??\\link_mydevice"

#define IOCTL_BASE          0x800
#define MY_CTL_CODE(i)  \
	CTL_CODE            \
	(                   \
	FILE_DEVICE_UNKNOWN,\
	IOCTL_BASE + i,     \
	METHOD_BUFFERED,    \
	FILE_ANY_ACCESS     \
	)

PDEVICE_OBJECT DeviceObj = NULL;
NTKERNELAPI PCHAR PsGetProcessImageFileName(PEPROCESS Process);
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);

//�����豸
PDEVICE_OBJECT	CreateDevice(IN PDRIVER_OBJECT pDriver)
{
	UNICODE_STRING	uDeviceName = { 0 };
	UNICODE_STRING	uSymbolName = { 0 };
	PDEVICE_OBJECT	pDeviceObj = NULL;
	NTSTATUS nStatsus = STATUS_UNSUCCESSFUL;
	RtlInitUnicodeString(&uDeviceName, DEVICE_NAME);            //תΪUnicode�ַ���
	RtlInitUnicodeString(&uSymbolName, SYMBOL_LINK_NAME);
	//�����豸
	nStatsus = IoCreateDevice(
		pDriver, //�����������.
		0,            //ָ����������Ϊ�豸��չ���������Ľṹ��Ĵ�С
		&uDeviceName, //(��ѡ�Ĳ���)ָ��һ�������β�İ���Unicode�ַ����Ļ�����, ��������豸������
		FILE_DEVICE_UNKNOWN, //ָ��һ����һ��ϵͳ�����FILE_DEVICE_XXX����, ����������豸������
		0,                   //һ������ϵͳ����ĳ���, ������һ��, �ṩ�й�����������豸������Ϣ.
		FALSE,               //�豸�Ƕ�ռ��,��ռ�Ļ�����ΪTRUE���Ƕ�ռ����ΪFALSE.һ��FALSE
		&pDeviceObj);
	if (!NT_SUCCESS(nStatsus))
	{
		DbgPrint("�豸����ʧ��");
		return NULL;
	}
	if (pDeviceObj != NULL)
	{
		pDeviceObj->Flags |= DO_BUFFERED_IO;//�����豸�Ի�������ʽ��ȡ
	}
	nStatsus = IoCreateSymbolicLink(&uSymbolName, &uDeviceName);    //�����豸�ķ�������
	if (!NT_SUCCESS(nStatsus))
	{
		IoDeleteDevice(pDeviceObj);
		DbgPrint("�������Ӵ���ʧ��");
		return NULL;
	}
	DbgPrint("�豸�����ɹ�");
	return pDeviceObj;
}
//ɾ���豸
VOID DeleteDevice()
{
	UNICODE_STRING uSymbolName = { 0 };
	RtlInitUnicodeString(&uSymbolName, SYMBOL_LINK_NAME);
	IoDeleteSymbolicLink(&uSymbolName); //ɾ����������
	if (DeviceObj != NULL)
	{
		IoDeleteDevice(DeviceObj);
	}
	DeviceObj = NULL;
}

VOID UnDriver(PDRIVER_OBJECT pDriver)
{
	//PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)NotifyCreateProcess, TRUE);
	DeleteDevice();
	DbgPrint("����ж�����...");
}
// �����ص�����
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	//��ɴ�IRP
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return status;
}
// �رջص�����
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	//�������
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
//���ַ�������,�����ж�R3���͵Ŀ����ź�
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;	 // STATUS_UNSUCCESSFUL
	ULONG information = 0;
	char* retBuffer = "hello lyshark";
	UNICODE_STRING uncode_buffer = { 0 };
	//�ж����󷢸�˭
	if (pDevObj == DeviceObj)
	{
		//ȡ����ĵ�ǰջ�ռ�
		PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(pIrp);
		//ȡ�û�����
		PVOID InputData = pIrp->AssociatedIrp.SystemBuffer;
		PVOID OutputData = pIrp->AssociatedIrp.SystemBuffer;

		//ȡ�������ĳ���
		ULONG inlen = irpsp->Parameters.DeviceIoControl.InputBufferLength;
		//ȡ����������ĳ���
		ULONG outlen = irpsp->Parameters.DeviceIoControl.OutputBufferLength;
		
		switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
		{
		case MY_CTL_CODE(1):
			//�����ڴ�
			//uncode_buffer.Buffer = (PWSTR)ExAllocatePool(PagedPool, inlen);
			//RtlCopyMemory(uncode_buffer.Buffer, (WCHAR*)InputData, uncode_buffer.Length);
			//wcscpy_s(uncode_buffer.Buffer, sizeof(uncode_buffer.Buffer), (WCHAR*)InputData);//ȡ��Ϣ
			DbgPrint((WCHAR*)InputData);
			//DbgPrint("����ַ���: %wZ \n", uncode_buffer.Buffer);
			//�ͷ��ڴ�
			//ExFreePool(uncode_buffer.Buffer);
			//wcscpy_s((WCHAR*)buffer, outlen, L"ok");
			memcpy(OutputData, retBuffer, strlen(retBuffer)); //��Ӧ�ò㴫����Ϣ
			information = strlen(retBuffer) + 1;
			status = STATUS_SUCCESS;
			break;
		default:
			//δ֪������ͳһ���طǷ�
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		//��������
		pIrp->IoStatus.Information = information;     //���ص��������
		pIrp->IoStatus.Status = status;           //�������״̬
		IoCompleteRequest(pIrp, IO_NO_INCREMENT); //��������
		return status;
	}
	else
	{
		pIrp->IoStatus.Information = 0;
		// Complete the I/O Request
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return status;
	}
	RtlFreeUnicodeString(&uncode_buffer);
}
/// <summary>
/// ��ں����൱��main ��ϵͳ����System����һ��
/// </summary>
/// <param name="driver"></param>
/// <param name="reg_path"></param>
/// <returns></returns>
// ��ں���
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING RegistryPath)
{

	//�����豸
	DeviceObj = CreateDevice(pDriver);
	pDriver->DriverUnload = UnDriver;                               // ж�غ���
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;         // ������ǲ����
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;           // �ر���ǲ����
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;  // �ַ�����
	//status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)NotifyCreateProcess, FALSE);
	DbgPrint(L"�������");
	return STATUS_SUCCESS;
}
PCHAR GetProcessNameByProcessId(HANDLE ProcessId)
{
	NTSTATUS st = STATUS_UNSUCCESSFUL;
	PEPROCESS ProcessObj = NULL;
	PCHAR string = NULL;
	st = PsLookupProcessByProcessId(ProcessId, &ProcessObj);
	if (NT_SUCCESS(st))
	{
		string = PsGetProcessImageFileName(ProcessObj);
		ObfDereferenceObject(ProcessObj);
	}
	return string;
}

VOID
NotifyCreateProcess(
	__inout PEPROCESS Process,//����Ǵ���(�˳�),���Ǳ�����(�˳�)���̵�exe��(����������·��)
	__in HANDLE ProcessId,//����Ǵ���(�˳�)����,���Ǳ�����(�˳�)���̵�pid
	__in_opt PPS_CREATE_NOTIFY_INFO CreateInfo//����Ǵ�������,�����������������������·����
)
{

	if (CreateInfo)
	{
		//    DbgPrint("param ProcessId is %d\n", ProcessId); //����������id
		//    DbgPrint("param Process is %s\n", PsGetProcessImageFileName(Process));
		DbgPrint("%s of who the pid is %d create process %wZ\n",
			GetProcessNameByProcessId(CreateInfo->ParentProcessId),
			CreateInfo->ParentProcessId,
			CreateInfo->ImageFileName);
		if (_stricmp("calc.exe", PsGetProcessImageFileName(Process)) == 0)
		{
			DbgPrint("forbidding start calc.exe!\n");
			CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
	else
	{
		DbgPrint("process %s exit\n", PsGetProcessImageFileName(Process));
	}
}