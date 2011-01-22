using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace osgvWPF
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();
        }

        private void WindowLoaded(object sender, RoutedEventArgs e)
        {
            // Create the interop host control.
            System.Windows.Forms.Integration.WindowsFormsHost host =
                new System.Windows.Forms.Integration.WindowsFormsHost();

            // Create the ActiveX control.
            AxosgvAXMFCLib.AxosgvAXMFC axOSG = new AxosgvAXMFCLib.AxosgvAXMFC();

            // Assign the ActiveX control as the host control's child.
            host.Child = axOSG;

            // Add the interop host control to the Grid
            // control's collection of child controls.
            this.grid1.Children.Add(host);

            // Listen to the event
            axOSG.Frame1000 += new AxosgvAXMFCLib._DosgvAXMFCEvents_Frame1000EventHandler(axOSG_Frame1000);

            // Tell OSG that we are ready
            axOSG.SceneName = "POSER_Federation_Ramora\\Ramora Setup.lws";
            axOSG.BackgroundColor = System.Drawing.Color.Black;
            axOSG.InitOSG("Fringe_Content");
        }

        void axOSG_Frame1000(object sender, AxosgvAXMFCLib._DosgvAXMFCEvents_Frame1000Event e)
        {
            MessageBox.Show(e.frameNum.ToString(), "Frame1000");
        }
    }
}
