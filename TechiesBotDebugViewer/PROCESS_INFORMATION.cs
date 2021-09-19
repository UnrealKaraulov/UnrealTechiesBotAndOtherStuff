// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.PROCESS_INFORMATION
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;

namespace Syringe.Win32
{
  public struct PROCESS_INFORMATION
  {
    public IntPtr hProcess;
    public IntPtr hThread;
    public int dwProcessId;
    public int dwThreadId;
  }
}
