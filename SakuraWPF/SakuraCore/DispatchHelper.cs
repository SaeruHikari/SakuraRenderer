using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace SakuraWPF
{
    public static class DispatcherHelper
    {
        [SecurityPermissionAttribute(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
        public static void DoEvents()
        {
            DispatcherFrame frame = new DispatcherFrame();
            Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.Background, new DispatcherOperationCallback(ExitFrames), frame);
            try { Dispatcher.PushFrame(frame); }
            catch (InvalidOperationException) { }
        }
        private static object ExitFrames(object frame)
        {
            ((DispatcherFrame)frame).Continue = false;
            return null;
        }
    }
}
