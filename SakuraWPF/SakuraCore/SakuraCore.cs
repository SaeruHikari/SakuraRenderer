using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace SakuraWPF
{
    class SakuraCore
    {
        public delegate void VoidFuncPointerType();
        public delegate void VoidWstringFuncPointerType([MarshalAs(UnmanagedType.LPWStr)] string param1);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool CreateSakuraCore(uint GraphicsAPIMask = 0);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool InitSakuraGraphicsCore(IntPtr hwnd, uint width, uint height);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TickSakuraCore(double deltaTime, uint coreMask = 255);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MsgSakuraCore(uint coreMask, uint MSG, uint param0, uint param1, uint param2);

        [DllImport(@"SakuraCore64_dbg.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Run();
    }

    public enum SAKURA_INPUT_MOUSE_TYPES
    {
        SAKURA_INPUT_MOUSE_LBUTTON = 0b00000001,
        SAKURA_INPUT_MOUSE_RBUTTON = 0b00000010,
    };
    public enum CORE_GRAPHICS_API_CONF
    {
        SAKURA_DRAW_WITH_D3D12 = 0b00000000,
        SAKURA_DRAW_WITH_VULKAN = 0b00000001,
        COUNT
    };
    public enum SAKURA_CORE_COMPONENT_MASKS : uint
    {
        SAKURA_CORE_COMPONENT_MASK_NONE = 0b00000000,
        SAKURA_CORE_COMPONENT_MASK_GRAPHICS = 0b00000001,
        SAKURA_CORE_COMPONENT_MASK_AUDIO = 0b00000010,
        SAKURA_CORE_COMPONENT_MASK_ALL = 0b11111111
    };
    public enum SAKURA_GRAPHICS_CORE_MSGS
    {
        SAKURA_GRAPHICS_CORE_MSG_TICK = 0b0000000001,   // param0: deltaTime
        SAKURA_GRAPHICS_CORE_MSG_DRAW = (SAKURA_GRAPHICS_CORE_MSG_TICK << 1),   // param...
        SAKURA_GRAPHICS_CORE_MSG_RESIZE = (SAKURA_GRAPHICS_CORE_MSG_DRAW << 1), // param0 width, param1 height
        SAKURA_GRAPHICS_CORE_MSG_KEYPRESS = (SAKURA_GRAPHICS_CORE_MSG_RESIZE << 1), //
        SAKURA_GRAPHICS_CORE_MSG_KEYRELEASE = (SAKURA_GRAPHICS_CORE_MSG_KEYPRESS << 1),
        SAKURA_GRAPHICS_CORE_MSG_MOUSEPRESS = (SAKURA_GRAPHICS_CORE_MSG_KEYRELEASE << 1),
        SAKURA_GRAPHICS_CORE_MSG_MOUSERELEASE = (SAKURA_GRAPHICS_CORE_MSG_MOUSEPRESS << 1),
        SAKURA_GRAPHICS_CORE_MSG_MOUSEMOVE = (SAKURA_GRAPHICS_CORE_MSG_MOUSERELEASE << 1),
        SAKURA_GRAPHICS_CORE_MSG_WHEEL = (SAKURA_GRAPHICS_CORE_MSG_MOUSEMOVE << 1)
    };
}
