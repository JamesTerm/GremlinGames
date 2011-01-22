using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace ConsoleTools
{
    class CreateArgs
    {
        public void FillArguments(string input_line,ref List<string> args )
        {
            bool in_argument = false;
           	const char group_char = '"';
            int PreviousPosition=-1;
            int Index=0;
            foreach (char c in input_line)
            {
                //Console.Write("{0}", c);
                if (c==group_char)
                    in_argument = !in_argument;
                else if (!in_argument && (c==' ' || c=='\t'))
                {
                    string SubString = input_line.Substring(PreviousPosition + 1, Index - PreviousPosition);
                    args.Add(SubString.Trim(group_char));
                    PreviousPosition = Index;
                }
                Index++;
            }
            if (Index > 0)
            {
                string SubString = input_line.Substring(PreviousPosition + 1);
                args.Add(SubString.Trim(group_char));
            }

            Debug.Assert(in_argument == false); // Uneven quotes?
        }
    }
}

namespace TestCode
{
    class Program
    {
        static void Main(string[] args)
        {
            string line=Console.ReadLine();
            List<string> CLI_args=new List<string>();
            ConsoleTools.CreateArgs tools=new ConsoleTools.CreateArgs();
            tools.FillArguments(line, ref CLI_args);
            //Console.WriteLine("Test {0}",line);
            foreach (string s in CLI_args)
            {
                Console.WriteLine("Test {0}",s);
            }
        }
    }
}
