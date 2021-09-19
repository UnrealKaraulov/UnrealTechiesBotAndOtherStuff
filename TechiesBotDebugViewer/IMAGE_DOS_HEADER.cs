// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.IMAGE_DOS_HEADER
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System.Runtime.InteropServices;

namespace Syringe.Win32
{
  public struct IMAGE_DOS_HEADER
  {
    public ushort e_magic;
    public ushort e_cblp;
    public ushort e_cp;
    public ushort e_crlc;
    public ushort e_cparhdr;
    public ushort e_minalloc;
    public ushort e_maxalloc;
    public ushort e_ss;
    public ushort e_sp;
    public ushort e_csum;
    public ushort e_ip;
    public ushort e_cs;
    public ushort e_lfarlc;
    public ushort e_ovno;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
    public ushort[] e_res1;
    public ushort e_oemid;
    public ushort e_oeminfo;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
    public ushort[] e_res2;
    public int e_lfanew;
  }
}
