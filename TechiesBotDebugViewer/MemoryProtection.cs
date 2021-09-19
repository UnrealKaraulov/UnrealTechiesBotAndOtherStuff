// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.MemoryProtection
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

namespace Syringe.Win32
{
  public enum MemoryProtection : uint
  {
    NoAccess = 1U,
    ReadOnly = 2U,
    ReadWrite = 4U,
    WriteCopy = 8U,
    Execute = 16U,
    ExecuteRead = 32U,
    ExecuteReadWrite = 64U,
    ExecuteWriteCopy = 128U,
    PageGuard = 256U,
    NoCache = 512U,
    WriteCombine = 1024U,
  }
}
