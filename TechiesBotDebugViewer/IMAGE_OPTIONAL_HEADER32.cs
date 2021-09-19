// Decompiled with JetBrains decompiler
// Type: Syringe.Win32.IMAGE_OPTIONAL_HEADER32
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System.Runtime.InteropServices;

namespace Syringe.Win32
{
  public struct IMAGE_OPTIONAL_HEADER32
  {
    public ushort Magic;
    public byte MajorLinkerVersion;
    public byte MinorLinkerVersion;
    public uint SizeOfCode;
    public uint SizeOfInitializedData;
    public uint SizeOfUninitializedData;
    public uint AddressOfEntryPoint;
    public uint BaseOfCode;
    public uint BaseOfData;
    public uint ImageBase;
    public uint SectionAlignment;
    public uint FileAlignment;
    public ushort MajorOperatingSystemVersion;
    public ushort MinorOperatingSystemVersion;
    public ushort MajorImageVersion;
    public ushort MinorImageVersion;
    public ushort MajorSubsystemVersion;
    public ushort MinorSubsystemVersion;
    public uint Win32VersionValue;
    public uint SizeOfImage;
    public uint SizeOfHeaders;
    public uint CheckSum;
    public ushort Subsystem;
    public ushort DllCharacteristics;
    public uint SizeOfStackReserve;
    public uint SizeOfStackCommit;
    public uint SizeOfHeapReserve;
    public uint SizeOfHeapCommit;
    public uint LoaderFlags;
    public uint NumberOfRvaAndSizes;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
    public IMAGE_DATA_DIRECTORY[] DataDirectory;
  }
}
