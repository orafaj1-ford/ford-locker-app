using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Phygital.FordLockerApp.Business.Models
{
    [Serializable]
    public class RFIDDataItem
    {
        public string DataId { get; set; }
        public int DoorId { get; set; }
    }
}
