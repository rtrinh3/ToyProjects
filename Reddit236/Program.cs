using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// https://www.reddit.com/r/dailyprogrammer/comments/3opin7/20151014_challenge_236_intermediate_fibonacciish/
namespace Reddit236
{
    class Program
    {
        // Infinite
        static IEnumerable<long> Fibs()
        {
            yield return 0;
            yield return 1;
            long a = 0;
            long b = 1;
            while (true)
            {
                yield return a + b;
                b += a;
                a = b - a;
            }
        }

        static long FindFibFactor(long n)
        {
            return Fibs()
                .Skip(1)
                .TakeWhile(x => x <= n)
                .Where(x => n % x == 0)
                .Last(); // The max should be the last
        }

        static void Main(string[] args)
        {
            long n = long.Parse(Console.ReadLine());
            if (n == 0)
            {
                Console.WriteLine("0");
            }
            else
            {
                var highestFactor = FindFibFactor(n);
                var scaleFactor = n / highestFactor;
                var ans = Fibs()
                    .Select(x => x * scaleFactor)
                    .TakeWhile(x => x <= n);
                Console.WriteLine(String.Join(" ", ans));
            }
        }
    }
}
