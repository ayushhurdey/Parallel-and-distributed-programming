using System;
using System.Collections.Generic;

namespace Lab5Http
{
    class Program
    {
        static void Main(string[] args)
        {
            List<string> servers = new List<String>();
            servers.Add("www.scs.ubbcluj.ro");
            servers.Add("www.cs.ubbcluj.ro");
            servers.Add("www.math.ubbcluj.ro");
            CallBack callBack = new CallBack(servers);
            callBack.run();

            List<TaskCall> tasks=new List<TaskCall>();
            for(int i = 0; i < servers.Count; i++)
            {
                tasks.Add(new TaskCall(servers[i], i));
            }

            Tasks tasksToRun = new Tasks(tasks);
            tasksToRun.run();

            AsyncCall asyncCall = new AsyncCall(servers);
            asyncCall.run();
        }
    }
}
