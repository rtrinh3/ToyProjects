//https://www.reddit.com/r/dailyprogrammer/comments/3bzipa/20150703_challenge_221_hard_poetry_in_a_haystack/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Reddit221_Hard
{
    class Program
    {
        static void Main(string[] args)
        {
            var dict = new HashSet<string>(File.ReadLines("enable1.txt"));
            Func<string, double> Confidence = s =>
            {
                var words = s.Split();
                return words.Where(w => dict.Contains(w)).Count() / (double)words.Length;
            };
            var query = File.ReadLines("challenge.txt")
                .Select((line, index) => new { Index = index, Line = line, Confidence = Confidence(line) })
                .OrderByDescending(x => x.Confidence).ThenByDescending(x => x.Line)
                .Take(20);
            foreach (var thing in query)
            {
                Console.WriteLine(thing);
            }
        }
    }
}
