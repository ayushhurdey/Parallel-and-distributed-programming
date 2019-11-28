using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Lab5Http
{
    class AsyncCall
    {
        List<string> servers;
        int serverNo = 0;
        public AsyncCall(List<string> servers)
        {
            this.servers = servers;
        }


        public void run()
        {
            foreach (var server in servers)
            {
                runTask(serverNo++, server);
            }
        }

        private async void runTask(int id, string server)
        {
            IPHostEntry iPHostEntry = Dns.GetHostEntry(server);
            IPAddress iPAddress = iPHostEntry.AddressList[0];
            IPEndPoint iPEndPoint = new IPEndPoint(iPAddress, 80);
            Socket client =
                new Socket(iPAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            TaskResponse response = new TaskResponse(serverNo, client, server);
            await connectTask(response, iPEndPoint);

            await sendTask(response, "GET / HTTP/1.0\r\n\r\n");

            await receiveTask(response);
            int bLength = Parser.getBody(response.stringContent).Length;
            int hLength = Parser.getLength(response.stringContent);
            Console.WriteLine("Id:" + response.id + " expected:" + hLength + " got:" + bLength);
            response.socket.Shutdown(SocketShutdown.Both);
            response.socket.Close();
        }

        private async Task connectTask(TaskResponse response, IPEndPoint iPEndPoint)
        {
            response.socket.BeginConnect(iPEndPoint, onConnect, response);

            await Task.FromResult(response.connectEvent.Wait());
        }

        private void onConnect(IAsyncResult result)
        {
            TaskResponse response = (TaskResponse)result.AsyncState;
            response.socket.EndConnect(result);
            Console.WriteLine("Connected id:" + response.id + " server:" + response.server);
            response.connectEvent.Notify();
        }

        private async Task sendTask(TaskResponse response, string connString)
        {
            var bytes = Encoding.ASCII.GetBytes(connString);
            response.socket.BeginSend(bytes, 0, bytes.Length, SocketFlags.None, onSend, response);
            await Task.FromResult(response.sendEvent.Wait());
        }

        private void onSend(IAsyncResult result)
        {
            TaskResponse response = (TaskResponse)result.AsyncState;
            int sent = response.socket.EndSend(result);
            Console.WriteLine("Sent " + sent + " bytes to: " + response.server);
            response.sendEvent.Notify();
        }

        private async Task receiveTask(TaskResponse response)
        {
            response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
            await Task.FromResult(response.receiveEvent.Wait());
        }

        private void onReceive(IAsyncResult result)
        {
            TaskResponse response = (TaskResponse)result.AsyncState;
            int read = response.socket.EndReceive(result);
            response.stringContent += Encoding.ASCII.GetString(response.byteContent, 0, read);
            if (!Parser.responseDone(response.stringContent))
            {
                response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
            }
            else
            {
                int bLength = Parser.getBody(response.stringContent).Length;
                int hLength = Parser.getLength(response.stringContent);
                if (bLength < hLength)
                    response.socket.BeginReceive(response.byteContent, 0, 1000, SocketFlags.None, onReceive, response);
                else
                {
                    response.receiveEvent.Notify();
                }
            }


        }
    }
}
