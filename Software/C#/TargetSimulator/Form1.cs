using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using TargetSimulator.Properties;

namespace TargetSimulator
{

    public partial class Form1 : Form
    {
        public const decimal distanceBetweenSensors = 160; //in milimiters

        private int range = (int)Math.Round(distanceBetweenSensors * 10 / 2, 0);
        bool isConnected = false;
        int count = 1;
        String[] ports;
        
        // Default shooter properties for simulation
        private const double DEFAULT_SHOOTER_SKILL = 0.8; // 0.0 to 1.0 where 1.0 is perfect
        private const double DEFAULT_BREATHING_STRENGTH = 0.3; // How much breathing affects shots
        private const double DEFAULT_MISS_CHANCE = 0.005; // Probability of a complete miss
        
        // Shooter properties for realistic simulation
        private Random random = new Random();
        private double shooterSkill = DEFAULT_SHOOTER_SKILL;
        private decimal centerOffsetX = 0; // Shooter's natural aim offset X
        private decimal centerOffsetY = 0; // Shooter's natural aim offset Y
        private int breathingPhase = 0; // 0-100 for breathing cycle
        private bool breathingEnabled = true;
        private double breathingStrength = DEFAULT_BREATHING_STRENGTH;
        private int consecutiveShots = 0; // Track shots in sequence
        private DateTime lastShotTime = DateTime.Now;
        private double missChance = DEFAULT_MISS_CHANCE;
        private bool settingsChanged = false;


        public Form1()
        {
            InitializeComponent();

            refreshAvailablePorts();
            
            // Initialize UI controls with default values
            InitializeShooterSettingsControls();
            
            // Wire up event handlers manually
            WireUpEventHandlers();
        }
        
        private void WireUpEventHandlers()
        {
            // Connect the event handlers for shooter settings
            trkShooterSkill.ValueChanged += trkShooterSkill_ValueChanged;
            trkBreathingStrength.ValueChanged += trkBreathingStrength_ValueChanged;
            trkMissChance.ValueChanged += trkMissChance_ValueChanged;
            numXOffset.ValueChanged += numXOffset_ValueChanged;
            numYOffset.ValueChanged += numYOffset_ValueChanged;
            chkBreathingEnabled.CheckedChanged += chkBreathingEnabled_CheckedChanged;
            btnApplySettings.Click += btnApplySettings_Click;
            btnResetToDefault.Click += btnResetToDefault_Click;
            btnRefreshPorts.Click += btnRefreshPorts_Click;
        }
        
        private void InitializeShooterSettingsControls()
        {
            // Set initial values for controls
            trkShooterSkill.Value = (int)(shooterSkill * 100);
            trkBreathingStrength.Value = (int)(breathingStrength * 100);
            trkMissChance.Value = (int)(missChance * 1000);
            numXOffset.Value = centerOffsetX;
            numYOffset.Value = centerOffsetY;
            chkBreathingEnabled.Checked = breathingEnabled;
            
            // Disable Apply button until changes are made
            btnApplySettings.Enabled = false;
            
            // Set initial label values
            lblShooterSkill.Text = $"Shooter Skill: {shooterSkill:F2}";
            lblBreathingStrength.Text = $"Breathing: {breathingStrength:F2}";
            lblMissChance.Text = $"Miss: {missChance:P1}";
        }

        void refreshAvailablePorts()
        {
            // Store the previously selected port if any
            string previousPort = comboBox1.SelectedItem?.ToString() ?? String.Empty;
            
            // Clear the combo box and get fresh port names
            comboBox1.Items.Clear();
            ports = SerialPort.GetPortNames();
            
            if (ports.Length == 0)
            {
                // No ports available
                statusText.Text = "No COM ports detected";
                txtOutput.AppendText($"{DateTime.Now}: No COM ports available\r\n");
                return;
            }
            
            // Add the new ports to the combo box
            foreach (string port in ports)
            {
                comboBox1.Items.Add(port);
                Console.WriteLine(port);
            }
            
            // Attempt to reselect the previously selected port
            if (!String.IsNullOrEmpty(previousPort) && comboBox1.Items.Contains(previousPort))
            {
                comboBox1.SelectedItem = previousPort;
            }
            else if (comboBox1.Items.Count > 0)
            {
                comboBox1.SelectedIndex = 0; // Select the first port
            }
            
            txtOutput.AppendText($"{DateTime.Now}: Detected {ports.Length} COM ports\r\n");
        }

