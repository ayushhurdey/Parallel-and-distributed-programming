using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace Lab5Http
{
    class ConditionalEvent
    {
        private object lck = new object();
        private bool notified;

        public ConditionalEvent()
        {
            this.notified = false;
        }

        public virtual void Notify()
        {
            lock (lck)
            {
                if (!notified)
                {
                    notified = true;
                    Monitor.Pulse(lck);
                }
            }
        }

        public bool Wait()
        {
            lock (lck)
            {
                if (!notified)
                {
                    Monitor.Wait(lck);
                }
            }
            return true;
        }
    }
}
