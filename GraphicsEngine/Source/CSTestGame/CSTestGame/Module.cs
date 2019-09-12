using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using CSharpCore;
namespace CSTestGame.CSTestGame
{
    public class Module
    {
        public static void Init()
        {
            Container.AddType(typeof(LightTest));
            Console.WriteLine("asjhdfguij");
            //Thread.Sleep(2000);
        }
    }
}
