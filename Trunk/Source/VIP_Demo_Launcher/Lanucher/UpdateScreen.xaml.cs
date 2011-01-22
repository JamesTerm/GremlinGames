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
using System.IO;
using System.Threading;
using System.Net;
using System.Collections.Specialized;

namespace Launcher
{
    /****************************************************
     * StringProperties
     *   Summary:
     *      NameValueCollection is a map <string, string>.
     *         StringProperties is a wrapper for this to allow 
     *         it to load a property map and write a property map.
     ***************************************************/
    public class StringProperties : NameValueCollection
    {
        public StringProperties(StreamReader readFrom)
        {
            while (readFrom.Peek() >= 0)
            {
                String line = readFrom.ReadLine ();
                //The : is the symbol for the property
                if (line.Contains(":"))
                {
                    String[] values = line.Split(':');
                    Add(values[0], values[1]);
                }
            }
        }
        public StringProperties()
        {
        }
        public void WriteOut (StreamWriter writeTo)
        {
            foreach (KeyValuePair <String,String> valuePair in this)
            {
                writeTo.WriteLine(valuePair.Key + ":" + valuePair.Value);
            }
        }
    }
    
    public class Updater
    {
        //Downloaded revision filename
        String dlRevisionFileName = "RemoteRevisionList.txt";
        String revisionFileName = "RevisionList.txt";
        String rootUpdate = "http://gremlingames.com/update/";
        int m_Progress;
        public delegate void IAmDone (bool successfulDownload);
        public delegate void ProgressHandler (int progress);
        public ProgressHandler Progress;
        public IAmDone CallWhenDone;
        bool successfulDownload;
        public Updater()
        {
            m_Progress = 0;
        }
        public FileInfo GetHttpFile(String url)
        {
            String tempFileName = "TemporaryDownload";
            int byteArraySize = 512;

            try
            {
                HttpWebRequest webRequest =
                    (HttpWebRequest)WebRequest.Create(url);
                HttpWebResponse webResponse = (HttpWebResponse)
                    webRequest.GetResponse();
                Stream copyFrom = webResponse.GetResponseStream();
                Stream copyTo = File.OpenWrite(tempFileName);
                byte[] readByte = new byte[byteArraySize];
                while (copyFrom.Read(readByte, 0, byteArraySize) > 0)
                {
                    copyTo.Write(readByte, 0, byteArraySize);
                }
                copyFrom.Close();
                copyTo.Close();
            }
            catch (Exception)
            {
                return null;
            }
            return new FileInfo(tempFileName);
        }

        //Note that this will not make a list of new files
        public List <String> GetUpdatesNeeded
            (NameValueCollection oldVersion, 
            NameValueCollection newVersion)
        {
            List <String> toReturn = new List<String>();
            foreach (String propName in newVersion.AllKeys)
            {
                if (newVersion.Get(propName) != oldVersion.Get(propName))
                {
                    toReturn.Add(propName); 
                }
            }
            return toReturn;
        }

        private void DownloadList(List<String> toDownload)
        {
            List<FileInfo> toReturn = new List<FileInfo>();
            foreach (String download in toDownload)
            {
                if (!File.Exists(download + ".tmp"))
                {
                    FileInfo toAdd = GetHttpFile(rootUpdate + download);
                    if (toAdd != null)
                    {
                        toAdd.MoveTo(download + ".tmp");
                        toReturn.Add(toAdd);
                    }
                }
                else
                {
                    toReturn.Add(new FileInfo(download + ".tmp"));
                }
            }
            foreach (FileInfo toMove in toReturn)
            {
                try
                {
                    String moveTo = toMove.Name.Remove(toMove.Name.Length - 4);
                    File.Delete (moveTo);
                    toMove.MoveTo(moveTo);
                }
                catch (FieldAccessException error)
                {
                    MessageBox.Show(error.Message);
                }         
            }
        }

        public void Run()
        {
            successfulDownload = true;
            //Request the file from the server
            FileInfo newRevisionFile = 
                GetHttpFile (rootUpdate + revisionFileName);
            if (newRevisionFile != null)
            {
                //Setup to read
                StreamReader current = newRevisionFile.OpenText ();
                StringProperties remoteVersions = 
                    new StringProperties (current);
                current.Close();
                File.Delete(dlRevisionFileName);
                newRevisionFile.MoveTo(dlRevisionFileName);
                StringProperties localVersions;
                try
                {
                    current = File.OpenText(revisionFileName);
                    localVersions = new StringProperties (current);
                    current.Close ();
                }
                catch (FileNotFoundException )
                {
                    File.Create(revisionFileName);
                    localVersions = new StringProperties ();
                }
                //Compare with file on local drive
                List <String> toUpdate = GetUpdatesNeeded(localVersions, remoteVersions);
                //Update all flagged things
                DownloadList(toUpdate);
                File.Delete(revisionFileName);
                newRevisionFile.MoveTo(revisionFileName);
            }
            else
            {
                successfulDownload = false;
            }
            m_Progress = 100;
            if (Progress != null)
                Progress(m_Progress);
            if (CallWhenDone != null) 
                CallWhenDone (successfulDownload);
        }
    }
    /// <summary>
    /// Interaction logic for UpdateScreen.xaml
    /// </summary>
    public partial class UpdateScreen : Window
    {
        Updater m_Updater;
        public UpdateScreen()
        {
            InitializeComponent();
        }
        private void LoadNextWindow (bool successStatus)
        {
            this.Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Normal,
                new Action(
                    delegate()
                    {
                        if (!successStatus)
                        {
                            MessageBox.Show("Update Failed!");
                        }
                        //Close this window and continue to the next one
                        this.Hide();
                        new Window1().Show();
                        this.Close();
                    }));

        }
        private void UpdateProgress(int total)
        {
            //Different thread causes issues use a dispatcher
            //http://www.switchonthecode.com/tutorials/working-with-the-wpf-dispatcher
            ProgressTotal.Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Normal,
                new Action(
                    delegate()
                    {
                        ProgressTotal.Value = total;
                    }
            ));
        }
        private void Updater_Loaded(object sender, RoutedEventArgs e)
        {
            m_Updater = new Updater();
            m_Updater.CallWhenDone += LoadNextWindow;
            m_Updater.Progress += UpdateProgress;
            new Thread (m_Updater.Run).Start();
        }
    }
}
