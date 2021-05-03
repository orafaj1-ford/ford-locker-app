using Newtonsoft.Json;
using Phygital.FordLockerApp.Business;
using Phygital.FordLockerApp.Business.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Text;
using System.Windows;
using System.Windows.Threading;

namespace Phygital.FordLockerApp
{
    public partial class MainWindow : Window
    {
        private int _port = 9600;
        private SerialPort _serialPort = new SerialPort();
        private bool _readingCommand = false;
        private StringBuilder _commandBuffer = new StringBuilder();
        private List<RFIDDataItem> _dataItems = new List<RFIDDataItem>();
        private DispatcherTimer _timer = new DispatcherTimer();

        public MainWindow()
        {
            InitializeComponent();
            _timer.Tick += Timer_Tick;
        }

        private string AutodetectArduinoPort()
        {
            var connectionScope = new ManagementScope();
            var serialQuery = new SelectQuery("SELECT * FROM Win32_SerialPort");
            var searcher = new ManagementObjectSearcher(connectionScope, serialQuery);
            
            try
            {
                foreach (var item in searcher.Get())
                {
                    string desc = item["Description"].ToString();
                    string deviceId = item["DeviceID"].ToString();

                    if (desc.Contains("Arduino"))
                    {
                        return deviceId;
                    }
                }
            }
            catch (ManagementException)
            {
                /* Do Nothing */
            }

            return null;
        }  

        private void Window_Activated(object sender, EventArgs e)
        {
            StartArduino();
        }

        private bool StartListeningToArduino(string portName)
        {
            try
            {
                _timer.Interval = TimeSpan.FromSeconds(5);
             
                _serialPort.PortName = portName;
                _serialPort.BaudRate = _port;
                _serialPort.Open();
                _serialPort.DataReceived += new SerialDataReceivedEventHandler(SerialPort_DataReceived);
                _serialPort.WriteLine("START");
                Status.Text = $"Connected{Environment.NewLine}";
                ShowText(TextItem.Intro);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                if (e.EventType == SerialData.Eof) return;
                var content = _serialPort.ReadExisting();

                var chars = content.ToArray();
                foreach (var chr in chars)
                {                           
                    if (chr == ']')
                    {
                        _readingCommand = false;
                        var command = _commandBuffer.ToString();
                        RunCommand(command);
                        _commandBuffer.Clear();
                    }
                    if (_readingCommand)
                    {
                        _commandBuffer.Append(chr);
                    }
                    if (chr == '[')
                    {
                        _readingCommand = true;
                    }
                }

                Status.Text += content;
            });
        }

        private void RunCommand(string commandVals)
        {
            if (commandVals.StartsWith("RFIDFAIL"))
            {
                ShowText(TextItem.RFIDError);
                StopTimer();
                StartTimer();
            }
            else if (commandVals.StartsWith("RFIDOK"))
            {
                var dataId = commandVals.Split(":").Last();
                var dataItem = _dataItems.FirstOrDefault(x => x.DataId == dataId);
                if (dataItem == null) {
                };
                _serialPort.WriteLine($"o{dataItem.DoorId}");
                ShowText(TextItem.DoorOpened, dataItem.DoorId + 1);
                StopTimer();
                StartTimer();
            }
            else if (commandVals.StartsWith("DOOR"))
            {
                var doorId = int.Parse(commandVals.Split(":").Last());
                ShowText(TextItem.DoorLeftOpenError, doorId + 1);
                StopTimer();
                StartTimer();
            }
        }

        private void StartArduino()
        {
            if (_serialPort.IsOpen) return;

            LoadRFIDData();
            var portName = AutodetectArduinoPort();
            if (portName == null)
            {
                ShowText(TextItem.ArduinoNotFoundError);
                return;
            }
            
            var connected = StartListeningToArduino(portName);
            if (!connected)
            {
                ShowText(TextItem.ArduinoConnectionError);
                return;
            }

            _serialPort.Write("0");
        }

        private void LoadRFIDData()
        {
            var json = File.ReadAllText("cards.json");
            _dataItems = JsonConvert.DeserializeObject<RFIDDataItem[]>(json).ToList();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_serialPort.IsOpen)
            {
                _serialPort.Close();
                Status.Text = "Disconnected";
            }
        }

        private void ShowText(TextItem textItem, int? door = null)
        {            
            Intro.Visibility = textItem == TextItem.Intro ? Visibility.Visible : Visibility.Hidden;
            RFIDFail.Visibility = textItem == TextItem.RFIDError ? Visibility.Visible : Visibility.Hidden;
            DoorOpened.Visibility = textItem == TextItem.DoorOpened ? Visibility.Visible : Visibility.Hidden;
            DoorLeftOpen.Visibility = textItem == TextItem.DoorLeftOpenError ? Visibility.Visible : Visibility.Hidden;
            ArduinoNotFoundError.Visibility = textItem == TextItem.ArduinoNotFoundError ? Visibility.Visible : Visibility.Hidden;
            ArduinoConnectionError.Visibility = textItem == TextItem.ArduinoConnectionError ? Visibility.Visible : Visibility.Hidden;
            DoorOpenedDoorId.Text = door.ToString();
            DoorLeftOpenDoorId.Text = door.ToString();
        }
     
        private void Timer_Tick(object sender, EventArgs e)
        {
            ShowText(TextItem.Intro);
            StopTimer();
        }

        private void StartTimer()
        {            
            _timer.Start();
        }

        private void StopTimer()
        {
            _timer.Stop();
         }
    }
}
