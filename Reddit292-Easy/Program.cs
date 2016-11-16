using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// https://www.reddit.com/r/dailyprogrammer/comments/5d1l7v/20161115_challenge_292_easy_increasing_range/
namespace Reddit292_Easy
{
    class Program
    {
        static string[] rangeSeperators = { "-", ":", ".." };

        static void Main(string[] args)
        {
            string input = Console.ReadLine();
            var ranges = input.Split(',');
            int max = 0;
            IEnumerable<int> res = Enumerable.Empty<int>();
            foreach (var range in ranges)
            {
                var parts = range.Split(rangeSeperators, StringSplitOptions.RemoveEmptyEntries);
                switch (parts.Length)
                {
                case 1:
                    max = parseShorthand(max, parts[0]);
                    res = res.Concat(Enumerable.Repeat(max, 1));
                    break;
                case 2:
                    int left2 = max = parseShorthand(max, parts[0]);
                    int right2 = max = parseShorthand(max, parts[1]);
                    res = res.Concat(Enumerable.Range(left2, right2 - left2 + 1));
                    break;
                case 3:
                    int left3 = max = parseShorthand(max, parts[0]);
                    int right3 = max = parseShorthand(max, parts[1]);
                    int step = int.Parse(parts[2]);
                    res = res.Concat(RangeStep(left3, right3 + 1, step));
                    break;
                default:
                    throw new Exception("Malformed string");
                }
            }
            Console.WriteLine(string.Join(" ", res));
            Console.ReadLine();
        }

        static int parseShorthand(int previous, string shorthand)
        {
            string previousString = previous.ToString();
            if (shorthand.Length > previousString.Length)
            {
                return int.Parse(shorthand);
            }
            string next = previousString.Substring(0, previousString.Length - shorthand.Length) + shorthand;
            int nextVal = int.Parse(next);
            if (nextVal <= previous)
            {
                nextVal += TenPow(shorthand.Length);
            }
            return nextVal;
        }

        static int TenPow(int n)
        {
            int x = 1;
            for (int i = 0; i < n; ++i)
            {
                x *= 10;
            }
            return x;
        }

        static IEnumerable<int> RangeStep(int begin, int end, int step)
        {
            for (int i = begin; i < end; i += step)
            {
                yield return i;
            }
        }
    }
}
