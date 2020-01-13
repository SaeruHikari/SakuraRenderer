using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
    
namespace SakuraWinformLib
{
    class SakuraCore
    {
        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool CreateSakuraCore(uint GraphicsAPIMask = 0);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool InitSakuraGraphicsCore(IntPtr hwnd, uint width, uint height);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TickSakuraCore(double deltaTime, uint coreMask = 255);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MsgSakuraCore(uint coreMask, uint MSG, uint param0, uint param1, uint param2);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MsgProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam);
    }
}
