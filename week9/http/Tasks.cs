using System;
using System.Collections.Generic;
using System.Text;

namespace Lab5Http
{
    class Tasks
    {
        List<TaskCall> tasks;

        public Tasks(List<TaskCall> tasks)
        {
            this.tasks = tasks;
        }

        public void run()
        {
            foreach (TaskCall task in tasks)
            {
                task.runTask();
            }
        }
    }
}
