using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

using System.Collections;
using System.Threading;

namespace PhpBB
{
    /// <summary>
    /// Source obtained from: http://www.codeproject.com/KB/security/phpbblogin.aspx?msg=3343828
    /// Provides functions to login and logout from phpbb forum, ver 2.0
    /// Session is kept in a file.
    /// If you need to relog in just call the LoginThread.  
    /// </summary>
    [Serializable]
    public class PhpBBLogin
    {
        [field: NonSerialized]
        public event EventHandler OnLoginFinish;

        [field: NonSerialized]
        public event EventHandler OnLogoutFinish;
        /// <summary>
        /// This class is singleton
        /// </summary>
        private static PhpBBLogin _instance = null;

        private string _password;

        /// <summary>
        /// the domain where cookies will be valid
        /// </summary>
        public string _domain;

        /// <summary>
        /// The full server address, with http
        /// </summary>
        private string _server;

        public Hashtable _cookies;

        private bool _logged = false;

        /// <summary>
        /// Current logged user
        /// </summary>
        private string _user;

        /// <summary>
        /// Current version of dll, used for serialisation
        /// </summary>
        private Version _ver = new Version(1, 0);

        /// <summary>
        /// When the current session will expire
        /// </summary>
        private DateTime _dateTimeExpire=DateTime.Now;

        /// <summary>
        /// Based on admin setting from phpbb
        /// </summary>
        private int _validityDaysForKey=2;

        [NonSerialized]
        private Thread _loginThread;

        [NonSerialized]
        public string LastError=String.Empty;

        [field: NonSerialized]
        public event EventHandler OnError;

        /// <summary>
        /// Hidden constructor
        /// </summary>
        private PhpBBLogin()
        {
            _cookies = new Hashtable();
        }
        /// <summary>
        /// Gets the current key session
        /// </summary>
        public string SID
        {
            get
            {
                if (_cookies != null && _cookies.ContainsKey("phpbb2mysql_sid"))
                    return _cookies["phpbb2mysql_sid"].ToString();

                return "-1";
            }
        }

        /// <summary>
        /// How many days is valid the key
        /// </summary>
        public int ValidityDaysForKey
        {
            set { _validityDaysForKey = value; }

            get { return _validityDaysForKey; }
        }

