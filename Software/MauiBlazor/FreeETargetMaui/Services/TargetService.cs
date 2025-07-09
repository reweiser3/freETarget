using System.IO.Ports;
using System.Text;

namespace FreeETargetMaui.Services;

public class TargetService
{
    private readonly StringBuilder _buffer = new();
    private SerialPort? _port;

    public event Action<Shot>? ShotReceived;

    public void Connect(string portName, int baudRate = 115200)
    {
        if (_port != null && _port.IsOpen)
            return;

        _port = new SerialPort(portName, baudRate)
        {
            DtrEnable = true,
            RtsEnable = true,
            NewLine = "\n"
        };
        _port.DataReceived += OnDataReceived;
        _port.Open();
        _port.Write("*");
    }

    public void Disconnect()
    {
        if (_port == null) return;
        _port.DataReceived -= OnDataReceived;
        if (_port.IsOpen)
            _port.Close();
        _port.Dispose();
        _port = null;
    }

    private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        if (_port == null) return;
        var data = _port.ReadExisting();
        _buffer.Append(data);
        string text = _buffer.ToString();
        int idx;
        while ((idx = text.IndexOf('}')) >= 0)
        {
            int open = text.IndexOf('{');
            if (open >= 0 && open < idx)
            {
                string json = text.Substring(open + 1, idx - open - 1);
                if (Shot.TryParse(json, out var shot) && shot != null)
                {
                    ShotReceived?.Invoke(shot);
                }
                text = text[(idx + 1)..];
                _buffer.Clear();
                _buffer.Append(text);
            }
            else
            {
                break;
            }
        }
    }
}
