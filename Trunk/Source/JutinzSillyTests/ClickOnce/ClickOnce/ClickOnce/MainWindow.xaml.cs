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
using System.Windows.Shapes;

namespace ClickOnce
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			this.InitializeComponent();

			// Insert code required on object creation below this point.
		}

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("You clicked!");
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Create the interop host control.
            System.Windows.Forms.Integration.WindowsFormsHost host =
                new System.Windows.Forms.Integration.WindowsFormsHost();

            // Create the ActiveX control.
            AxRimSpaceSingleAxLib.AxRimSpaceSingleAx axOSG = new AxRimSpaceSingleAxLib.AxRimSpaceSingleAx();

            // Assign the ActiveX control as the host control's child.
            host.Child = axOSG;

            // Add the interop host control to the Grid
            // control's collection of child controls.
            this.Grid_OSG_Parent.Children.Add(host);
        }
	}
}