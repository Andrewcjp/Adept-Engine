using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace InterOpTest
{
    public static class ManagedClass
    {
        public static void ShowValue(ref int value)
        {
            int pid = EngineVersion.GetVersion();
            DialogResult result = MessageBox.Show("C# Message Box ", "C# Message Box" + value + "ver" + pid, MessageBoxButtons.OKCancel);
            if (result == DialogResult.OK)
                value = 1;
            else
                value = 2;
            return;
        }
    }
}
