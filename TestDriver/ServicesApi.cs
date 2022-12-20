using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestDriver
{
    public class ServicesApi
    {
        private const int SC_MANAGER_CREATE_SERVICE = 2;
        private const int SERVICE_START = 16;
        private const int SERVICE_KERNEL_DRIVER = 1;
        private const int SERVICE_DEMAND_START = 3;
        private const int SERVICE_ERROR_IGNORE = 0;
        private readonly static IntPtr NULL = IntPtr.Zero;

        [DllImport("advapi32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr OpenSCManager(string machineName, string databaseName, uint dwAccess);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr CreateService(IntPtr hSCManager, string lpServiceName, string lpDisplayName, int dwDesiredAccess, int dwServiceType, int dwStartType, int dwErrorControl, string lpBinaryPathName, int lpLoadOrderGroup, int lpdwTagId, int lpDependencies, int lpServiceStartName, int lpPassword);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern IntPtr OpenService(IntPtr hSCManager, string lpServiceName, uint dwDesiredAccess);

        [DllImport("advapi32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool CloseServiceHandle(IntPtr hSCManager);

        [DllImport("advapi32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool StartService(IntPtr hService, int dwNumServiceArgs, string[] lpServiceArgVectors);

        public static bool LoadSysDriver(string strFileName, string strServiceName, string strDisplayName)
        {
            IntPtr hSCManager = OpenSCManager(null, null, SC_MANAGER_CREATE_SERVICE);
            if (hSCManager == NULL)
            {
                return false;
            }
            IntPtr hService = CreateService(hSCManager, strServiceName, strDisplayName, SERVICE_START, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, strFileName, 0, 0, 0, 0, 0);
            if (hService == NULL)
            {
                hService = OpenService(hSCManager, strServiceName, SERVICE_START);
                if (hService == NULL)
                {
                    return !CloseServiceHandle(hSCManager);
                }
            }
            try
            {
                return CloseServiceHandle(hService) && CloseServiceHandle(hSCManager);
            }
            finally
            {
                StartService(hService, 0, null);
            }
        }
    }
}
