// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.IMAGE_FILE_HEADER
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

namespace Syringe.Win32
{
  public struct IMAGE_FILE_HEADER
  {
    public ushort Machine;
    public ushort NumberOfSections;
    public uint TimeDateStamp;
    public uint PointerToSymbolTable;
    public uint NumberOfSymbols;
    public ushort SizeOfOptionalHeader;
    public ushort Characteristics;
  }
}
