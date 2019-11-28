using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Lab5Http
{
    class CallBack
    {
        List<string> servers;
        int serverNo = 0;
        public CallBack(List<string> servers)
        {
            this.servers = servers;
        }


        public void run()
        {
            foreach (var server in servers)
            {
                runServer(serverNo++, server);

            }
        }

        private void runServer(int serverNo, string server)
        {

            IPHostEntry iPHostEntry = Dns.GetHostEntry(server);
            IPAddress iPAddress = iPHostEntry.AddressList[0];
            IPEndPoint iPEndPoint = new IPEndPoint(iPAddress, 80);
            Socket client =
                new Socket(iPAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            Response response = new Response(serverNo, client, server);
            client.BeginConnect(iPEndPoint, onConnect, response);

            
        }

        private void onConnect(IAsyncResult result)
        {
            Response response = (Response)result.AsyncState;
            response.socket.EndConnect(result);
            Console.WriteLine("Connected id:" + response.id + " server:" + response.server);
            byte[] connString = Encoding.ASCII.GetBytes("GET / HTTP/1.0\r\n\r\n");
            response.socket.BeginSend(connString, 0, connString.Length, SocketFlags.None, onSend, response);
        }

        private void onSend(IAsyncResult result)
        {
            Response response = (Response)result.AsyncState;
            int sent = response.socket.EndSend(result);
            Console.WriteLine("Sent " + sent + " bytes to: " + response.server);
            response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
        }

        private void onReceive(IAsyncResult result)
        {
            Response response = (Response)result.AsyncState;

            int read = response.socket.EndReceive(result);
            response.stringContent += Encoding.ASCII.GetString(response.byteContent, 0, 1000);
            if (!Parser.responseDone(response.stringContent))
                response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
            else
            {

                int bLength = Parser.getBody(response.stringContent).Length;
                int hLength = Parser.getLength(response.stringContent);
                if (bLength < hLength)
                    response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
                else
                {
                    Console.WriteLine("Id:" + response.id + " expected:" + hLength + " got:" + bLength);
                    response.socket.Shutdown(SocketShutdown.Both);
                    response.socket.Close();
                }
            }

        }
    }
}
