using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

namespace Localization_Test2
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            string culture = "fr-FR";
            //string culture = "en-US";
            System.Threading.Thread.CurrentThread.CurrentUICulture =
                new System.Globalization.CultureInfo(culture);
            System.Threading.Thread.CurrentThread.CurrentCulture =
                new System.Globalization.CultureInfo(culture);
        }
    }
}
