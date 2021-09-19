// Decompiled with JetBrains decompiler
// Type: Syringe.CustomMarshalAsAttribute
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

namespace Syringe
{
  public class CustomMarshalAsAttribute : CustomMarshalAttribute
  {
    private CustomUnmanagedType _val;

    public CustomUnmanagedType Value
    {
      get
      {
        return this._val;
      }
    }

    public CustomMarshalAsAttribute(CustomUnmanagedType unmanagedType)
    {
      this._val = unmanagedType;
    }

    public CustomMarshalAsAttribute(short unmanagedType)
    {
      this._val = (CustomUnmanagedType) unmanagedType;
    }
  }
}