        public static PhpBBLogin Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new PhpBBLogin();
                }
                return _instance;
            }
        }
        /// <summary>
        /// Indicates if the user is logged
        /// </summary>
        public bool IsLogged
        {
            get
            {                               
                return _logged;
            }
        }

        /// <summary>
        /// Checks the server for login
        /// </summary>
        public bool IsReallyLogged
        {
            get
            {
                if (IsLogged)
                {
                    string html=RetrieveDataFromForum(_server+"index.php?sid="+SID, "", "GET");

                    if (html != null)
                    {
                        if (html.IndexOf("login.php?logout=true") > 0)
                        {
                            return true;
                        }
                    }
                }
                return false;
            }
        }

        /// <summary>
        /// Returns the logged user
        /// </summary>
        public string User
        {
            get
            {
                return _user;
            }
        }

        /// <summary>
        /// Gets or sets the current domain
        /// </summary>
        public string Domain
        {
            set
            {
                _domain = value;
                _server = "http://" + _domain + "/";
            }
            get
            {
                return _domain;
            }
        }
        /// <summary>
        /// Gets the server address
        /// </summary>
        public string Server
        {
            get
            {
                return _server;
            }
        }
        /// <summary>
        /// Logout from phpbb forum, erases the session file
        /// </summary>
        public void Logout()
        {
            if (_cookies.ContainsKey("phpbb2mysql_sid"))
            {
                string r = RetrieveDataFromForum(_server + "login.php?logout=true&sid=" + SID, "", "GET");
            }

            _logged = false;

            if (File.Exists("cache\\inf.cookies"))
            {
                try
                {
                    File.Delete("cache\\inf.cookies");
                }
                catch (Exception)
                {

                }
            }

            if (OnLogoutFinish != null)
            {
                OnLogoutFinish.Invoke(this, EventArgs.Empty);
            }  
        }

        /// <summary>
        /// Retrieve html string from address using specified method
        /// </summary>
        /// <param name="address">address</param>
        /// <param name="param">post or get param</param>
        /// <param name="method">post or get</param>
        /// <returns>html string</returns>
        public string RetrieveDataFromForum(string address, string param, string method)
        {
            StringBuilder builder = new StringBuilder(param);
            WebResponse response=null;

            try
            {
                byte[] data = Encoding.ASCII.GetBytes(builder.ToString());
                Uri addrUri = new Uri(address);
                HttpWebRequest request = (HttpWebRequest)WebRequest.Create(addrUri);
                request.Timeout = 5000;
                request.Method = method;

                if (method == "POST")
                {
                    request.ContentType = "application/x-www-form-urlencoded";
                    request.ContentLength = data.Length;
                }

                //Set existing cookies, if any
                request.CookieContainer = new CookieContainer();

                lock (_cookies)
                {
                    if (_cookies != null)
                    {
                        foreach (string name in _cookies.Keys)
                        {
                            if (name.Trim() != "expires" && name.Trim() != "path")
                            {
                                Cookie c = new Cookie(name.Trim(), _cookies[name].ToString().Trim());
                                c.Path = "/";
                                c.HttpOnly = true;
                                c.Domain = _domain;
                                request.CookieContainer.Add(c);
                            }
                        }
                    }

                    ///////////////////////////////////////////////////////////////////////////////
                    if (method == "POST")
                    {
                        //send data for post
                        Stream stream = request.GetRequestStream();
                        stream.Write(data, 0, data.Length);
                        stream.Close();
                    }

                    response = request.GetResponse();

                    //update last cookies sent by server
                    string[] keys = response.Headers.GetValues("Set-Cookie");

                    GetCookies(keys);
                    ////////////////////////////////////////////////////////////////////
                }

                StreamReader reader = new StreamReader(response.GetResponseStream());

                string html = reader.ReadToEnd();

                reader.Close();
                response.GetResponseStream().Close();
                response.Close();

                return html;
            }

            catch (Exception e)
            {
                if (this.OnError != null)
                {
                    OnError.Invoke(this, EventArgs.Empty);
                }
                return null;
            }
        }

        /// <summary>
        /// Login to PHPBB
        /// </summary>
        /// <param name="user">User nick</param>
        /// <param name="password">User password</param>
        /// <returns>server message</returns>
        public void Login(string user, string password)
        {
            _user = user;

            _password = password;

            _loginThread = new Thread(new ThreadStart(LoginThead));

            _loginThread.Start();            
        }

        public void GetCookies(string[] keys)
        {
            for (int i = 0; i < keys.Length; i++)
            {
                keys[i] = keys[i] + ";";
            }
            string allCookies = String.Concat(keys);

            string[] items = allCookies.Split(new char[] { (';') });

            foreach (string item in items)
            {
                string[] c = item.Split(new char[] { ('=') });

                if (c.Length == 2)
                {
                    if (_cookies.ContainsKey(c[0]))
                    {
                        _cookies.Remove(c[0]);
                    }

                    _cookies.Add(c[0], c[1]);
                }
            }
        }

        /// <summary>
        /// Login to PHPBB
        /// </summary>
        /// <param name="user">User nick</param>
        /// <param name="password">User password</param>
        /// <returns>server message</returns>
        public void LoginThead()
        {
            try
            {                
                _cookies.Clear();
                StringBuilder builder = new StringBuilder();
                builder.Append("autologin=1&login=true&username=" + _user + "&password=" + _password + "&redirect=\n");

                byte[] data = Encoding.ASCII.GetBytes(builder.ToString());

                HttpWebRequest request = (HttpWebRequest)WebRequest.Create(new Uri(_server + "ucp.php?mode=login&"));
                request.Timeout = 5000;
                request.Method = "POST";
                request.ContentType = "application/x-www-form-urlencoded";
                request.ContentLength = data.Length;

                Stream stream = request.GetRequestStream();
                stream.Write(data, 0, data.Length);
                stream.Close();

                WebResponse response = request.GetResponse();

                string[] keys = response.Headers.GetValues("Set-Cookie");

                GetCookies(keys);

                _logged = false;

                foreach (string name in _cookies.Keys)
                {
                    if (name == "phpbb3_63hdw_u")
                    {
                        if (_cookies[name].ToString() != "1")
                        {
                            _logged = true;

                            _dateTimeExpire = DateTime.Now;

                            _dateTimeExpire= _dateTimeExpire.AddDays(_validityDaysForKey);
                        }
                    }
                }
               
                response.GetResponseStream().Close();

                response.Close();

                SaveCache();               

                if (OnLoginFinish != null)
                {                    
                    OnLoginFinish.Invoke(this, EventArgs.Empty);
                }                                
            }
            catch (Exception e)
            {
                System.Windows.MessageBox.Show(e.Message);
                _logged = false;

                if (OnLoginFinish != null)
                {
                    OnLoginFinish.Invoke(this, EventArgs.Empty);
                }  
            }
        }

        /// <summary>
        /// cache information about user
        /// </summary>
        public bool SaveCache()
        {
            try
            {
                if (!Directory.Exists("cache"))
                {
                    Directory.CreateDirectory("cache");
                }
                if (File.Exists("cache\\inf.cookies"))
                {
                    File.Delete("cache\\inf.cookeis");
                }
                byte[] data = CSerialize.Serialize(Instance);
                FileStream fs = new FileStream("cache\\inf.cookies", FileMode.CreateNew);
                BinaryWriter w = new BinaryWriter(fs);
                w.Write(data);
                w.Close();
                fs.Close();
            }
            catch (Exception e)
            {
                return false;
            }
            return true;
        }

        /// <summary>
        /// loading cache
        /// </summary>
        public void LoadCache()
        {
            if (File.Exists("cache\\inf.cookies"))
            {
                try
                {
                    FileStream fs = new FileStream("cache\\inf.cookies", FileMode.Open, FileAccess.Read);
                    BinaryReader r = new BinaryReader(fs);
                    byte[] data = r.ReadBytes((int)fs.Length);
                    PhpBBLogin temp = new PhpBBLogin();
                    temp = (PhpBBLogin)CSerialize.DeSerialize(data);

                    if (_ver == temp._ver)
                    {
                        _cookies = temp._cookies;
                        _user = temp._user;
                        _password = temp._password;
                        _logged = temp._logged;
                        _dateTimeExpire = temp._dateTimeExpire;
                        _validityDaysForKey = temp._validityDaysForKey;

                        if (DateTime.Now > _dateTimeExpire)
                        {
                            //login has expired
                            _logged = false;
                        }
                    }
                    else
                    {
                        _logged = false;

                        File.Delete("cache\\inf.cookies");
                    }
                }
                catch (Exception)
                {

                }
            }
        }

        public string PostMessage(string p)
        {
            if (this.IsLogged)
            {
                return RetrieveDataFromForum(_server+"posting.php?sid="+SID, p, "POST");
            }
            return null;
        }
    }

    public class CSerialize
    {

        /// <summary>
        /// Serializa object to memory
        /// </summary>
        /// <param name="obj">object to serialize</param>
        /// <returns>bytes[]:binnary object serialization</returns>
        public static byte[] Serialize(object obj)
        {
            MemoryStream stream = new MemoryStream();
            if (obj == null)
            {
                return null;
            }
            try
            {
                if (stream != null)
                {
                    IFormatter formatter = new BinaryFormatter();
                    formatter.Serialize(stream, obj);
                    stream.Seek(0, 0);
                    return stream.ToArray();
                }
                else
                {
                    return null;
                }
            }
            catch (Exception)
            {              
                return null;
            }
            finally
            {
                if (stream != null) stream.Close();
            }
        }


        /// <summary>
        /// Deserialize classessa sd ad asfasfas fsdfsdf
        /// </summary>
        /// <param name="buffer">buffer represent serialized object</param>
        /// <returns>object- deserialized object</returns>
        public static object DeSerialize(byte[] buffer)
        {
            MemoryStream stream = new MemoryStream(buffer);

            stream.Capacity = buffer.Length;
            try
            {
                IFormatter formatter = new BinaryFormatter();
                object tmp;
                tmp = formatter.Deserialize(stream);
                stream.Close();
                return tmp;
            }
            catch (Exception)
            {                
                return null;
            }
            finally
            {
                if (stream != null) stream.Close();
            }
        }
    }
}
