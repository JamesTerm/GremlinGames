using System;
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
using System.Net;
using System.IO;
namespace Launcher
{

    /// <summary>
    /// Interaction logic for Login.xaml
    /// </summary>
    public partial class Login : Window
    {
        private String userName;
        private String password;
        public PhpBB.PhpBBLogin bbLogin;
        String domain = "www.gremlingames.com/forum";
        public Login()
        {
            InitializeComponent();
        }

        private void LoadUpdateScreen()
        {
            this.Dispatcher.Invoke(
             System.Windows.Threading.DispatcherPriority.Normal,
             new Action(
             delegate()
             {
                 //Close this window and continue to the next one
                 this.Hide();
                 new UpdateScreen().Show();
                 this.Close();
             }));
        }

        private void LoginFinished(object sender, EventArgs e)
        {
            if (bbLogin.IsLogged)
            {
                MessageBox.Show("Login Successful");
                LoadUpdateScreen();
            }
            else
            {
                MessageBox.Show ("Login Failed!");
            }
        }

        private void SendLoginInfo()
        {
            bbLogin = PhpBB.PhpBBLogin.Instance;
            bbLogin.Domain = domain;
            bbLogin.ValidityDaysForKey = 1;
            bbLogin.LoadCache();
            bbLogin.OnLoginFinish += new EventHandler(LoginFinished);

            bbLogin.Login(userName, password);
        }

        private void Login_Click(object sender, RoutedEventArgs e)
        {
            userName = Username.Text;
            password = PasswordBox.Password;
            SendLoginInfo();
        }
    }

}
