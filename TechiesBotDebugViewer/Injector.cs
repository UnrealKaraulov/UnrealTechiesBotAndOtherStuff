// Decompiled with JetBrains decompiler
// Type: Syringe.Injector
// Assembly: Syringe, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: 26D8B4D5-6B67-41A6-838D-CA36AD0BE10C
// Assembly location: C:\Projects\HackProjects\Syringe.dll

using Syringe.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace Syringe
{
    public class Injector : IDisposable
    {
        private Process _process;
        private IntPtr _handle;
        private Dictionary<string , Injector.InjectedModule> injectedModules;

        public bool EjectOnDispose { get; set; }

        public Injector ( Process process )
            : this( process , true )
        {
        }

        public Injector ( Process process , bool ejectOnDispose )
        {
            if ( process == null )
                throw new ArgumentNullException( "process" );
            if ( process.Id == Process.GetCurrentProcess( ).Id )
                throw new InvalidOperationException( "Cannot create an injector for the current process" );
            Process.EnterDebugMode( );
            this._handle = Imports.OpenProcess( ProcessAccessFlags.CreateThread | ProcessAccessFlags.VMOperation | ProcessAccessFlags.VMRead | ProcessAccessFlags.VMWrite | ProcessAccessFlags.QueryInformation , false , process.Id );
            if ( this._handle == IntPtr.Zero )
                throw new Win32Exception( Marshal.GetLastWin32Error( ) );
            this._process = process;
            this.EjectOnDispose = ejectOnDispose;
            this.injectedModules = new Dictionary<string , Injector.InjectedModule>( );
        }

        public void InjectLibrary ( string libPath )
        {
            if ( this._process == null )
                throw new InvalidOperationException( "This injector has no associated process and thus cannot inject a library" );
            if ( this._handle == IntPtr.Zero )
                throw new InvalidOperationException( "This injector does not have a valid handle to the associated process and thus cannot inject a library" );
            if ( !File.Exists( libPath ) )
                throw new FileNotFoundException( string.Format( "Unable to find library {0} to inject into process {1}" , ( object ) libPath , ( object ) this._process.ProcessName ) , libPath );
            string fullPath = Path.GetFullPath( libPath );
            string fileName = Path.GetFileName( fullPath );
            IntPtr num1 = IntPtr.Zero;
            IntPtr num2 = IntPtr.Zero;
            IntPtr num3 = Marshal.StringToHGlobalUni( fullPath );
            try
            {
                uint num4 = ( uint ) Encoding.Unicode.GetByteCount( fullPath );
                IntPtr moduleHandle = Imports.GetModuleHandle( "Kernel32" );
                if ( moduleHandle == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                IntPtr procAddress = Imports.GetProcAddress( moduleHandle , "LoadLibraryW" );
                if ( procAddress == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                num1 = Imports.VirtualAllocEx( this._handle , IntPtr.Zero , num4 , AllocationType.Commit , MemoryProtection.ReadWrite );
                if ( num1 == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                int lpNumberOfBytesWritten;
                if ( !Imports.WriteProcessMemory( this._handle , num1 , num3 , num4 , out lpNumberOfBytesWritten ) || lpNumberOfBytesWritten != ( int ) num4 )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                num2 = Imports.CreateRemoteThread( this._handle , IntPtr.Zero , 0U , procAddress , num1 , 0U , IntPtr.Zero );
                if ( num2 == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                if ( ( int ) Imports.WaitForSingleObject( num2 , uint.MaxValue ) != 0 )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                IntPtr lpExitCode;
                if ( !Imports.GetExitCodeThread( num2 , out lpExitCode ) )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                if ( lpExitCode == IntPtr.Zero )
                    throw new Exception( "Code executed properly, but unable to get an appropriate module handle, possible Win32Exception" , ( Exception ) new Win32Exception( Marshal.GetLastWin32Error( ) ) );
                ProcessModule module = ( ProcessModule ) null;
                foreach ( ProcessModule processModule in ( ReadOnlyCollectionBase ) this._process.Modules )
                {
                    if ( processModule.ModuleName == fileName )
                    {
                        module = processModule;
                        break;
                    }
                }
                if ( module == null )
                    throw new Exception( "Injected module could not be found within the target process!" );
                this.injectedModules.Add( fileName , new Injector.InjectedModule( module ) );
            }
            finally
            {
                Marshal.FreeHGlobal( num3 );
                Imports.CloseHandle( num2 );
                Imports.VirtualFreeEx( this._process.Handle , num1 , 0U , AllocationType.Release );
            }
        }

        public void EjectLibrary ( string libName )
        {
            string key = File.Exists( libName ) ? Path.GetFileName( Path.GetFullPath( libName ) ) : libName;
            if ( !this.injectedModules.ContainsKey( key ) )
                throw new InvalidOperationException( "That module has not been injected into the process and thus cannot be ejected" );
            IntPtr num = IntPtr.Zero;
            try
            {
                IntPtr moduleHandle = Imports.GetModuleHandle( "Kernel32" );
                if ( moduleHandle == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                IntPtr procAddress = Imports.GetProcAddress( moduleHandle , "FreeLibrary" );
                if ( procAddress == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                num = Imports.CreateRemoteThread( this._handle , IntPtr.Zero , 0U , procAddress , this.injectedModules [ key ].BaseAddress , 0U , IntPtr.Zero );
                if ( num == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                if ( ( int ) Imports.WaitForSingleObject( num , uint.MaxValue ) != 0 )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                IntPtr lpExitCode;
                if ( !Imports.GetExitCodeThread( num , out lpExitCode ) )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                if ( lpExitCode == IntPtr.Zero )
                    throw new Exception( "FreeLibrary failed in remote process" );
            }
            finally
            {
                Imports.CloseHandle( num );
            }
        }

        public IntPtr CallExport ( string libName , string funcName )
        {
            return this.CallExport( uint.MaxValue , libName , funcName );
        }

        public IntPtr CallExport ( uint timeout , string libName , string funcName )
        {
            return this.CallExportInternal( timeout , libName , funcName , IntPtr.Zero , ( Type ) null , 0U );
        }

        public IntPtr CallExport<T> ( string libName , string funcName , T data ) where T : struct
        {
            return this.CallExport<T>( uint.MaxValue , libName , funcName , data );
        }

        public IntPtr CallExport<T> ( string libName , string funcName , T data , int size ) where T : struct
        {
            IntPtr num = IntPtr.Zero;
            try
            {
                int cb = size;
                num = Marshal.AllocHGlobal( cb );
                CustomMarshal.StructureToPtr( ( object ) data , num , true );
                return this.CallExportInternal( 1 , libName , funcName , num , typeof( T ) , ( uint ) cb );
            }
            finally
            {
                Marshal.FreeHGlobal( num );
            }
        }

        public IntPtr CallExport<T> ( uint timeout , string libName , string funcName , T data ) where T : struct
        {
            IntPtr num = IntPtr.Zero;
            try
            {
                int cb = CustomMarshal.SizeOf( ( object ) data );
                num = Marshal.AllocHGlobal( cb );
                CustomMarshal.StructureToPtr( ( object ) data , num , true );
                return this.CallExportInternal( timeout , libName , funcName , num , typeof( T ) , ( uint ) cb );
            }
            finally
            {
                Marshal.FreeHGlobal( num );
            }
        }

        public IntPtr CallExport<T> ( uint timeout , string libName , string funcName , T data,int size) where T : struct
        {
            IntPtr num = IntPtr.Zero;
            try
            {
                int cb = size;
                num = Marshal.AllocHGlobal( cb );
                CustomMarshal.StructureToPtr( ( object ) data , num , true );
                return this.CallExportInternal( timeout , libName , funcName , num , typeof( T ) , ( uint ) cb );
            }
            finally
            {
                Marshal.FreeHGlobal( num );
            }
        }

        private IntPtr GetModuleAddr ( string mdl , string export )
        {
            foreach ( ProcessModule pm in _process.Modules )
            {
                if ( pm.FileName.IndexOf( mdl ) > -1 )
                {
                    return FindExport( pm , export );
                }
                try
                {
                    if ( pm.FileName.IndexOf( Path.GetFileName( mdl ) ) > -1 )
                    {
                        return FindExport( pm , export );
                    }
                }
                catch
                {

                }
            }
            return IntPtr.Zero;
        }

        private IntPtr CallExportInternal ( uint timeout , string libName , string funcName , IntPtr data , Type dataType , uint dataSize )
        {
            IntPtr lpStartAddress = GetModuleAddr( libName , funcName );
            IntPtr num1 = IntPtr.Zero;
            IntPtr num2 = IntPtr.Zero;
            try
            {
                if ( !( data == IntPtr.Zero ) && ( int ) dataSize != 0 && dataType != null )
                {
                    num1 = Imports.VirtualAllocEx( this._handle , IntPtr.Zero , dataSize , AllocationType.Commit , MemoryProtection.ReadWrite );
                    if ( num1 == IntPtr.Zero )
                        throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                    CustomMarshal.RebaseUnmanagedStructure( data , num1 , dataType );
                    int lpNumberOfBytesWritten;
                    if ( !Imports.WriteProcessMemory( this._handle , num1 , data , dataSize , out lpNumberOfBytesWritten ) )
                        throw new Win32Exception( Marshal.GetLastWin32Error( ) );

                    num2 = Imports.CreateRemoteThread( this._handle , IntPtr.Zero , 0U , lpStartAddress , num1 , 0U , IntPtr.Zero );
                    if ( num2 == IntPtr.Zero )
                        throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                    Imports.WaitForSingleObject( num2 , 3000 );
                }

                return num2;
            }
            finally
            {

                Imports.VirtualFreeEx( this._process.Handle , num1 , 0U , AllocationType.Release );
                Imports.CloseHandle( num2 );
            }
        }

        public void Dispose ( )
        {
            if ( this.EjectOnDispose )
            {
                foreach ( string libName in this.injectedModules.Keys )
                    this.EjectLibrary( libName );
            }
            if ( this._handle != IntPtr.Zero )
                Imports.CloseHandle( this._handle );
            this._handle = IntPtr.Zero;
            Process.LeaveDebugMode( );
        }

        private IntPtr xxnum = IntPtr.Zero;

        private IntPtr FindExport ( ProcessModule Module , string func )
        {

            try
            {
                xxnum = Imports.LoadLibraryEx( Module.FileName , IntPtr.Zero , LoadLibraryExFlags.DontResolveDllReferences );
                if ( xxnum == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                IntPtr procAddress = Imports.GetProcAddress( xxnum , func );
                if ( procAddress == IntPtr.Zero )
                    throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                return IntPtr.Size != 8 ? new IntPtr( Module.BaseAddress.ToInt32( ) + ( procAddress.ToInt32( ) - xxnum.ToInt32( ) ) ) : new IntPtr( Module.BaseAddress.ToInt64( ) + ( procAddress.ToInt64( ) - xxnum.ToInt64( ) ) );
            }
            catch
            {
                return xxnum;
            }

        }

        private class InjectedModule
        {
            private Dictionary<string , IntPtr> exports;

            public ProcessModule Module { get; private set; }

            public IntPtr BaseAddress
            {
                get
                {
                    return this.Module.BaseAddress;
                }
            }

            public IntPtr this [ string func ]
            {
                get
                {
                    if ( !this.exports.ContainsKey( func ) )
                        this.exports [ func ] = this.FindExport( func );
                    return this.exports [ func ];
                }
            }

            public InjectedModule ( ProcessModule module )
            {
                this.Module = module;
                this.exports = new Dictionary<string , IntPtr>( );
            }

            private IntPtr FindExport ( string func )
            {
                IntPtr num = IntPtr.Zero;
                try
                {
                    num = Imports.LoadLibraryEx( this.Module.FileName , IntPtr.Zero , LoadLibraryExFlags.DontResolveDllReferences );
                    if ( num == IntPtr.Zero )
                        throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                    IntPtr procAddress = Imports.GetProcAddress( num , func );
                    if ( procAddress == IntPtr.Zero )
                        throw new Win32Exception( Marshal.GetLastWin32Error( ) );
                    return IntPtr.Size != 8 ? new IntPtr( this.Module.BaseAddress.ToInt32( ) + ( procAddress.ToInt32( ) - num.ToInt32( ) ) ) : new IntPtr( this.Module.BaseAddress.ToInt64( ) + ( procAddress.ToInt64( ) - num.ToInt64( ) ) );
                }
                finally
                {
                    Imports.CloseHandle( num );
                }
            }
        }
    }
}
