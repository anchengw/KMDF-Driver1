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

//创建设备
PDEVICE_OBJECT	CreateDevice(IN PDRIVER_OBJECT pDriver)
{
	UNICODE_STRING	uDeviceName = { 0 };
	UNICODE_STRING	uSymbolName = { 0 };
	PDEVICE_OBJECT	pDeviceObj = NULL;
	NTSTATUS nStatsus = STATUS_UNSUCCESSFUL;
	RtlInitUnicodeString(&uDeviceName, DEVICE_NAME);            //转为Unicode字符串
	RtlInitUnicodeString(&uSymbolName, SYMBOL_LINK_NAME);
	//创建设备
	nStatsus = IoCreateDevice(
		pDriver, //驱动程序对象.
		0,            //指定驱动程序为设备扩展对象而定义的结构体的大小
		&uDeviceName, //(可选的参数)指向一个以零结尾的包含Unicode字符串的缓冲区, 那是这个设备的名称
		FILE_DEVICE_UNKNOWN, //指定一个由一个系统定义的FILE_DEVICE_XXX常量, 表明了这个设备的类型
		0,                   //一个或多个系统定义的常量, 连接在一起, 提供有关驱动程序的设备其他信息.
		FALSE,               //设备是独占的,独占的话设置为TRUE，非独占设置为FALSE.一般FALSE
		&pDeviceObj);
	if (!NT_SUCCESS(nStatsus))
	{
		DbgPrint("设备创建失败");
		return NULL;
	}
	if (pDeviceObj != NULL)
	{
		pDeviceObj->Flags |= DO_BUFFERED_IO;//设置设备以缓冲区方式读取
	}
	nStatsus = IoCreateSymbolicLink(&uSymbolName, &uDeviceName);    //创建设备的符号链接
	if (!NT_SUCCESS(nStatsus))
	{
		IoDeleteDevice(pDeviceObj);
		DbgPrint("符号链接创建失败");
		return NULL;
	}
	DbgPrint("设备创建成功");
	return pDeviceObj;
}
//删除设备
VOID DeleteDevice()
{
	UNICODE_STRING uSymbolName = { 0 };
	RtlInitUnicodeString(&uSymbolName, SYMBOL_LINK_NAME);
	IoDeleteSymbolicLink(&uSymbolName); //删除符号链接
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
	DbgPrint("驱动卸载完毕...");
}
// 创建回调函数
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	//完成此IRP
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return status;
}
// 关闭回调函数
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	//完成请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
//主分发控制器,用于判断R3发送的控制信号
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;	 // STATUS_UNSUCCESSFUL
	ULONG information = 0;
	char* retBuffer = "hello lyshark";
	UNICODE_STRING uncode_buffer = { 0 };
	//判断请求发给谁
	if (pDevObj == DeviceObj)
	{
		//取请求的当前栈空间
		PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(pIrp);
		//取得缓冲区
		PVOID InputData = pIrp->AssociatedIrp.SystemBuffer;
		PVOID OutputData = pIrp->AssociatedIrp.SystemBuffer;

		//取缓冲区的长度
		ULONG inlen = irpsp->Parameters.DeviceIoControl.InputBufferLength;
		//取输出缓冲区的长度
		ULONG outlen = irpsp->Parameters.DeviceIoControl.OutputBufferLength;
		
		switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
		{
		case MY_CTL_CODE(1):
			//分配内存
			//uncode_buffer.Buffer = (PWSTR)ExAllocatePool(PagedPool, inlen);
			//RtlCopyMemory(uncode_buffer.Buffer, (WCHAR*)InputData, uncode_buffer.Length);
			//wcscpy_s(uncode_buffer.Buffer, sizeof(uncode_buffer.Buffer), (WCHAR*)InputData);//取消息
			DbgPrint((WCHAR*)InputData);
			//DbgPrint("输出字符串: %wZ \n", uncode_buffer.Buffer);
			//释放内存
			//ExFreePool(uncode_buffer.Buffer);
			//wcscpy_s((WCHAR*)buffer, outlen, L"ok");
			memcpy(OutputData, retBuffer, strlen(retBuffer)); //向应用层传递消息
			information = strlen(retBuffer) + 1;
			status = STATUS_SUCCESS;
			break;
		default:
			//未知的请求统一返回非法
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		//返回请求
		pIrp->IoStatus.Information = information;     //返回的输出长度
		pIrp->IoStatus.Status = status;           //请求完成状态
		IoCompleteRequest(pIrp, IO_NO_INCREMENT); //结束请求
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
/// 入口函数相当于main 被系统进程System调用一次
/// </summary>
/// <param name="driver"></param>
/// <param name="reg_path"></param>
/// <returns></returns>
// 入口函数
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING RegistryPath)
{

	//创建设备
	DeviceObj = CreateDevice(pDriver);
	pDriver->DriverUnload = UnDriver;                               // 卸载函数
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;         // 创建派遣函数
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;           // 关闭派遣函数
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;  // 分发函数
	//status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)NotifyCreateProcess, FALSE);
	DbgPrint(L"驱动入口");
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
	__inout PEPROCESS Process,//如果是创建(退出),则是被创建(退出)进程的exe名(不包括完整路径)
	__in HANDLE ProcessId,//如果是创建(退出)进程,则是被创建(退出)进程的pid
	__in_opt PPS_CREATE_NOTIFY_INFO CreateInfo//如果是创建进程,则里面包含被创建进程完整路径名
)
{

	if (CreateInfo)
	{
		//    DbgPrint("param ProcessId is %d\n", ProcessId); //被创建进程id
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