using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace FogBugzReader
{
	class XMLParser
	{
		public XMLParser()
		{
		}
		
		~XMLParser()
		{
		}

		public static XmlDocument Result(string xmlString)
		{
			XmlDocument xmlDoc = new XmlDocument();
			xmlDoc.Load(new StringReader(xmlString));

			return xmlDoc;
		}
	}
}