        private void btnRefreshPorts_Click(object sender, EventArgs e)
        {
            refreshAvailablePorts();
            statusText.Text = "Port list refreshed";
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            Console.WriteLine("Range " + range);
            if (isConnected == false)
            {
                try
                {
                    string selectedPort = comboBox1.GetItemText(comboBox1.SelectedItem);
                    if (string.IsNullOrEmpty(selectedPort))
                    {
                        statusText.Text = "Error: No COM port selected";
                        MessageBox.Show("Please select a COM port from the dropdown list.", "COM Port Required", 
                            MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        return;
                    }
                    
                    serialPort1.PortName = selectedPort;
                    serialPort1.BaudRate = Settings.Default.BaudRate; 
                    serialPort1.WriteTimeout = 1000; // Increase timeout to 1 second
                    serialPort1.ReadTimeout = 1000;  // Add read timeout
                    serialPort1.Handshake = Handshake.None;
                    serialPort1.Parity = Parity.None;
                    serialPort1.DataBits = 8;
                    serialPort1.StopBits = StopBits.One;
                    
                    // Display connection parameters for troubleshooting
                    txtOutput.AppendText($"{DateTime.Now}: Attempting to connect to {selectedPort} with BaudRate={serialPort1.BaudRate}\r\n");
                    
                    serialPort1.Open();
                    btnConnect.Text = "Disconnect";
                    isConnected = true;
                    btnTimer.Enabled = true;
                    btnShot.Enabled = true;

                    btnBottom.Enabled = true;
                    btnCenter.Enabled = true;
                    btnHalfway.Enabled = true;
                    btnLeft.Enabled = true;
                    btnRight.Enabled = true;
                    btnTop.Enabled = true;
                    btnTopRight.Enabled = true;
                    btnShoot.Enabled = true;
                    btnImport.Enabled = true;
                    btnImportLog.Enabled = true;
                    btnMiss.Enabled = true;

                    statusText.Text = "Connected to " + selectedPort;
                    count = 1;

                    // Send initial connection message
                    txtOutput.AppendText($"{DateTime.Now}: Connected successfully to {selectedPort}\r\n");
                    serialPort1.WriteLine("freETarget Simulator" + Environment.NewLine);
                }
                catch (UnauthorizedAccessException)
                {
                    statusText.Text = "Error: Port in use by another application";
                    MessageBox.Show("The selected COM port is in use by another application. " +
                        "Close any other applications using this port and try again.", 
                        "Port In Use", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                catch (IOException)
                {
                    statusText.Text = "Error: Port does not exist";
                    MessageBox.Show("The selected COM port does not exist or is no longer available. " +
                        "Refresh the port list and try again.", "Port Not Available", 
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                catch (ArgumentException ex)
                {
                    statusText.Text = "Error: Invalid port settings";
                    MessageBox.Show($"Invalid port settings: {ex.Message}", 
                        "Configuration Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                catch (Exception ex)
                {
                    statusText.Text = "Connection error: " + ex.Message;
                    MessageBox.Show($"Error connecting to port: {ex.Message}", 
                        "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            } 
            else 
            {
                try
                {
                    serialPort1.Close();
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error closing port: " + ex.Message);
                }
                
                btnConnect.Text = "Connect";
                isConnected = false;
                timer1.Enabled = false;
                btnTimer.Enabled = false;
                btnShot.Enabled = false;

                btnBottom.Enabled = false;
                btnCenter.Enabled = false;
                btnHalfway.Enabled = false;
                btnLeft.Enabled = false;
                btnRight.Enabled = false;
                btnTop.Enabled = false;
                btnTopRight.Enabled = false;
                btnShoot.Enabled = false;
                btnImport.Enabled = false;
                btnImportLog.Enabled = false;
                btnMiss.Enabled = false;

                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";

                statusText.Text = "Disconnected";
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            // Calculate a realistic interval for the next shot
            SetRealisticShotInterval();
            
            // Generate realistic shot position based on shooter attributes
            var shotPosition = GenerateRealisticShot();
            decimal xPos = shotPosition.Item1;
            decimal yPos = shotPosition.Item2;
            
            // Decide whether this will be a miss
            if (random.NextDouble() < missChance && consecutiveShots > 5)
            {
                // Generate a miss based on miss probability
                generateMissAndSend();
                consecutiveShots = 0;
            }
            else
            {
                generateAndSend(xPos, yPos);
                consecutiveShots++;
                
                // Update breathing cycle
                if (breathingEnabled)
                {
                    breathingPhase = (breathingPhase + 5) % 100;
                }
            }
            
            lastShotTime = DateTime.Now;
        }
        
        private void SetRealisticShotInterval()
        {
            // Real shooters don't fire at perfectly random intervals
            // They tend to have a rhythm with some variation
            
            // Base timing depends on how many consecutive shots fired (fatigue)
            int baseInterval;
            double variability;
            
            if (consecutiveShots < 3)
            {
                // First few shots: deliberate, slower
                baseInterval = 3000;
                variability = 0.5; // 50% variation
            }
            else if (consecutiveShots < 10)
            {
                // Mid-sequence: rhythm established
                baseInterval = 2000;
                variability = 0.3; // 30% variation
            }
            else
            {
                // Later in sequence: faster but more variable (fatigue)
                baseInterval = 1500;
                variability = 0.4; // 40% variation
            }
            
            // Add randomness within the variability range
            double randomFactor = 1 - variability/2 + random.NextDouble() * variability;
            int interval = (int)(baseInterval * randomFactor);
            
            // Occasionally add a pause to simulate reloading or adjustments
            if (random.NextDouble() > 0.9 && consecutiveShots > 5)
            {
                interval += random.Next(3000, 8000);
                // Reset consecutive shots counter after a pause
                consecutiveShots = 0;
            }
            
            timer1.Interval = interval;
        }
        
        private Tuple<decimal, decimal> GenerateRealisticShot()
        {
            // A realistic shot takes into account:
            // 1. The shooter's skill level
            // 2. Natural tendencies/offset from center
            // 3. Breathing patterns
            // 4. Shooter fatigue

            // Calculate maximum deviation based on shooter skill
            double maxDeviation = range * (1.0 - shooterSkill * 0.9);
            
            // Base accuracy uses a normal distribution
            double deviation = NormalRandom(0, maxDeviation / 3.0);
            double angle = random.NextDouble() * 2 * Math.PI;
            
            decimal baseX = (decimal)(deviation * Math.Cos(angle));
            decimal baseY = (decimal)(deviation * Math.Sin(angle));
            
            // Apply shooter's natural tendency/offset
            baseX += centerOffsetX;
            baseY += centerOffsetY;
            
            // Apply breathing effect if enabled
            if (breathingEnabled)
            {
                // Breathing mostly affects vertical position
                double breathingEffect = Math.Sin(breathingPhase * Math.PI / 50) * breathingStrength;
                baseY += (decimal)(breathingEffect * maxDeviation * 0.5);
                baseX += (decimal)(breathingEffect * maxDeviation * 0.2); // Slight horizontal movement
            }
            
            // Apply fatigue effects for prolonged shooting sessions
            if (consecutiveShots > 10)
            {
                double fatigueFactor = Math.Min(consecutiveShots / 40.0, 0.5);
                double fatigueDeviation = range * 0.1 * fatigueFactor;
                baseX += (decimal)(random.NextDouble() * fatigueDeviation - fatigueDeviation/2);
                baseY += (decimal)(random.NextDouble() * fatigueDeviation - fatigueDeviation/2);
            }
            
            // Limit to range boundaries
            decimal xPos = Math.Max(Math.Min(baseX, range / 10m), -range / 10m);
            decimal yPos = Math.Max(Math.Min(baseY, range / 10m), -range / 10m);
            
            return new Tuple<decimal, decimal>(xPos, yPos);
        }
        
        // Helper method to generate normally distributed random numbers
        private double NormalRandom(double mean, double stdDev)
        {
            // Box-Muller transform
            double u1 = 1.0 - random.NextDouble(); // Uniform(0,1) random doubles
            double u2 = 1.0 - random.NextDouble();
            double randStdNormal = Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Sin(2.0 * Math.PI * u2);
            return mean + stdDev * randStdNormal;
        }

        void generateAndSend(decimal xPos, decimal yPos)
        {
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                statusText.Text = "Error: Not connected to port";
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
                return;
            }

            decimal radius = (decimal)pitagora(xPos, yPos);
            decimal angle = (decimal)findDegree((float)yPos, (float)xPos);

            //Console.WriteLine("X: " + xPos + " Y: " + yPos + " Radius: " + radius);

            string command = "{\"shot\":" + count + ", \"x\":" + xPos.ToString("F2", CultureInfo.InvariantCulture) + ", \"y\":" + yPos.ToString("F2", CultureInfo.InvariantCulture) + ", \"r\":" + radius.ToString("F2", CultureInfo.InvariantCulture) + ", \"a\":" + angle.ToString("F2", CultureInfo.InvariantCulture) +"}" + Environment.NewLine;
            
          
            txtOutput.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff") + " | ");
            txtOutput.AppendText(command);

            try
            {
                serialPort1.WriteLine(command);
            }
            catch(TimeoutException ex)
            {
                statusText.Text = "Error writing to port: (" + count + ") " + ex.Message;
                Console.WriteLine("ERROR: ("+ count +") " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
            }
            catch (InvalidOperationException ex)
            {
                statusText.Text = "Serial port error: " + ex.Message;
                Console.WriteLine("ERROR: Port closed unexpectedly: " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
                isConnected = false;
                btnConnect.Text = "Connect";
            }
            catch (Exception ex)
            {
                statusText.Text = "Error: " + ex.Message;
                Console.WriteLine("ERROR: General error: " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
            }

            count++;
        }

        void generateMissAndSend() {
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                statusText.Text = "Error: Not connected to port";
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
                return;
            }

            string command = "{\"shot\":0, \"miss\": 1,\"name\":\"BOSS\", \"x\": 0, \"y\": 0}" + Environment.NewLine;


            txtOutput.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff") + " | ");
            txtOutput.AppendText(command);

            try {
                serialPort1.WriteLine(command);
            }
            catch(TimeoutException ex)
            {
                statusText.Text = "Error writing to port: (" + count + ") " + ex.Message;
                Console.WriteLine("ERROR: ("+ count +") " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
            }
            catch (InvalidOperationException ex)
            {
                statusText.Text = "Serial port error: " + ex.Message;
                Console.WriteLine("ERROR: Port closed unexpectedly: " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
                isConnected = false;
                btnConnect.Text = "Connect";
            }
            catch (Exception ex)
            {
                statusText.Text = "Error: " + ex.Message;
                Console.WriteLine("ERROR: General error: " + ex.Message);
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
            }
        }

        public float findDegree(float x, float y)
        {
            float value = (float)((System.Math.Atan2(x, y) / System.Math.PI) * 180f);
            if (value < 0) value += 360f;
            return value;
        }

        double pitagora(decimal x, decimal y)
        {
            return Math.Sqrt(Math.Pow((double)Math.Abs(x), 2) + Math.Pow((double)Math.Abs(y), 2));
        }

        private void btnTimer_Click(object sender, EventArgs e)
        {
            if (timer1.Enabled==false)
            {
                // First check if still connected
                if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
                {
                    MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                        "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                
                // Reset simulation variables when starting a new session
                breathingPhase = 0;
                consecutiveShots = 0;
                lastShotTime = DateTime.Now;
                
                // If settings weren't manually applied, use current UI values
                if (settingsChanged)
                {
                    ApplySettingsFromUI();
                    settingsChanged = false;
                    btnApplySettings.Enabled = false;
                }
                
                // Set first shot interval
                timer1.Interval = random.Next(1000, 3000);
                timer1.Enabled = true;
                btnTimer.Text = "Stop Timer";
            } else {
                timer1.Enabled = false;
                btnTimer.Text = "Start Timer";
            }
            
        }

        private void btnShot_Click(object sender, EventArgs e)
        {
            // First check if still connected
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                    "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            var rand = new Random();
            decimal xPos = rand.Next(-range, range) / 10m;
            decimal yPos = rand.Next(-range, range) / 10m;
            generateAndSend(xPos, yPos);
        }

        private void btnCenter_Click(object sender, EventArgs e)
        {
            decimal xPos = 0;
            decimal yPos = 0;
            generateAndSend(xPos, yPos);
        }

        private void btnLeft_Click(object sender, EventArgs e)
        {
            decimal xPos = -range / 10m;
            decimal yPos = 0;
            generateAndSend(xPos, yPos);
        }

        private void btnTop_Click(object sender, EventArgs e)
        {
            decimal xPos = 0;
            decimal yPos = range / 10m;
            generateAndSend(xPos, yPos);
        }

        private void btnBottom_Click(object sender, EventArgs e)
        {
            decimal xPos = 0;
            decimal yPos = -range / 10m;
            generateAndSend(xPos, yPos);
        }

        private void btnRight_Click(object sender, EventArgs e)
        {
            decimal xPos = range / 10m;
            decimal yPos = 0;
            generateAndSend(xPos, yPos);
        }

        private void btnTopRight_Click(object sender, EventArgs e)
        {
            decimal xPos = range / 10m;
            decimal yPos = range / 10m;
            generateAndSend(xPos, yPos);
        }

        private void btnHalfway_Click(object sender, EventArgs e)
        {
            decimal xPos = range / 20m;
            decimal yPos = range / 20m;
            generateAndSend(xPos, yPos);
        }

        private void bthShoot_Click(object sender, EventArgs e)
        {
            // First check if still connected
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                    "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            try
            {
                decimal xPos = Decimal.Parse(txtX.Text, CultureInfo.InvariantCulture);
                decimal yPos = Decimal.Parse(txtY.Text, CultureInfo.InvariantCulture);
                generateAndSend(xPos, yPos);
            }catch(Exception ex)
            {
                Console.WriteLine("Parse error: " + ex.Message);
                MessageBox.Show("Invalid X or Y coordinates. Please enter valid decimal numbers.", 
                    "Input Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void btnImport_Click(object sender, EventArgs e) {
            // First check if still connected
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                    "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            openFileDialog.Filter = "TargetScan App file|*.csv";
            DialogResult r = openFileDialog.ShowDialog();
            if ( r == DialogResult.OK) {
                StreamReader sr = new StreamReader(openFileDialog.FileName);
                string fileData = sr.ReadToEnd();
                string[] lines = fileData.Split( '\n');
                for (int i = 1; i < lines.Length; i++) {
                    string line = lines[i];
                    if (line != "") {
                        string[] items = line.Split(',');
                        decimal x = Decimal.Parse(items[3].Substring(1, items[3].Length - 2), CultureInfo.InvariantCulture);
                        decimal y = Decimal.Parse(items[4].Substring(1, items[4].Length - 2), CultureInfo.InvariantCulture);
                        string s = items[0].Substring(1, items[0].Length - 2);
                        Console.WriteLine("Shot: " + s + " Score: " + items[2] + " x: " + x + " y: " + y);

                        generateAndSend(x, y);
                        Thread.Sleep(300);
                        Application.DoEvents();
                    }
                }
            }
        }

        private void chkChamp_CheckedChanged(object sender, EventArgs e) {
            if (chkChamp.Checked) {
                range = 150;
            } else {
                range = (int)Math.Round(distanceBetweenSensors * 10 / 2, 0);
            }
        }

        private void btnImportLog_Click(object sender, EventArgs e) {
            // First check if still connected
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                    "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            openFileDialog.Filter = "Cleaned log file|*.logc";
            DialogResult r = openFileDialog.ShowDialog();
            if (r == DialogResult.OK) {
                StreamReader sr = new StreamReader(openFileDialog.FileName);
                string fileData = sr.ReadToEnd();
                string[] lines = fileData.Split('\n');
                for (int i = 0; i < lines.Length; i++) {
                    string line = lines[i];
                    if (line != "") {

                        line += Environment.NewLine;
                        txtOutput.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff") + " | ");
                        txtOutput.AppendText(line);

                        try {
                            serialPort1.WriteLine(line);
                        } catch (TimeoutException ex) {
                            statusText.Text = "Error writing to port: (" + count + ") " + ex.Message;
                            Console.WriteLine("ERROR: (" + count + ") " + ex.Message);
                            timer1.Enabled = false;
                            btnTimer.Text = "Start Timer";
                        }

                        Thread.Sleep(300);
                        Application.DoEvents();
                    }
                }
            }
        }

        private void btnMiss_Click(object sender, EventArgs e) {
            // First check if still connected
            if (!isConnected || serialPort1 == null || !serialPort1.IsOpen)
            {
                MessageBox.Show("The simulator is not connected to a serial port. Please connect first.", 
                    "Not Connected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            generateMissAndSend();
        }
        
        // Event handlers for shooter settings UI
        private void trkShooterSkill_ValueChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
            
            // Update label to show current value
            double skillValue = trkShooterSkill.Value / 100.0;
            lblShooterSkill.Text = $"Shooter Skill: {skillValue:F2}";
        }
        
        private void trkBreathingStrength_ValueChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
            
            // Update label to show current value
            double breathingValue = trkBreathingStrength.Value / 100.0;
            lblBreathingStrength.Text = $"Breathing: {breathingValue:F2}";
        }
        
        private void trkMissChance_ValueChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
            
            // Update label to show current value
            double missValue = trkMissChance.Value / 1000.0;
            lblMissChance.Text = $"Miss: {missValue:P1}";
        }
        
        private void numXOffset_ValueChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
        }
        
        private void numYOffset_ValueChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
        }
        
        private void chkBreathingEnabled_CheckedChanged(object sender, EventArgs e)
        {
            settingsChanged = true;
            btnApplySettings.Enabled = true;
            
            // Enable/disable breathing strength slider based on checkbox
            trkBreathingStrength.Enabled = chkBreathingEnabled.Checked;
        }
        
        private void btnApplySettings_Click(object sender, EventArgs e)
        {
            ApplySettingsFromUI();
            settingsChanged = false;
            btnApplySettings.Enabled = false;
            statusText.Text = "Shooter settings applied";
        }
        
        private void ApplySettingsFromUI()
        {
            // Apply settings from UI to the simulation properties
            shooterSkill = trkShooterSkill.Value / 100.0;
            breathingStrength = trkBreathingStrength.Value / 100.0;
            missChance = trkMissChance.Value / 1000.0;
            centerOffsetX = numXOffset.Value;
            centerOffsetY = numYOffset.Value;
            breathingEnabled = chkBreathingEnabled.Checked;
        }
        
        private void btnResetToDefault_Click(object sender, EventArgs e)
        {
            // Reset all settings to their default values
            trkShooterSkill.Value = (int)(DEFAULT_SHOOTER_SKILL * 100);
            trkBreathingStrength.Value = (int)(DEFAULT_BREATHING_STRENGTH * 100);
            trkMissChance.Value = (int)(DEFAULT_MISS_CHANCE * 1000);
            numXOffset.Value = 0;
            numYOffset.Value = 0;
            chkBreathingEnabled.Checked = true;
            
            // Apply the reset settings immediately
            ApplySettingsFromUI();
            
            settingsChanged = false;
            btnApplySettings.Enabled = false;
            statusText.Text = "Shooter settings reset to default";
            
            // Update labels
            lblShooterSkill.Text = $"Shooter Skill: {DEFAULT_SHOOTER_SKILL:F2}";
            lblBreathingStrength.Text = $"Breathing: {DEFAULT_BREATHING_STRENGTH:F2}";
            lblMissChance.Text = $"Miss: {DEFAULT_MISS_CHANCE:P1}";
        }
    }
}
