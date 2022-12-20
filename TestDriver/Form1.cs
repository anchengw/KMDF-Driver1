using System;
using System.Windows.Forms;

namespace TestDriver
{
    public partial class Form1 : Form
    {
        IntPtr hDevice;
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string deviceName = @"\\.\link_mydevice";
            if (!WinAPi.OpenDriver(deviceName, out hDevice))
            {
                int erroCode = MarshalHelper.GetLastError();
                ShowMsg($"设备{deviceName}打开失败:{MarshalHelper.GetWin32ErrorMessage(erroCode)}");
            }
            else
                ShowMsg($"{deviceName}已成功打开，Handle为:{hDevice}");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //Test();
            CustomeDevice();
        }
        /// <summary>
        /// 取物理盘大小
        /// </summary>
        private void Test()
        {
            uint ioctlCode = (uint)WinAPi.EIOControlCode.DiskGetLengthInfo;
            byte[] inarr = new byte[1];
            byte[] barr = WinAPi.DeviceIoControl(hDevice, ioctlCode, inarr, sizeof(Int64));
            var len = BitConverter.ToInt64(barr, 0);
            len = len / (1024 * 1024 * 1024);
            ShowMsg($"长度为：{len}GB");
        }
        private void CustomeDevice()
        {
            uint ioctlCode = WinAPi.CustomCtlCode(1);
            string str = "Hello driver,this is a message from app.";
            byte[] inByteArray = System.Text.Encoding.Default.GetBytes(str);
            byte[] barr = WinAPi.DeviceIoControl(hDevice, ioctlCode, inByteArray, 256);
            var retstr = System.Text.Encoding.Default.GetString(barr);
            
            ShowMsg($"驱动回应消息：{retstr}");
        }
        private void button3_Click(object sender, EventArgs e)
        {
            if(WinAPi.CloseHandle(hDevice))
            {
                ShowMsg("设备已关闭！");
            }
            else
            {
                int erroCode = MarshalHelper.GetLastError();
                ShowMsg($"设备关闭失败:{MarshalHelper.GetWin32ErrorMessage(erroCode)}");
            }
        }
        private void ShowMsg(string text)
        {
            listBox1.Items.Add(text);
        }
        //加载驱动
        private void button4_Click(object sender, EventArgs e)
        {
            if(ServicesApi.LoadSysDriver(Application.StartupPath + @"\MyDriver1.sys", "FirstDrvier", "FirstDrvier"))
                ShowMsg("驱动加载成功！");
            else
                ShowMsg("驱动加载失败！");
        }
        
    }
}
