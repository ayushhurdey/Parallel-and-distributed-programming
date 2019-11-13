using System.Collections.Generic;
using PPD_LAB5.implementation;

namespace PPD_LAB5
{
    public static class Program
    {
        private static readonly List<string> Hosts = new List<string>
        {
            "www.google.com",
            "www.ebay.co.uk",
            "www.amazon.de"
        };

        public static void Main()
        {
            //DirectCallbacks.Run(Hosts);
            TaskMechanism.Run(Hosts);
            //AsyncTaskMechanism.Run(Hosts);
        }
    }
}