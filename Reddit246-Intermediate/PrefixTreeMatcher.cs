using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Reddit246_Intermediate
{
    class PrefixTreeMatcher
    {
        public bool IsTerminal = false;
        public PrefixTreeMatcher[] Children = new PrefixTreeMatcher[26];

        public IEnumerable<int> Find(IEnumerable<char> s)
        {
            PrefixTreeMatcher node = this;
            int i = 0;
            foreach(char c in s)
            {
                PrefixTreeMatcher next = node.Children[c - 'A'];
                if (null == next)
                {
                    break;
                }
                else
                {
                    if (next.IsTerminal)
                    {
                        yield return (i + 1);
                    }
                    node = next;
                }
                ++i;
            }
        }

        public void Insert(string s)
        {
            PrefixTreeMatcher node = this;
            int i = 0;
            int n = s.Length;
            while (i < n)
            {
                var next = node.Children[s[i] - 'A'];
                if (next != null)
                {
                    node = next;
                    ++i;
                }
                else
                {
                    break;
                }
            }
            // Append new nodes
            while (i < n)
            {
                node = node.Children[s[i] - 'A'] = new PrefixTreeMatcher();
                ++i;
            }
            node.IsTerminal = true;
        }

        public static PrefixTreeMatcher ConstructFromList(IEnumerable<string> strings)
        {
            PrefixTreeMatcher root = new PrefixTreeMatcher();
            foreach (string s in strings)
            {
                root.Insert(s);
            }
            return root;
        }
    }
}
