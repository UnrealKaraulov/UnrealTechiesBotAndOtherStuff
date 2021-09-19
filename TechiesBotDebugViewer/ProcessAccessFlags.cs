// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.ProcessAccessFlags
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;

namespace Syringe.Win32
{
  [Flags]
  public enum ProcessAccessFlags : uint
  {
    Terminate = 1U,
    CreateThread = 2U,
    SetSessionID = 4U,
    VMOperation = 8U,
    VMRead = 16U,
    VMWrite = 32U,
    DUPHandle = 64U,
    CreateProcess = 128U,
    SetQuota = 256U,
    SetInformation = 512U,
    QueryInformation = 1024U,
    SuspendResume = 2048U,
    QueryLimitedInformation = 4096U,
    AllAccess = 2097151U,
    Synchronize = 1048576U,
    StandardRightsRequired = 983040U,
  }
}
