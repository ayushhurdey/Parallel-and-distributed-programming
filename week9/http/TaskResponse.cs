using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Lab5Http
{
    class TaskResponse : Response
    {
        public ConditionalEvent connectEvent = new ConditionalEvent();
        public ConditionalEvent sendEvent = new ConditionalEvent();
        public ConditionalEvent receiveEvent = new ConditionalEvent();

        public TaskResponse(int id, Socket socket, string server) : base(id, socket, server)
        {
        }
    }
}
