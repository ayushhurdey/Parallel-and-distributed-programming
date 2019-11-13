using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace PPD_LAB5.model
{
    public class StateObject
    {
        public const int BufferSize = 512;
        public readonly byte[] Buffer = new byte[BufferSize];
        public readonly ManualResetEvent ConnectDone = new ManualResetEvent(false);
        public readonly ManualResetEvent ReceiveDone = new ManualResetEvent(false);
        public readonly StringBuilder ResponseContent = new StringBuilder();
        public readonly ManualResetEvent SendDone = new ManualResetEvent(false);
        public string Endpoint;
        public string Hostname;
        public int Id;
        public IPEndPoint RemoteEndpoint;
        public Socket Socket = null;
    }
}