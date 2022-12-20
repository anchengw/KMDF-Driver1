using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestDriver
{
    public static class MarshalHelper
    {
        /// <summary>
        /// byte[]转IntPtr
        /// </summary>
        /// <param name="array"></param>
        /// <returns></returns>
        public static IntPtr ArrToPtr(byte[] array)
        {
            if(array == null)
                return IntPtr.Zero;
            return System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(array, 0);
        }
        //byte[]转换为Intptr (方法一)
        public static IntPtr BytesToIntptr(byte[] bytes)
        {
            int size = bytes.Length;
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(bytes, 0, buffer, size);
                return buffer;
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }

        //struct转换为byte[]
        public static byte[] StructToBytes(object structObj)
        {
            int size = Marshal.SizeOf(structObj);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(structObj, buffer, false);
                byte[] bytes = new byte[size];
                Marshal.Copy(buffer, bytes, 0, size);
                return bytes;
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }
        //byte[]转换为struct
        public static object BytesToStruct(byte[] bytes, Type type)
        {
            int size = Marshal.SizeOf(type);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(bytes, 0, buffer, size);
                return Marshal.PtrToStructure(buffer, type);
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }
        public static T ToStructure<T>(this IntPtr ptr)
        {
#if NETCORE
            return Marshal.PtrToStructure<T>(ptr);
#else
            return (T)Marshal.PtrToStructure(ptr, typeof(T));
#endif
        }

        public static uint SizeOf<T>()
        {
#if NETCORE
            return (uint)Marshal.SizeOf<T>();
#else
            return (uint)Marshal.SizeOf(typeof(T));
#endif
        }
        public static string GetWin32ErrorMessage(int errorCode)
        {
            return new Win32Exception(errorCode).Message;
        }
        public static int GetLastError()
        { 
            return Marshal.GetLastWin32Error(); 
        }
    }
}
