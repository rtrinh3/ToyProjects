// https://www.reddit.com/r/dailyprogrammer/comments/3xye4g/20151223_challenge_246_intermediate_letter_splits/
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Reddit246_Intermediate
{
    class LetterFinder
    {
        private static string[] ListOfEmptyString = { "" };
        private static string[] EmptyList = { };
        private Lazy<IEnumerable<string>>[] sets;
        private string haystack;
        public LetterFinder(string h)
        {
            haystack = h;
            sets = new Lazy<IEnumerable<string>>[h.Length + 1];
            sets[h.Length] = new Lazy<IEnumerable<string>>(() => ListOfEmptyString);
            for (int i = 0; i < h.Length; ++i)
            {
                int index = i; // Need a copy of i, because closures...
                sets[index] = new Lazy<IEnumerable<string>>(() =>
                {
                    char c = haystack[index];
                    if ('1' == c)
                    {
                        int j = index + 1;
                        var PartOne = sets[j].Value.Select(tail => "A" + tail);
                        if (j < haystack.Length)
                        {
                            string nextHead = ((char)(haystack[j] - '0' + 'J')).ToString();
                            var PartTwo = sets[j + 1].Value.Select(tail => nextHead + tail);
                            return PartOne.Concat(PartTwo);
                        }
                        else
                        {
                            return PartOne;
                        }
                    }
                    else if ('2' == c)
                    {
                        int j = index + 1;
                        var PartOne = sets[j].Value.Select(tail => "B" + tail);
                        if (j < haystack.Length && '0' <= haystack[j] && haystack[j] <= '6')
                        {
                            string nextHead = ((char)(haystack[j] - '0' + 20 - 1 + 'A')).ToString();
                            var PartTwo = sets[j + 1].Value.Select(tail => nextHead + tail);
                            return PartOne.Concat(PartTwo);
                        }
                        else
                        {
                            return PartOne;
                        }
                    }
                    else if ('3' <= c && c <= '9')
                    {
                        string head = ((char)(c - '3' + 'C')).ToString();
                        return sets[index + 1].Value.Select(tail => head + tail);
                    }
                    else
                    {
                        return EmptyList;
                    }
                });
            }
        }

        public IEnumerable<string> GetLetters()
        {
            return sets[0].Value;
        }
    }

    static class Program
    {
        static string[] ListOfEmptyString = { "" };
        static string[] EmptyList = { };

        static IEnumerable<string> Combos(string s)
        {
            var finder = new LetterFinder(s);
            return finder.GetLetters();
        }

        static private PrefixTreeMatcher Matcher = null;

        static bool ValidWords(string s)
        {
            /*if (string.IsNullOrWhiteSpace(s))
            {
                return true;
            }
            foreach (var len in Matcher.Find(s))
            {
                if (ValidWords(s.Substring(len)))
                {
                    return true;
                }
            }
            return false;*/
            return string.IsNullOrWhiteSpace(s) || Matcher.Find(s).Any(len => ValidWords(s.Substring(len)));
        }

        static void Main(string[] args)
        {
            // More initialisation
            const string test = "81161625815129412519419122516181571811313518";

            // Initialize matcher
            var lines = System.IO.File.ReadLines("enable1.txt");
            Matcher = PrefixTreeMatcher.ConstructFromList(lines.Select(s => s.ToUpper()));

            var timer = System.Diagnostics.Stopwatch.StartNew();
            var letters = Combos(test);
            var words = letters.Where(ValidWords); // letters for normal, letters.Where(ValidWords) for bonus
            foreach (var s in words)
            {
                Console.WriteLine(s);
            }
            timer.Stop();
            Console.WriteLine("Time: {0}", timer.Elapsed);
        }
    }
}
