using System;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Threading;
using SakuraWPF;


namespace SakuraWPF.UserControls
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        protected bool bInitialized = false;
        //protected TextBlock TitleText;
        public DispatcherTimer SysTimer = new DispatcherTimer();
        protected string Title;
        public SakuraWinformLib.Viewport MainViewPort = new SakuraWinformLib.Viewport();
        public double totalTime = 0;
        public double deltaTime = 0;

        // Window properties
        double normaltop;
        double normalleft;
        double normalwidth;
        double normalheight;
        public MainWindow()
        {
            InitializeComponent();
            this.Activated += MainWindow_SourceInitialized;
        }

        protected void MainWindow_SourceInitialized(object sender, System.EventArgs e)
        {
            ExitButton.Click += ExitButton_Click;
            TitleBar.MouseLeftButtonDown += TitleBar_MouseLeftButtonDown;
            MaxButton.Click += MaxButton_Click;

            if (!bInitialized)
            {
                bInitialized = true;
                SakuraCore.CreateSakuraCore((uint)CORE_GRAPHICS_API_CONF.SAKURA_DRAW_WITH_D3D12);
                MainViewPort.BorderStyle = BorderStyle.None;
                viewportHost.Child = MainViewPort;
                IntPtr Hwnd = MainViewPort.Handle;
                SakuraCore.InitSakuraGraphicsCore(Hwnd, (uint)MainViewPort.Width, (uint)MainViewPort.Height);
                SakuraCore.Run();
                
                //System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
                //timer.Interval = 1;
                //timer.Tick += MasterTick;
                //timer.Start();
            }
        }

        private void MaxButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.WindowState == WindowState.Normal)
            {
                normaltop = this.Top;
                normalleft = this.Left;
                normalwidth = this.Width;
                normalheight = this.Height;

                double top = SystemParameters.WorkArea.Top;
                double left = SystemParameters.WorkArea.Left;
                double right = SystemParameters.PrimaryScreenWidth - SystemParameters.WorkArea.Right;
                double bottom = SystemParameters.PrimaryScreenHeight - SystemParameters.WorkArea.Bottom;
                this.Margin = new Thickness(left, top, right, bottom);

                this.WindowState = WindowState.Maximized;
            }
            else
            {
                this.Top = 0;
                this.Left = 0;
                this.Width = 0;
                this.Height = 0;

                this.Top = normaltop;
                this.Left = normalleft;
                this.Width = normalwidth;
                this.Height = normalheight;

                this.WindowState = WindowState.Normal;

                this.Margin = new Thickness(0);
            }

        }

        private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }

        private void ExitButton_Click(object sender, RoutedEventArgs e)
        {
            SakuraWPF.App.Current.Shutdown();
        }

        protected void MasterTick(object sender, EventArgs e)
        {
            deltaTime = (double)DateTime.Now.Millisecond * 0.001 + (double)DateTime.Now.Second - totalTime;
            totalTime = (double)DateTime.Now.Millisecond * 0.001 + (double)DateTime.Now.Second;
            TitleText.Text = Title + "FPS: " + (uint)(1/deltaTime);
            SakuraCore.TickSakuraCore(deltaTime);
        }

    }
}
