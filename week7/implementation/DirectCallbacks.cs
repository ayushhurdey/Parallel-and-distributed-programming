using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using PPD_LAB5.model;
using PPD_LAB5.util;

namespace PPD_LAB5.implementation
{
    public static class DirectCallbacks
    {
        private static List<string> _hosts;

        public static void Run(List<string> hostnames)
        {
            _hosts = hostnames;

            for (var i = 0; i < _hosts.Count; i++)
            {
                DoStart(i);

                Thread.Sleep(5000);
            }
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

            state.Socket.BeginConnect(state.RemoteEndpoint, Connected, state);
        }

        private static void Connected(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;
            var clientId = state.Id;
            var hostname = state.Hostname;

            clientSocket.EndConnect(ar);
            Console.WriteLine("{0} --> Socket connected to {1} ({2})", clientId, hostname, clientSocket.RemoteEndPoint);

            var byteData = Encoding.ASCII.GetBytes(HttpUtils.GetRequestString(state.Hostname, state.Endpoint));

            state.Socket.BeginSend(byteData, 0, byteData.Length, 0, Sent, state);
        }

        private static void Sent(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;
            var clientId = state.Id;

            var bytesSent = clientSocket.EndSend(ar);
            Console.WriteLine("{0} --> Sent {1} bytes to server.", clientId, bytesSent);

            state.Socket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, Receiving, state);
        }

        private static void Receiving(IAsyncResult ar)
        {
            var state = (StateObject) ar.AsyncState;
            var clientSocket = state.Socket;
            var clientId = state.Id;

            try
            {
                var bytesRead = clientSocket.EndReceive(ar);

                state.ResponseContent.Append(Encoding.ASCII.GetString(state.Buffer, 0, bytesRead));

                if (!HttpUtils.ResponseHeaderFullyObtained(state.ResponseContent.ToString()))
                {
                    clientSocket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, Receiving, state);
                }
                else
                {
                    var responseBody = HttpUtils.GetResponseBody(state.ResponseContent.ToString());

                    var contentLengthHeaderValue = HttpUtils.GetContentLength(state.ResponseContent.ToString());
                    if (responseBody.Length < contentLengthHeaderValue)
                    {
                        clientSocket.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0, Receiving, state);
                    }
                    else
                    {
                        foreach (var i in state.ResponseContent.ToString().Split('\r', '\n'))
                            Console.WriteLine(i);
                        Console.WriteLine(
                            "{0} --> Response received : expected {1} chars in body, got {2} chars (headers + body)",
                            clientId, contentLengthHeaderValue, state.ResponseContent.Length);

                        clientSocket.Shutdown(SocketShutdown.Both);
                        clientSocket.Close();
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
    }
}