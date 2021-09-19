// Decompiled with JetBrains decompiler
// Type: Syringe.CustomMarshal
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Syringe
{
  public static class CustomMarshal
  {
    public static bool IsCustomMarshalType(Type t)
    {
      foreach (MemberInfo memberInfo in t.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
      {
        if (memberInfo.IsDefined(typeof (CustomMarshalAttribute), true))
          return true;
      }
      return false;
    }

    public static bool IsCustomMarshalObject(object o)
    {
      return CustomMarshal.IsCustomMarshalType(o.GetType());
    }

    public static int SizeOf(object o)
    {
      if (!CustomMarshal.IsCustomMarshalObject(o))
        return CustomMarshal.SizeOf(o.GetType());
      int num = Marshal.SizeOf(o);
      foreach (FieldInfo fieldInfo in o.GetType().GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
      {
        if (fieldInfo.IsDefined(typeof (CustomMarshalAttribute), true))
        {
          foreach (object obj in fieldInfo.GetCustomAttributes(typeof (CustomMarshalAttribute), true))
          {
            int byteCount;
            switch (((CustomMarshalAsAttribute) obj).Value)
            {
              case CustomUnmanagedType.LPStr:
                byteCount = Encoding.ASCII.GetByteCount((string) fieldInfo.GetValue(o) + (object) char.MinValue);
                break;
              case CustomUnmanagedType.LPWStr:
                byteCount = Encoding.Unicode.GetByteCount((string) fieldInfo.GetValue(o) + (object) char.MinValue);
                break;
              default:
                throw new NotSupportedException("Operation not yet supported by CustomMarshaller");
            }
            num += byteCount;
          }
        }
      }
      return num;
    }

    public static int SizeOf(Type t)
    {
      return Marshal.SizeOf(t);
    }

    public static void StructureToPtr(object structure, IntPtr ptr, bool fDeleteOld)
    {
      if (!CustomMarshal.IsCustomMarshalObject(structure))
      {
        Marshal.StructureToPtr(structure, ptr, fDeleteOld);
      }
      else
      {
        StructLayoutAttribute structLayoutAttribute = structure.GetType().StructLayoutAttribute;
        if (structLayoutAttribute.IsDefaultAttribute() || structLayoutAttribute.Value == LayoutKind.Auto)
          throw new ArgumentException("Structure must have StructLayoutAttribute with LayoutKind Explicit or Sequential", "structure");
        uint num1 = 0U;
        uint num2 = (uint) ptr.ToInt32();
        uint num3 = (uint) Marshal.SizeOf(structure);
        foreach (FieldInfo fieldInfo in structure.GetType().GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
        {
          uint num4 = num2 + (uint) (int) Marshal.OffsetOf(structure.GetType(), fieldInfo.Name);
          if (fieldInfo.IsDefined(typeof (CustomMarshalAsAttribute), true))
          {
            byte[] bytes;
            switch (((CustomMarshalAsAttribute) fieldInfo.GetCustomAttributes(typeof (CustomMarshalAsAttribute), true)[0]).Value)
            {
              case CustomUnmanagedType.LPStr:
                bytes = Encoding.ASCII.GetBytes((string) fieldInfo.GetValue(structure) + (object) char.MinValue);
                break;
              case CustomUnmanagedType.LPWStr:
                bytes = Encoding.Unicode.GetBytes((string) fieldInfo.GetValue(structure) + (object) char.MinValue);
                break;
              default:
                throw new NotSupportedException("Operation not yet supported");
            }
            uint num5 = num2 + num3 + num1;
            Marshal.WriteIntPtr(new IntPtr((long) num4), new IntPtr((long) num5));
            int index = 0;
            while (index < bytes.Length)
            {
              Marshal.WriteByte(new IntPtr((long) (num5 + (uint) index)), bytes[index]);
              ++index;
              ++num1;
            }
          }
          else
            Marshal.StructureToPtr(fieldInfo.GetValue(structure), new IntPtr((long) num4), fDeleteOld);
        }
      }
    }

    public static object PtrToStructure(IntPtr ptr, Type structureType)
    {
      if (ptr == IntPtr.Zero)
        return (object) null;
      if (structureType == null)
        throw new ArgumentNullException("structureType");
      if (structureType.IsGenericType)
        throw new ArgumentException("Structure type must be non-generic", "structureType");
      if (!CustomMarshal.IsCustomMarshalType(structureType))
        return Marshal.PtrToStructure(ptr, structureType);
      StructLayoutAttribute structLayoutAttribute = structureType.StructLayoutAttribute;
      if (structLayoutAttribute.IsDefaultAttribute() || structLayoutAttribute.Value == LayoutKind.Auto)
        throw new ArgumentException("Structure must have StructLayoutAttribute with LayoutKind Explicit or Sequential", "structure");
      object instance = Activator.CreateInstance(structureType);
      uint num1 = (uint) ptr.ToInt32();
      Marshal.SizeOf(instance);
      foreach (FieldInfo fieldInfo in structureType.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
      {
        uint num2 = num1 + (uint) (int) Marshal.OffsetOf(structureType, fieldInfo.Name);
        if (fieldInfo.IsDefined(typeof (CustomMarshalAsAttribute), true))
        {
          IntPtr ptr1 = Marshal.ReadIntPtr(new IntPtr((long) num2));
          switch (((CustomMarshalAsAttribute) fieldInfo.GetCustomAttributes(typeof (CustomMarshalAsAttribute), true)[0]).Value)
          {
            case CustomUnmanagedType.LPStr:
              fieldInfo.SetValue(instance, (object) Marshal.PtrToStringAnsi(ptr1));
              continue;
            case CustomUnmanagedType.LPWStr:
              fieldInfo.SetValue(instance, (object) Marshal.PtrToStringUni(ptr1));
              continue;
            default:
              throw new NotSupportedException("Operation not currently supported");
          }
        }
        else
          fieldInfo.SetValue(instance, Marshal.PtrToStructure(new IntPtr((long) num2), fieldInfo.FieldType));
      }
      return instance;
    }

    public static void RebaseUnmanagedStructure(IntPtr baseAddress, IntPtr targetAddress, Type structureType)
    {
      if (baseAddress == IntPtr.Zero)
        throw new ArgumentException("Invalid base address", "baseAddress");
      if (targetAddress == IntPtr.Zero)
        throw new ArgumentException("Invalid target address", "targetAddress");
      if (structureType == null)
        throw new ArgumentNullException("structureType");
      if (!CustomMarshal.IsCustomMarshalType(structureType))
        return;
      int num = targetAddress.ToInt32() - baseAddress.ToInt32();
      foreach (FieldInfo fieldInfo in structureType.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
      {
        if (fieldInfo.IsDefined(typeof (CustomMarshalAsAttribute), true))
        {
          IntPtr ptr = new IntPtr(baseAddress.ToInt32() + Marshal.OffsetOf(structureType, fieldInfo.Name).ToInt32());
          IntPtr val = new IntPtr(Marshal.ReadIntPtr(ptr).ToInt32() + num);
          Marshal.WriteIntPtr(ptr, val);
        }
      }
    }
  }
}
