// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.AllocationType
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;

namespace Syringe.Win32
{
  [Flags]
  public enum AllocationType : uint
  {
    Commit = 4096U,
    Reserve = 8192U,
    Decommit = 16384U,
    Release = 32768U,
    Free = 65536U,
    Private = 131072U,
    Mapped = 262144U,
    Reset = 524288U,
    TopDown = 1048576U,
    WriteWatch = 2097152U,
    Physical = 4194304U,
    Rotate = 8388608U,
    LargePages = 536870912U,
    FourMbPages = 2147483648U,
  }
}
