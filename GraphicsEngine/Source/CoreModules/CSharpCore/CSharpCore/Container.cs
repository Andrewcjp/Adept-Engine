using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CSharpCore
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct CSObjectCreationArgs
    {
        public int ClassId;
    };

    public class Container
    {
        List<ObjectBase> Objects = new List<ObjectBase>();
        List<Type> Types = new List<Type>();
        static Container Instance;
        public static void Init()
        {
            Instance = new Container();
            Console.WriteLine("C# Container Init");
            try
            {
                Assembly a = Assembly.Load("CSTestGame");
                Console.WriteLine(a.FullName);
                Type myType = a.GetType("CSTestGame.CSTestGame.Module", true, true);
                MethodInfo myMethod = myType.GetMethod("Init");
                object obj = Activator.CreateInstance(myType);
                //// Execute the method.
                myMethod.Invoke(obj, null);
            }
            catch (System.Exception E)
            {
                Console.WriteLine("ERROR" + E.Message);
            }

            string Name = " " + EngineVersion.GetFullVersionString();
            Console.WriteLine(Name);
           
        }
        Container()
        {
            Objects.Add(new ObjectBase());
        }
        public static void Tick()
        {
            foreach (ObjectBase B in Instance.Objects)
            {
                B.Update();
            }
        }
        public static void ShutDown()
        {

        }
        public static int CreateObject(ref CSObjectCreationArgs args)
        {
            Console.WriteLine("Creating object with ID " + args.ClassId);
            return args.ClassId;
        }
        public static void AddType(Type T)
        {
            Console.WriteLine("Type: " + T.Name);
            Instance.Types.Add(T);
        }
    }
}
