using System;
using System.Collections.Generic;
using System.Text;

namespace Lab5Http
{
    class Parser
    {
       
        public static int getLength(string content)
        {
            int l = 0;
            string[] lines = content.Split('\r', '\n');
            foreach (string line in lines)
            { 
                var header = line.Split(':');

                if (header[0].Equals("Content-Length"))
                {
                    l = int.Parse(header[1]);
                }
            }
            return l ;
        }
        public static string getBody(string content)
        {
            string[] response = content.Split("\r\n\r\n", StringSplitOptions.RemoveEmptyEntries);
            if (response.Length > 1)
                return response[1];
            return "";
        }

        public static bool responseDone(string content)
        {
            return content.Contains("\r\n\r\n");
        }
    }
}
