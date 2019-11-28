using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;

namespace Lab5Http
{
    class Response
    {
        public int id;
        public Socket socket;
        public string server;
        public byte[] byteContent = new byte[1000];
        public string stringContent="";

        public Response(int id,Socket socket, string server)
        {
            this.id = id;
            this.socket = socket;
            this.server = server;
        }

        
    }
}
