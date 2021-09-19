// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.LoadLibraryExFlags
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;

namespace Syringe.Win32
{
  [Flags]
  public enum LoadLibraryExFlags : uint
  {
    DontResolveDllReferences = 1U,
    LoadLibraryAsDatafile = 2U,
    LoadLibraryWithAlteredSearchPath = 8U,
    LoadIgnoreCodeAuthzLevel = 16U,
    LoadLibraryAsImageResource = 32U,
    LoadLibraryAsDatafileExclusive = 64U,
  }
}
