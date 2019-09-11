using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CSharpCore
{
    public static class Container
    {

        public static void Init()
        {
            Console.WriteLine("C# Container Init");

            string Name = " " + EngineVersion.GetFullVersionString();
            Console.WriteLine(Name);
        }
        public static void Tick()
        {

        }
        public static void ShutDown()
        {

        }
    }
}
