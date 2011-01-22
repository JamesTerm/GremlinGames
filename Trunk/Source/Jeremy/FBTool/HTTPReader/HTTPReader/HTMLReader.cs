using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;

namespace FogBugzReader
{
    public class HTMLReader
    {
        public static string Result(string site)
        {
			string responseString = "";

			try
			{
				// Create a URL request and get response.	
				WebRequest request = WebRequest.Create(site);
				HttpWebResponse response = (HttpWebResponse)request.GetResponse();

				// Create stream, open using StreamReader, and get the content.
				Stream dataStream = response.GetResponseStream();
				StreamReader reader = new StreamReader(dataStream);
				responseString = reader.ReadToEnd();
				
				// Cleanup the streams and the response.
				reader.Close();
				dataStream.Close();
				response.Close();
			}
			catch (Exception)
			{
				Console.WriteLine("There was a problem getting data from web server.");
			}
			return responseString;
        }
    }
}
