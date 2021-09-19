using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace TechiesBotDebugViewer
{
    public partial class Form1 : Form
    {


        public Form1()
        {
            InitializeComponent();
        }


        List<string> previewdata = new List<string>();


        int errorcount = 0;
        int errorcount2 = 0;
        int errorcount3 = 0;

        bool inupdate = false;

        private void addtexttolisbox1(string x)
        {
            if (previewdata.Count < 50)
            {
                previewdata.Add(x);
            }
            else
            {
                previewdata.RemoveAt(0);
                previewdata.Add(x);
            }

            if (errorcount < int.MaxValue)
            {
                if ("ErrorErrorError1" == x)
                {
                    errorcount++;
                }

            }

            if (errorcount2 < int.MaxValue)
            {
                if ("ErrorErrorError2" == x)
                {
                    errorcount2++;
                }

            }

            if (errorcount2 < int.MaxValue)
            {
                if ("ErrorErrorError3" == x)
                {
                    errorcount3++;
                }

            }

            return;
        }

        ProcessMemory war3mem;

        int counterx = 0;

        bool needskiplolka = false;
        private void ReadTechiesDebugInfo()
        {
            Process war3proc;

            if (!needskiplolka)
                goto skiplolka;

            try
            {
                war3proc = Process.GetProcessesByName("war3")[1];
                goto skiplolka2;
            }
            catch { return; }

        skiplolka:
            ;

            try
            {
                war3proc = Process.GetProcessesByName("war3")[0];
            }
            catch { return; }
        skiplolka2:
            ;
            try
            {
                war3mem = new ProcessMemory(war3proc.Id);
                war3mem.StartProcess();
                Thread.Sleep(200);
                int addr = BitConverter.ToInt32(File.ReadAllBytes(Path.GetDirectoryName(war3proc.MainModule.FileName) + @"\debug.bin"), 0);
                for (int i = 0; i < 50; i++)
                {
                    if (war3mem.ReadUInt(addr + 256 * i) == 0)
                    {
                        if (i == 0)
                        {
                            needskiplolka = !needskiplolka;
                        }
                        break;
                    }
                    addtexttolisbox1(Encoding.UTF8.GetString(war3mem.ReadMem(addr + 256 * i, 256)));
                }
            }
            catch
            {

            }


        }

        private void SuperThread()
        {
            try
            {
                ReadTechiesDebugInfo();
            }
            catch
            {

            }
        }


        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            SuperThread();
            listBox1.BeginUpdate();
            listBox1.Items.Clear();
            try
            {
                for (int i = 49; i >= 0; i--)
                    listBox1.Items.Add(previewdata[i]);
            }
            catch
            {

            }
            listBox1.EndUpdate();
            label1.Text = counterx.ToString();
            label2.Text = errorcount.ToString();
            label3.Text = errorcount2.ToString();
            label4.Text = errorcount3.ToString();
        }
    }
}
