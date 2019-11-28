using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Lab5Http
{
    class TaskCall
    {
        TaskResponse taskResponse;
        IPEndPoint iPEnd;
        public TaskCall(string server,int id)
        {
            IPHostEntry iPHostEntry = Dns.GetHostEntry(server);
            IPAddress iPAddress = iPHostEntry.AddressList[0];
            IPEndPoint iPEndPoint = new IPEndPoint(iPAddress, 80);
            iPEnd = iPEndPoint;
            Socket client =
                new Socket(iPAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
             taskResponse = new TaskResponse(id, client, server);
        }


        public void runTask()
        {

            connectTask(taskResponse, iPEnd).ContinueWith((task) =>
            {
                sendTask().ContinueWith((task)=> {
                    receiveTask().ContinueWith((task)=> {
                        int bLength = Parser.getBody(taskResponse.stringContent).Length;
                        int hLength = Parser.getLength(taskResponse.stringContent);
                        Console.WriteLine("Id:" + taskResponse.id + " expected:" + hLength + " got:" + bLength);
                        taskResponse.socket.Shutdown(SocketShutdown.Both);
                        taskResponse.socket.Close();
                    });
                });
            }); ;

            
        }

        private Task connectTask(TaskResponse response,IPEndPoint iPEndPoint)
        {
            response.socket.BeginConnect(iPEndPoint, onConnect, response);

            return Task.FromResult(response.connectEvent.Wait());
        }

        private void onConnect(IAsyncResult result)
        {
            TaskResponse response = (TaskResponse)result.AsyncState;
            response.socket.EndConnect(result);
            Console.WriteLine("Connected id:" + response.id + " server:" + response.server); 
            response.connectEvent.Notify();
        }

        private Task sendTask()
        {
            string connString = "GET / HTTP/1.0\r\n\r\n";
            var bytes = Encoding.ASCII.GetBytes(connString);
            taskResponse.socket.BeginSend(bytes, 0, bytes.Length, SocketFlags.None, onSend, taskResponse);
            return Task.FromResult(taskResponse.sendEvent.Wait());
        }

        private void onSend(IAsyncResult result)
        {
            TaskResponse response = (TaskResponse)result.AsyncState;
            int sent = response.socket.EndSend(result);
            Console.WriteLine("Sent " + sent + " bytes to: " + response.server);
            response.sendEvent.Notify();
        }

        private Task receiveTask()
        {
            taskResponse.socket.BeginReceive(taskResponse.byteContent, 0, 1000, SocketFlags.None, onReceive, taskResponse);
            return Task.FromResult(taskResponse.receiveEvent.Wait());
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
