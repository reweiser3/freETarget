namespace TargetSimulator
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);
            this.btnConnect = new System.Windows.Forms.Button();
            this.txtOutput = new System.Windows.Forms.TextBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.btnShot = new System.Windows.Forms.Button();
            this.btnTimer = new System.Windows.Forms.Button();
            this.status = new System.Windows.Forms.StatusStrip();
            this.statusText = new System.Windows.Forms.ToolStripStatusLabel();
            this.btnCenter = new System.Windows.Forms.Button();
            this.btnLeft = new System.Windows.Forms.Button();
            this.btnTop = new System.Windows.Forms.Button();
            this.btnTopRight = new System.Windows.Forms.Button();
            this.btnBottom = new System.Windows.Forms.Button();
            this.btnRight = new System.Windows.Forms.Button();
            this.btnHalfway = new System.Windows.Forms.Button();
            this.txtX = new System.Windows.Forms.TextBox();
            this.txtY = new System.Windows.Forms.TextBox();
            this.btnShoot = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btnImport = new System.Windows.Forms.Button();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.chkChamp = new System.Windows.Forms.CheckBox();
            this.btnImportLog = new System.Windows.Forms.Button();
            this.btnMiss = new System.Windows.Forms.Button();
            this.grpShooterSettings = new System.Windows.Forms.GroupBox();
            this.btnResetToDefault = new System.Windows.Forms.Button();
            this.btnApplySettings = new System.Windows.Forms.Button();
            this.chkBreathingEnabled = new System.Windows.Forms.CheckBox();
            this.numXOffset = new System.Windows.Forms.NumericUpDown();
            this.numYOffset = new System.Windows.Forms.NumericUpDown();
            this.lblXOffset = new System.Windows.Forms.Label();
            this.lblYOffset = new System.Windows.Forms.Label();
            this.trkShooterSkill = new System.Windows.Forms.TrackBar();
            this.trkBreathingStrength = new System.Windows.Forms.TrackBar();
            this.trkMissChance = new System.Windows.Forms.TrackBar();
            this.lblBreathingStrength = new System.Windows.Forms.Label();
            this.lblShooterSkill = new System.Windows.Forms.Label();
            this.lblMissChance = new System.Windows.Forms.Label();
            this.btnRefreshPorts = new System.Windows.Forms.Button();
            this.status.SuspendLayout();
            this.grpShooterSettings.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trkMissChance)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trkBreathingStrength)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trkShooterSkill)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numYOffset)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numXOffset)).BeginInit();
            this.SuspendLayout();
            // 
            // comboBox1
            // 
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(12, 12);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(90, 21);
            this.comboBox1.TabIndex = 0;
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(139, 10);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(75, 23);
            this.btnConnect.TabIndex = 1;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // txtOutput
            // 
            this.txtOutput.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtOutput.Location = new System.Drawing.Point(12, 207);
            this.txtOutput.Multiline = true;
            this.txtOutput.Name = "txtOutput";
            this.txtOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtOutput.Size = new System.Drawing.Size(763, 424);
            this.txtOutput.TabIndex = 2;
            this.txtOutput.WordWrap = false;
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // btnShot
            // 
            this.btnShot.Enabled = false;
            this.btnShot.Location = new System.Drawing.Point(416, 10);
            this.btnShot.Name = "btnShot";
            this.btnShot.Size = new System.Drawing.Size(75, 23);
            this.btnShot.TabIndex = 3;
            this.btnShot.Text = "Take a Shot";
            this.toolTip.SetToolTip(this.btnShot, "Takes a random shot");
            this.btnShot.UseVisualStyleBackColor = true;
            this.btnShot.Click += new System.EventHandler(this.btnShot_Click);
            // 
            // btnTimer
            // 
            this.btnTimer.Enabled = false;
            this.btnTimer.Location = new System.Drawing.Point(502, 10);
            this.btnTimer.Name = "btnTimer";
            this.btnTimer.Size = new System.Drawing.Size(75, 23);
            this.btnTimer.TabIndex = 4;
            this.btnTimer.Text = "Start Timer";
            this.toolTip.SetToolTip(this.btnTimer, "Generate random shots in a quick succesion");
            this.btnTimer.UseVisualStyleBackColor = true;
            this.btnTimer.Click += new System.EventHandler(this.btnTimer_Click);
            // 
            // status
            // 
            this.status.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusText});
            this.status.Location = new System.Drawing.Point(0, 631);
            this.status.Name = "status";
            this.status.Size = new System.Drawing.Size(787, 22);
            this.status.TabIndex = 5;
            this.status.Text = "Ready";
            // 
            // statusText
            // 
            this.statusText.Name = "statusText";
            this.statusText.Size = new System.Drawing.Size(39, 17);
            this.statusText.Text = "Ready";
            // 
            // btnCenter
            // 
            this.btnCenter.Enabled = false;
            this.btnCenter.Location = new System.Drawing.Point(13, 40);
            this.btnCenter.Name = "btnCenter";
            this.btnCenter.Size = new System.Drawing.Size(75, 23);
            this.btnCenter.TabIndex = 6;
            this.btnCenter.Text = "Center";
            this.btnCenter.UseVisualStyleBackColor = true;
            this.btnCenter.Click += new System.EventHandler(this.btnCenter_Click);
            // 
            // btnLeft
            // 
            this.btnLeft.Enabled = false;
            this.btnLeft.Location = new System.Drawing.Point(95, 40);
            this.btnLeft.Name = "btnLeft";
            this.btnLeft.Size = new System.Drawing.Size(75, 23);
            this.btnLeft.TabIndex = 7;
            this.btnLeft.Text = "Left";
            this.btnLeft.UseVisualStyleBackColor = true;
            this.btnLeft.Click += new System.EventHandler(this.btnLeft_Click);
            // 
            // btnTop
            // 
            this.btnTop.Enabled = false;
            this.btnTop.Location = new System.Drawing.Point(177, 40);
            this.btnTop.Name = "btnTop";
            this.btnTop.Size = new System.Drawing.Size(75, 23);
            this.btnTop.TabIndex = 8;
            this.btnTop.Text = "Top";
            this.btnTop.UseVisualStyleBackColor = true;
            this.btnTop.Click += new System.EventHandler(this.btnTop_Click);
            // 
            // btnTopRight
            // 
            this.btnTopRight.Enabled = false;
            this.btnTopRight.Location = new System.Drawing.Point(420, 39);
            this.btnTopRight.Name = "btnTopRight";
            this.btnTopRight.Size = new System.Drawing.Size(75, 23);
            this.btnTopRight.TabIndex = 9;
            this.btnTopRight.Text = "TopRight";
            this.btnTopRight.UseVisualStyleBackColor = true;
            this.btnTopRight.Click += new System.EventHandler(this.btnTopRight_Click);
            // 
            // btnBottom
            // 
            this.btnBottom.Enabled = false;
            this.btnBottom.Location = new System.Drawing.Point(258, 39);
            this.btnBottom.Name = "btnBottom";
            this.btnBottom.Size = new System.Drawing.Size(75, 23);
            this.btnBottom.TabIndex = 10;
            this.btnBottom.Text = "Bottom";
            this.btnBottom.UseVisualStyleBackColor = true;
            this.btnBottom.Click += new System.EventHandler(this.btnBottom_Click);
            // 
            // btnRight
            // 
            this.btnRight.Enabled = false;
            this.btnRight.Location = new System.Drawing.Point(339, 39);
            this.btnRight.Name = "btnRight";
            this.btnRight.Size = new System.Drawing.Size(75, 23);
            this.btnRight.TabIndex = 11;
            this.btnRight.Text = "Right";
            this.btnRight.UseVisualStyleBackColor = true;
            this.btnRight.Click += new System.EventHandler(this.btnRight_Click);
            // 
            // btnHalfway
            // 
            this.btnHalfway.Enabled = false;
            this.btnHalfway.Location = new System.Drawing.Point(501, 39);
            this.btnHalfway.Name = "btnHalfway";
            this.btnHalfway.Size = new System.Drawing.Size(75, 23);
            this.btnHalfway.TabIndex = 12;
            this.btnHalfway.Text = "Halfway";
            this.btnHalfway.UseVisualStyleBackColor = true;
            this.btnHalfway.Click += new System.EventHandler(this.btnHalfway_Click);
            // 
            // txtX
            // 
            this.txtX.Location = new System.Drawing.Point(37, 69);
            this.txtX.Name = "txtX";
            this.txtX.Size = new System.Drawing.Size(100, 20);
            this.txtX.TabIndex = 13;
            // 
            // txtY
            // 
            this.txtY.Location = new System.Drawing.Point(166, 69);
            this.txtY.Name = "txtY";
            this.txtY.Size = new System.Drawing.Size(100, 20);
            this.txtY.TabIndex = 14;
            // 
            // btnShoot
            // 
            this.btnShoot.Enabled = false;
            this.btnShoot.Location = new System.Drawing.Point(272, 67);
            this.btnShoot.Name = "btnShoot";
            this.btnShoot.Size = new System.Drawing.Size(75, 23);
            this.btnShoot.TabIndex = 15;
            this.btnShoot.Text = "Shoot";
            this.toolTip.SetToolTip(this.btnShoot, "Takes a shot at the X,Y coordinates");
            this.btnShoot.UseVisualStyleBackColor = true;
            this.btnShoot.Click += new System.EventHandler(this.bthShoot_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 73);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(17, 13);
            this.label1.TabIndex = 16;
            this.label1.Text = "X:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(143, 73);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(17, 13);
            this.label2.TabIndex = 17;
            this.label2.Text = "Y:";
            // 
            // btnImport
            // 
            this.btnImport.Enabled = false;
            this.btnImport.Location = new System.Drawing.Point(501, 69);
            this.btnImport.Name = "btnImport";
            this.btnImport.Size = new System.Drawing.Size(75, 23);
            this.btnImport.TabIndex = 18;
            this.btnImport.Text = "Import TS...";
            this.toolTip.SetToolTip(this.btnImport, "Imports a TargetScan App CVS exported file");
            this.btnImport.UseVisualStyleBackColor = true;
            this.btnImport.Click += new System.EventHandler(this.btnImport_Click);
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "TargetScan App file|*.csv";
            // 
            // chkChamp
            // 
            this.chkChamp.AutoSize = true;
            this.chkChamp.Location = new System.Drawing.Point(251, 14);
            this.chkChamp.Name = "chkChamp";
            this.chkChamp.Size = new System.Drawing.Size(86, 17);
            this.chkChamp.TabIndex = 19;
            this.chkChamp.Text = "Good scores";
            this.chkChamp.UseVisualStyleBackColor = true;
            this.chkChamp.CheckedChanged += new System.EventHandler(this.chkChamp_CheckedChanged);
            // 
            // btnImportLog
            // 
            this.btnImportLog.Enabled = false;
            this.btnImportLog.Location = new System.Drawing.Point(420, 66);
            this.btnImportLog.Name = "btnImportLog";
            this.btnImportLog.Size = new System.Drawing.Size(75, 26);
            this.btnImportLog.TabIndex = 20;
            this.btnImportLog.Text = "Import log...";
            this.btnImportLog.UseVisualStyleBackColor = true;
            this.btnImportLog.Click += new System.EventHandler(this.btnImportLog_Click);
            // 
            // btnMiss
            // 
            this.btnMiss.Enabled = false;
            this.btnMiss.Location = new System.Drawing.Point(353, 67);
            this.btnMiss.Name = "btnMiss";
            this.btnMiss.Size = new System.Drawing.Size(61, 23);
            this.btnMiss.TabIndex = 21;
            this.btnMiss.Text = "Miss";
            this.btnMiss.UseVisualStyleBackColor = true;
            this.btnMiss.Click += new System.EventHandler(this.btnMiss_Click);
            // 
            // grpShooterSettings
            // 
            this.grpShooterSettings.Controls.Add(this.btnResetToDefault);
            this.grpShooterSettings.Controls.Add(this.btnApplySettings);
            this.grpShooterSettings.Controls.Add(this.chkBreathingEnabled);
            this.grpShooterSettings.Controls.Add(this.numXOffset);
            this.grpShooterSettings.Controls.Add(this.numYOffset);
            this.grpShooterSettings.Controls.Add(this.lblXOffset);
            this.grpShooterSettings.Controls.Add(this.lblYOffset);
            this.grpShooterSettings.Controls.Add(this.trkShooterSkill);
            this.grpShooterSettings.Controls.Add(this.trkBreathingStrength);
            this.grpShooterSettings.Controls.Add(this.trkMissChance);
            this.grpShooterSettings.Controls.Add(this.lblBreathingStrength);
            this.grpShooterSettings.Controls.Add(this.lblShooterSkill);
            this.grpShooterSettings.Controls.Add(this.lblMissChance);
            this.grpShooterSettings.Location = new System.Drawing.Point(12, 95);
            this.grpShooterSettings.Name = "grpShooterSettings";
            this.grpShooterSettings.Size = new System.Drawing.Size(763, 106);
            this.grpShooterSettings.TabIndex = 22;
            this.grpShooterSettings.TabStop = false;
            this.grpShooterSettings.Text = "Shooter Settings";
            // 
            // btnResetToDefault
            // 
            this.btnResetToDefault.Location = new System.Drawing.Point(501, 56);
            this.btnResetToDefault.Name = "btnResetToDefault";
            this.btnResetToDefault.Size = new System.Drawing.Size(123, 30);
            this.btnResetToDefault.TabIndex = 12;
            this.btnResetToDefault.Text = "Reset to Default";
            this.toolTip.SetToolTip(this.btnResetToDefault, "Reset all shooting parameters to default values");
            this.btnResetToDefault.UseVisualStyleBackColor = true;
            // 
            // btnApplySettings
            // 
            this.btnApplySettings.Location = new System.Drawing.Point(501, 19);
            this.btnApplySettings.Name = "btnApplySettings";
            this.btnApplySettings.Size = new System.Drawing.Size(123, 31);
            this.btnApplySettings.TabIndex = 11;
            this.btnApplySettings.Text = "Apply Settings";
            this.toolTip.SetToolTip(this.btnApplySettings, "Apply shooter settings immediately");
            this.btnApplySettings.UseVisualStyleBackColor = true;
            // 
            // chkBreathingEnabled
            // 
            this.chkBreathingEnabled.AutoSize = true;
            this.chkBreathingEnabled.Checked = true;
            this.chkBreathingEnabled.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkBreathingEnabled.Location = new System.Drawing.Point(356, 69);
            this.chkBreathingEnabled.Name = "chkBreathingEnabled";
            this.chkBreathingEnabled.Size = new System.Drawing.Size(114, 17);
            this.chkBreathingEnabled.TabIndex = 10;
            this.chkBreathingEnabled.Text = "Breathing Enabled";
            this.toolTip.SetToolTip(this.chkBreathingEnabled, "Enable breathing effect on shots");
            this.chkBreathingEnabled.UseVisualStyleBackColor = true;
            // 
            // numXOffset
            // 
            this.numXOffset.DecimalPlaces = 1;
            this.numXOffset.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numXOffset.Location = new System.Drawing.Point(407, 20);
            this.numXOffset.Maximum = new decimal(new int[] {
            5,
            0,
            0,
            0});
            this.numXOffset.Minimum = new decimal(new int[] {
            5,
            0,
            0,
            -2147483648});
            this.numXOffset.Name = "numXOffset";
            this.numXOffset.Size = new System.Drawing.Size(76, 20);
            this.numXOffset.TabIndex = 9;
            this.toolTip.SetToolTip(this.numXOffset, "Shooter\'s natural horizontal aim offset");
            // 
            // numYOffset
            // 
            this.numYOffset.DecimalPlaces = 1;
            this.numYOffset.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numYOffset.Location = new System.Drawing.Point(407, 46);
            this.numYOffset.Maximum = new decimal(new int[] {
            5,
            0,
            0,
            0});
            this.numYOffset.Minimum = new decimal(new int[] {
            5,
            0,
            0,
            -2147483648});
            this.numYOffset.Name = "numYOffset";
            this.numYOffset.Size = new System.Drawing.Size(76, 20);
            this.numYOffset.TabIndex = 8;
            this.toolTip.SetToolTip(this.numYOffset, "Shooter\'s natural vertical aim offset");
            // 
            // lblXOffset
            // 
            this.lblXOffset.AutoSize = true;
            this.lblXOffset.Location = new System.Drawing.Point(353, 22);
            this.lblXOffset.Name = "lblXOffset";
            this.lblXOffset.Size = new System.Drawing.Size(48, 13);
            this.lblXOffset.TabIndex = 7;
            this.lblXOffset.Text = "X Offset:";
            // 
            // lblYOffset
            // 
            this.lblYOffset.AutoSize = true;
            this.lblYOffset.Location = new System.Drawing.Point(353, 48);
            this.lblYOffset.Name = "lblYOffset";
            this.lblYOffset.Size = new System.Drawing.Size(48, 13);
            this.lblYOffset.TabIndex = 6;
            this.lblYOffset.Text = "Y Offset:";
            // 
            // trkShooterSkill
            // 
            this.trkShooterSkill.Location = new System.Drawing.Point(116, 41);
            this.trkShooterSkill.Maximum = 100;
            this.trkShooterSkill.Minimum = 10;
            this.trkShooterSkill.Name = "trkShooterSkill";
            this.trkShooterSkill.Size = new System.Drawing.Size(174, 45);
            this.trkShooterSkill.TabIndex = 5;
            this.toolTip.SetToolTip(this.trkShooterSkill, "Higher skill = tighter grouping");
            this.trkShooterSkill.Value = 80;
            // 
            // trkBreathingStrength
            // 
            this.trkBreathingStrength.Location = new System.Drawing.Point(116, 62);
            this.trkBreathingStrength.Maximum = 100;
            this.trkBreathingStrength.Name = "trkBreathingStrength";
            this.trkBreathingStrength.Size = new System.Drawing.Size(174, 45);
            this.trkBreathingStrength.TabIndex = 4;
            this.toolTip.SetToolTip(this.trkBreathingStrength, "How much breathing affects shot placement");
            this.trkBreathingStrength.Value = 30;
            // 
            // trkMissChance
            // 
            this.trkMissChance.Location = new System.Drawing.Point(116, 19);
            this.trkMissChance.Maximum = 20;
            this.trkMissChance.Name = "trkMissChance";
            this.trkMissChance.Size = new System.Drawing.Size(174, 45);
            this.trkMissChance.TabIndex = 3;
            this.toolTip.SetToolTip(this.trkMissChance, "Probability of a complete miss (0-2%)");
            this.trkMissChance.Value = 1;
            // 
            // lblBreathingStrength
            // 
            this.lblBreathingStrength.AutoSize = true;
            this.lblBreathingStrength.Location = new System.Drawing.Point(10, 69);
            this.lblBreathingStrength.Name = "lblBreathingStrength";
            this.lblBreathingStrength.Size = new System.Drawing.Size(100, 13);
            this.lblBreathingStrength.TabIndex = 2;
            this.lblBreathingStrength.Text = "Breathing Strength:";
            // 
            // lblShooterSkill
            // 
            this.lblShooterSkill.AutoSize = true;
            this.lblShooterSkill.Location = new System.Drawing.Point(10, 48);
            this.lblShooterSkill.Name = "lblShooterSkill";
            this.lblShooterSkill.Size = new System.Drawing.Size(69, 13);
            this.lblShooterSkill.TabIndex = 1;
            this.lblShooterSkill.Text = "Shooter Skill:";
            // 
            // lblMissChance
            // 
            this.lblMissChance.AutoSize = true;
            this.lblMissChance.Location = new System.Drawing.Point(10, 26);
            this.lblMissChance.Name = "lblMissChance";
            this.lblMissChance.Size = new System.Drawing.Size(72, 13);
            this.lblMissChance.TabIndex = 0;
            this.lblMissChance.Text = "Miss Chance:";
            // 
            // btnRefreshPorts
            // 
            this.btnRefreshPorts.Location = new System.Drawing.Point(108, 10);
            this.btnRefreshPorts.Name = "btnRefreshPorts";
            this.btnRefreshPorts.Size = new System.Drawing.Size(25, 23);
            this.btnRefreshPorts.TabIndex = 23;
            this.btnRefreshPorts.Text = "↻";
            this.toolTip.SetToolTip(this.btnRefreshPorts, "Refresh COM Ports List");
            this.btnRefreshPorts.UseVisualStyleBackColor = true;
            this.btnRefreshPorts.Click += new System.EventHandler(this.btnRefreshPorts_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(787, 653);
            this.Controls.Add(this.btnRefreshPorts);
            this.Controls.Add(this.grpShooterSettings);
            this.Controls.Add(this.btnMiss);
            this.Controls.Add(this.btnImportLog);
            this.Controls.Add(this.chkChamp);
            this.Controls.Add(this.btnImport);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnShoot);
            this.Controls.Add(this.txtY);
            this.Controls.Add(this.txtX);
            this.Controls.Add(this.btnHalfway);
            this.Controls.Add(this.btnRight);
            this.Controls.Add(this.btnBottom);
            this.Controls.Add(this.btnTopRight);
            this.Controls.Add(this.btnTop);
            this.Controls.Add(this.btnLeft);
            this.Controls.Add(this.btnCenter);
            this.Controls.Add(this.status);
            this.Controls.Add(this.btnTimer);
            this.Controls.Add(this.btnShot);
            this.Controls.Add(this.txtOutput);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.comboBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "Target Simulator";
            this.status.ResumeLayout(false);
            this.status.PerformLayout();
            this.grpShooterSettings.ResumeLayout(false);
            this.grpShooterSettings.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trkMissChance)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trkBreathingStrength)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trkShooterSkill)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numYOffset)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numXOffset)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBox1;
        private System.IO.Ports.SerialPort serialPort1;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.TextBox txtOutput;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Button btnShot;
        private System.Windows.Forms.Button btnTimer;
        private System.Windows.Forms.StatusStrip status;
        private System.Windows.Forms.ToolStripStatusLabel statusText;
        private System.Windows.Forms.Button btnCenter;
        private System.Windows.Forms.Button btnLeft;
        private System.Windows.Forms.Button btnTop;
        private System.Windows.Forms.Button btnTopRight;
        private System.Windows.Forms.Button btnBottom;
        private System.Windows.Forms.Button btnRight;
        private System.Windows.Forms.Button btnHalfway;
        private System.Windows.Forms.TextBox txtX;
        private System.Windows.Forms.TextBox txtY;
        private System.Windows.Forms.Button btnShoot;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnImport;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.CheckBox chkChamp;
        private System.Windows.Forms.Button btnImportLog;
        private System.Windows.Forms.Button btnMiss;
        private System.Windows.Forms.GroupBox grpShooterSettings;
        private System.Windows.Forms.Button btnResetToDefault;
        private System.Windows.Forms.Button btnApplySettings;
        private System.Windows.Forms.CheckBox chkBreathingEnabled;
        private System.Windows.Forms.NumericUpDown numXOffset;
        private System.Windows.Forms.NumericUpDown numYOffset;
        private System.Windows.Forms.Label lblXOffset;
        private System.Windows.Forms.Label lblYOffset;
        private System.Windows.Forms.TrackBar trkShooterSkill;
        private System.Windows.Forms.TrackBar trkBreathingStrength;
        private System.Windows.Forms.TrackBar trkMissChance;
        private System.Windows.Forms.Label lblBreathingStrength;
        private System.Windows.Forms.Label lblShooterSkill;
        private System.Windows.Forms.Label lblMissChance;
        private System.Windows.Forms.Button btnRefreshPorts;
    }
}

