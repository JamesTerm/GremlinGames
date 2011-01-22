using System;
using System.Collections.Generic;
using System.IO;
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
using System.Xml;

namespace Launcher
{
    
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        [System.Runtime.InteropServices.DllImport("user32.dll")]
        public static extern bool SetForegroundWindow(IntPtr hWnd);
        private String xmlFileName = "LauncherSettings.xml";
        public Window1()
        {  
            InitializeComponent();
        }


        private List<FileInfo> GetDirectoryList(String directoryName, String extention)
        {
            List<FileInfo> fileNames = new List<FileInfo>();
            DirectoryInfo directory = new DirectoryInfo(directoryName);
            foreach (FileInfo gameFile in directory.GetFiles("*" + extention))
            {
                fileNames.Add(gameFile);
            }
            return fileNames;
        }

        private List <GameFileInfo> GetDirectoryList(String directory, 
            String gameName, String contentPath, String extention)
        {
            List<GameFileInfo> fileNames = new List<GameFileInfo>();
            DirectoryInfo scenarioDirectory = new DirectoryInfo(directory);
            foreach (FileInfo gameFile in scenarioDirectory.GetFiles("*" + extention))
            {
                fileNames.Add (new GameFileInfo (gameFile, gameName, contentPath));
            }
            return fileNames;
        }

        private void ListBox_Loaded(object sender, RoutedEventArgs e)
        {
            XmlTextReader xmlGameList = new XmlTextReader(xmlFileName);
            String thePath = "";
            String gameName = "";
            String contentPath = "";
           
            MusicList.SelectedIndex = 
                MusicList.Items.Add("No Music");
            //Skip root node
            xmlGameList.Read();
            while (xmlGameList.Read())
            {
                switch (xmlGameList.NodeType)
                {
                    case XmlNodeType.Element:
                        while (xmlGameList.MoveToNextAttribute())
                        {
                            switch (xmlGameList.Name)
                            {
                                case "path":
                                    thePath = xmlGameList.Value;
                                    break;
                                case "contentPath" :
                                    contentPath = xmlGameList.Value;
                                    break;
                            }
                            
                        }
                        break;
                    case XmlNodeType.Text:
                        gameName = xmlGameList.Value;
                        List<GameFileInfo> gameList = GetDirectoryList(thePath, gameName,
                            contentPath, ".lua");
                        foreach (GameFileInfo toAdd in gameList)
                        {
                            ScenarioList.Items.Add(toAdd);
                        }
                        List<FileInfo> musicDirectoryList = GetDirectoryList(contentPath + "\\Soundtrack\\", ".mp3");
                        foreach (FileInfo toAdd in musicDirectoryList)
                        {
                            MusicList.Items.Add(toAdd);
                        }
                        break;
                }
            }
            ScenarioList.SelectedIndex = 0;
        }

        private void Launch_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                FileInfo programPath = new FileInfo("Fringe.Single.exe");
                GameFileInfo selected = (GameFileInfo)ScenarioList.SelectedItem;
                FileInfo selectedMusic = null;
                try
                {
                    selectedMusic = (FileInfo)MusicList.SelectedItem;
                }
                catch (Exception)
                {
                }
                Environment.CurrentDirectory = selected.contentPath;

                if (ScenarioList.SelectedItem != null)
                {
                    String music = "";
                    if (selectedMusic != null)
                    {
                        music = " -music \"" + selectedMusic.FullName + "\"";
                    }
                    System.Diagnostics.Process runGame = new
                        System.Diagnostics.Process();
                    runGame.StartInfo.FileName = programPath.FullName;
                    runGame.StartInfo.Arguments = "\"" +
                        selected.FullName + "\"" + music;
                    runGame.Start();
                    SetForegroundWindow (runGame.MainWindowHandle);
                    
                    //System.Diagnostics.Process.Start(programPath.FullName,  "\"" +
                    //    selected.FullName + "\" -music \"" + selectedMusic.FullName + "\"");
                    Close();
                }
            }
            catch (Exception /*error*/)
            {
                //Tell them it failed.
            }

        }

        //Update the description and the Image for the selected item.
        private void ScenarioList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            GameFileInfo selected = (GameFileInfo)ScenarioList.SelectedItem;
            String baseName = selected.Name.ToLower();
            baseName = baseName.Remove(baseName.Length - 4);
            try
            {
                ImagePreview.Source = new BitmapImage(
                    new Uri(selected.DirectoryName + "\\" + baseName + ".jpg"));
            }
            catch (Exception /*ErrorEventArgs*/)
            {
                ImagePreview.Source = null;
            }
            try
            {
                StreamReader descriptionReader = 
                    new StreamReader (selected.DirectoryName + "\\" + baseName + ".txt");

               DescriptiveText.Text = descriptionReader.ReadToEnd();
            }
            catch (Exception /*error*/)
            {
                DescriptiveText.Text = "No description";
            }
        }

       
    }

    /*
     * GameFileInfo is used to override the ToString function
     *    so that when it is displayed in a table you can have
     *    several different directorys containing maps.
    */
    public class GameFileInfo
    {
       
        private FileInfo mainInfo;
        private String gameName;
        private String theContentPath;
        public String contentPath
        {
            get { return theContentPath; }
        }
        public String FullName
        {
            get { return mainInfo.FullName; }
        }
        public String Name
        {
            get { return mainInfo.Name; }
        }
        public String DirectoryName
        {
            get { return mainInfo.DirectoryName; } 
        }
        public GameFileInfo(FileInfo setTo, String theGameName, String setContentPath)
        {
            gameName = theGameName;
            mainInfo = setTo;
            theContentPath = setContentPath;
        }
        public override String ToString()
        {
            return gameName + " - " + mainInfo.Name;
        }
        
    }
    
    
}
