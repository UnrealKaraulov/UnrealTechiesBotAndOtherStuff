// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.ProcessCreationFlags
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;

namespace Syringe.Win32
{
  [Flags]
  public enum ProcessCreationFlags : uint
  {
    None = 0U,
    DebugProcess = 1U,
    DebugOnlyThisProcess = 2U,
    CreateSuspended = 4U,
    DetachedProcess = 8U,
    CreateNewConsole = 16U,
    CreateNewProcessGroup = 512U,
    CreateUnicodeEnvironment = 1024U,
    CreateSeparateWowVDM = 2048U,
    CreateSharedWowVDM = 4096U,
    InheritParentAffinity = 65536U,
    CreateProtectedProcess = 262144U,
    ExtendedStartupInfoPresent = 524288U,
    CreateBreakawayFromJob = 16777216U,
    CreatePreserveCodeAuthzLevel = 33554432U,
    CreateDefaultErrorMode = 67108864U,
    CreateNoWindow = 134217728U,
  }
}
