using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

using FogBugzReader;

namespace FogBugzReader
{
    class Program
    {
		struct User 
		{
			string userName = "";
			string userEmail = "";
			string userPhone = "";
		}

		static string[] columnName = new string[] { "sTitle","sLatestTextSummary","sPersonAssignedTo","sStatus" };
		static bool[] columnUse = new bool[] { true,true,true,true };

		static string token = "";
		static string cols=	"";
		static string userName = "";

		static void CreateCols()
		{	
			for (int i = 0; i < columnName.Count(); i ++)
			{
				if (columnUse[i])
				{
					cols += ("," + columnName[i]);
				}
			}
		}

		static string GetToken()
		{
			string result = HTMLReader.Result(String.Format("{0}cmd=logon&email={1}&password={2}",
				UserInfo.FB_baseURL, UserInfo.FB_userid, UserInfo.FB_passwd));
			XmlDocument xmlresult = XMLParser.Result(result);
			token = xmlresult.InnerText;

			return token;
		}

		static string GetUserInfo(uint userNum)
		{
			User theUser = new User;

			string result = HTMLReader.Result(String.Format("{0}cmd=viewPerson&ixPerson={1}&token={2}",
				UserInfo.FB_baseURL, userNum, token));
			XmlDocument xmlresult = XMLParser.Result(result);

			XmlNodeList xmlNodeList = xmlresult.SelectNodes("/response/person");
			foreach (XmlNode xn in xmlNodeList)
			{
				userName = xn["sFullName"].InnerText;
				string email = xn["sEmail"].InnerText;
				string phone = xn["sPhone"].InnerText;
			}
			return userName;
		}

		static XmlDocument GetCasesInFilter()
		{
			//string result = HTMLReader.Result(String.Format("{0}cmd=search&q=&cols={1}&token={2}",
			string rawXML = HTMLReader.Result(String.Format("{0}cmd=search&q=34742&cols={1}&token={2}",
				UserInfo.FB_baseURL, cols, token));
			XmlDocument xmlDoc = XMLParser.Result(rawXML);

			string test1 = xmlDoc.InnerXml;
			string test2 = xmlDoc.FirstChild.InnerText;
			int testx = xmlDoc.ChildNodes.Count;

			ProcessNodes(xmlDoc.ChildNodes);


//			XmlNodeList nodes = xmlResult.SelectNodes("response/cases/case/sTitle");

// 			XmlNode xmlNode = xmlResult.DocumentElement;
// 			XmlNodeList xmlNodeList = xmlNode.SelectNodes("/response/cases/case");

// 			XmlNodeList nodes = xmlResult.DocumentElement.SelectNodes("response/cases/case");
// 			foreach (XmlNode xmlNode in nodes)
// 			{
// 				Console.WriteLine(xmlNode.InnerXml);
// 			}

			return xmlDoc;
		}

		private static void ProcessNodes(XmlNodeList nodes)
		{
			foreach (XmlNode node in nodes)
			{
				if (node.GetType().Name == "XmlElement")
				{
					string key = node.Name;
					string value = node.Value;

					Console.WriteLine(key + " - " + value);

					if (node.HasChildNodes)
					{
						ProcessNodes(node.ChildNodes);
					}
				}
				else
				{
					string innerKey = node.Name;
					string innerValue = node.Value;
					Console.WriteLine(innerKey + " -- " + innerValue);
				}

				
// 				Console.Write(string.Format("{0} - {1} - {2}\n", node.GetType().Name, node.Name, node.Value));
// 
// 				if (node.HasChildNodes)
// 				{
// 					Console.Write("CHILD->");
// 					ProcessNodes(node.ChildNodes);
// 				}
			}
		}

        static void Main(string[] args)
        {
			CreateCols();
			Console.WriteLine(GetToken());
			Console.WriteLine(GetCasesInFilter());
			Console.WriteLine(GetUserInfo(84));
			Console.ReadLine();
        }
    }
}
