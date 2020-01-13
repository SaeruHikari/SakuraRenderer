using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace SakuraWPF
{
    public partial class SakuraCore
    {
        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetStringProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
            [MarshalAs(UnmanagedType.LPStr)] string val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetIntProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
            int val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetIntProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           out int val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetUintProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           uint val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetUintProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           out uint val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetFloatProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           float val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetSVectorProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           out SVector val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetSVectorProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           SVector val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetFloatProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           out float val, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetSubmemberProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
           out IntPtr submember, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject");

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetStringProp(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string name,
            out StringWrapper result, [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject", bool bCheckDerives = true);
        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetObjectProperties(IntPtr _object, out SPropertyDataArrayWrapper result,
            [MarshalAs(UnmanagedType.LPStr)] string _typename = "ISSilentObject", bool bCheckDerives = true);


        public static List<SPropertyData> GetObjectProperties(IntPtr _object, string _typename = "ISSilentObject")
        {
            var list = new List<SPropertyData>();
            SPropertyDataArrayWrapper wrapper;
            GetObjectProperties(_object, out wrapper, _typename);
            for(int i = 0; i < wrapper.num; i++)
            {
                SPropertyData _meta = new SPropertyData();
                unsafe
                {
                    SPropertyDataWrapper* _wrapper = wrapper.wrappers[i];
                    _meta.PropName = new string((*_wrapper).PropName);
                    _meta.PropType = new string((*_wrapper).PropType);
                    _meta.PropDescription = new string((*_wrapper).PropDescription);
                    _meta.SourceType = new string((*_wrapper).SourceType);
                    _meta.SObject = _object;
                    list.Add(_meta);
                }
            }
            return list;
        }

        public static string GetStringProp(IntPtr sakuraObject, string propName, string typeName = "ISSilentObject")
        {
            try
            {
                StringWrapper _name;
                SakuraCore.GetStringProp(sakuraObject, propName, out _name, typeName);
                unsafe
                {
                    string name = new string(_name.str);
                    return name;
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.ToString());
            }
            return null;
        }
        
        public struct SPropertyData
        {
            public string SourceType;
            public string PropName;
            public string PropType;
            public string PropDescription;
            public IntPtr SObject;
        };
        static public bool PropIsContainer(SPropertyData property)
        {
            return (property.PropType.Contains("std::vector") |
                property.PropType.Contains("std::map") |
                property.PropType.Contains("std::unordered") |
                property.PropType.Contains("std::array"));
        }
        public struct SVector
        {
            public float x;
            public float y;
            public float z;
        };

        private struct SPropertyDataArrayWrapper
        {
            public unsafe SPropertyDataWrapper** wrappers;
            public uint num;
        };
        private struct SPropertyDataWrapper
        {
            public unsafe sbyte* SourceType;
            public unsafe sbyte* PropName;
            public unsafe sbyte* PropType;
            public unsafe sbyte* PropDescription;
        };
        private struct StringWrapper
        {
            public unsafe sbyte* str;
        };
        private struct WStringWrapper
        {
            public unsafe char* wstr;
        };
    }
}
