using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using PPD_LAB5.model;
using PPD_LAB5.util;

namespace PPD_LAB5.implementation
{
    public static class TaskMechanism
    {
        private static List<string> _hosts;

        public static void Run(List<string> hostnames)
        {
            _hosts = hostnames;

            var tasks = new List<Task>();

            for (var i = 0; i < _hosts.Count; i++) tasks.Add(Task.Factory.StartNew(DoStart, i));

            Task.WaitAll(tasks.ToArray());
        }

        private static void DoStart(object idObject)
        {
            var id = (int) idObject;

            StartClient(_hosts[id], id);
        }

        private static void StartClient(string host, int id)
        {
            var ipHostInfo = Dns.GetHostEntry(host.Split('/')[0]);
            var ipAddress = ipHostInfo.AddressList[0];
            var remoteEndpoint = new IPEndPoint(ipAddress, HttpUtils.HttpPort);
            var client = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            var state = new StateObject
            {
                Socket = client,
                Hostname = host.Split('/')[0],
                Endpoint = host.Contains("/") ? host.Substring(host.IndexOf("/", StringComparison.Ordinal)) : "/",
                RemoteEndpoint = remoteEndpoint,
                Id = id
            };

            ConnectWrapper(state).Wait();

            SendWrapper(state, HttpUtils.GetRequestString(state.Hostname, state.Endpoint)).Wait();

            ReceiveWrapper(state).Wait();

            Console.WriteLine(
                "{0}) Response received : expected {1} chars in body, got {2} chars (headers + body)",
                id, HttpUtils.GetContentLength(state.ResponseContent.ToString()), state.ResponseContent.Length);

            client.Shutdown(SocketShutdown.Both);
            client.Close();
        }

        private static Task ConnectWrapper(StateObject state)
        {
            state.Socket.BeginConnect(state.RemoteEndpoint, ConnectCallback, state);

            return Task.FromResult(state.ConnectDone.WaitOne());
        }

        private static void ConnectCallback(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;
            var clientId = state.Id;
            var hostname = state.Hostname;

            clientSocket.EndConnect(ar);

            Console.WriteLine("{0} --> Socket connected to {1} ({2})", clientId, hostname, clientSocket.RemoteEndPoint);

            state.ConnectDone.Set();
        }

        private static Task SendWrapper(StateObject state, string data)
        {
            var byteData = Encoding.ASCII.GetBytes(data);

            state.Socket.BeginSend(byteData, 0, byteData.Length, 0, SendCallback, state);

            return Task.FromResult(state.SendDone.WaitOne());
        }

        private static void SendCallback(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;
            var clientId = state.Id;

            var bytesSent = clientSocket.EndSend(ar);
            Console.WriteLine("{0} --> Sent {1} bytes to server.", clientId, bytesSent);

            state.SendDone.Set();
        }

        private static Task ReceiveWrapper(StateObject state)
        {
            state.Socket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);

            return Task.FromResult(state.ReceiveDone.WaitOne());
        }

        private static void ReceiveCallback(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;

            try
            {
                var bytesRead = clientSocket.EndReceive(ar);

                state.ResponseContent.Append(Encoding.ASCII.GetString(state.Buffer, 0, bytesRead));

                if (!HttpUtils.ResponseHeaderFullyObtained(state.ResponseContent.ToString()))
                {
                    clientSocket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
                }
                else
                {
                    var responseBody = HttpUtils.GetResponseBody(state.ResponseContent.ToString());

                    foreach (var i in state.ResponseContent.ToString().Split('\r', '\n'))
                        Console.WriteLine(i);

                    if (responseBody.Length < HttpUtils.GetContentLength(state.ResponseContent.ToString()))
                        clientSocket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
                    else
                        state.ReceiveDone.Set();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
    }
}