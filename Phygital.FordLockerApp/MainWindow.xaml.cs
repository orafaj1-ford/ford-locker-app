using Newtonsoft.Json;
using Phygital.FordLockerApp.Business.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Text;
using System.Windows;

namespace Phygital.FordLockerApp
{
    public partial class MainWindow : Window
    {
        private int _port = 9600;

        SerialPort serialPort = new SerialPort();

        bool readingCommand = false;
        StringBuilder commandBuffer = new StringBuilder();
        List<RFIDDataItem> dataItems = new List<RFIDDataItem>();
        public MainWindow()
        {
            InitializeComponent();           
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
                serialPort.PortName = portName;
                serialPort.BaudRate = _port;
                serialPort.Open();
                serialPort.DataReceived += new SerialDataReceivedEventHandler(SerialPort_DataReceived);
                serialPort.WriteLine("START");
                Status.Text = $"Connected{Environment.NewLine}";
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
                var content = serialPort.ReadExisting();

                var chars = content.ToArray();
                foreach (var chr in chars)
                {                           
                    if (chr == ']')
                    {
                        readingCommand = false;
                        var command = commandBuffer.ToString();
                        RunCommand(command);
                        commandBuffer.Clear();
                    }
                    if (readingCommand)
                    {
                        commandBuffer.Append(chr);
                    }
                    if (chr == '[')
                    {
                        readingCommand = true;
                    }
                }

                Status.Text += content;
            });
        }

        private void RunCommand(string commandVals)
        {
            if (commandVals.StartsWith("RFIDFAIL"))
            {
                
            }
            else if (commandVals.StartsWith("RFIDOK"))
            {
                var dataId = commandVals.Split(":").Last();
                var dataItem = dataItems.FirstOrDefault(x => x.DataId == dataId);
                if (dataItem == null) {
                };
                serialPort.WriteLine($"o{dataItem.DoorId}");


            }
            else if (commandVals.StartsWith("DOOR"))
            {
                
            }
        }

        private void Retry_Click(object sender, RoutedEventArgs e)
        {
            Retry.IsEnabled = false;
            StartArduino();            
        }


        private void StartArduino()
        {
            if (serialPort.IsOpen) return;

            LoadRFIDData();

            Retry.Visibility = Visibility.Hidden;
            Retry.IsEnabled = false;
            
            var portName = AutodetectArduinoPort();
            if (portName == null)
            {
                Status.Text = "No Arduino Detected - Check Connection and Drivers!";
                Retry.Visibility = Visibility.Visible;
                Retry.IsEnabled = true;
                return;
            }
            
            var connected = StartListeningToArduino(portName);
            if (!connected)
            {
                Status.Text = "Ardunio Detected But Could Not Connect - Check Port and Connection!";
                Retry.Visibility = Visibility.Visible;
                Retry.IsEnabled = true;
                return;
            }

            serialPort.Write("0");
        }

        private void LoadRFIDData()
        {
            var json = File.ReadAllText("cards.json");
            dataItems = JsonConvert.DeserializeObject<RFIDDataItem[]>(json).ToList();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
                Status.Text = "Disconnected";
            }
        }

        private void ShowFailRFIDScanText()
        {

        }

        private void ShowSuccessRFIDScanText(int door)
        {

        }

    }
}
