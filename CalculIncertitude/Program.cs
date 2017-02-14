using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.RegularExpressions;

namespace CalculIncertitude
{
    public static class Parser
    {
        private static Expression Parse(IEnumerable<string> rpn)
        {
            Stack<Expression> stack = new Stack<Expression>();
            Dictionary<string, Func<Expression, Expression, Expression>> BinaryOps = new Dictionary<string, Func<Expression, Expression, Expression>>();
            BinaryOps["+"] = (a, b) => new AddExpression(a, b);
            BinaryOps["-"] = (a, b) => new SubExpression(a, b);
            BinaryOps["*"] = (a, b) => new MulExpression(a, b);
            BinaryOps["/"] = (a, b) => new DivExpression(a, b);
            BinaryOps["^"] = (a, b) => new ExpExpression(a, b);
            Dictionary<string, Func<Expression, Expression>> UnaryOps = new Dictionary<string, Func<Expression, Expression>>();
            UnaryOps["#"] = x => new NegExpression(x);
            UnaryOps["sin"] = x => new SinExpression(x);
            UnaryOps["cos"] = x => new CosExpression(x);
            UnaryOps["tan"] = x => new TanExpression(x);
            UnaryOps["ln"] = x => new LnExpression(x);
            foreach (string token in rpn)
            {
                if (BinaryOps.ContainsKey(token))
                {
                    var right = stack.Pop();
                    var left = stack.Pop();
                    stack.Push(BinaryOps[token](left, right));
                }
                else if (UnaryOps.ContainsKey(token))
                {
                    var arg = stack.Pop();
                    stack.Push(UnaryOps[token](arg));
                }
                else if (char.IsLetter(token[0]))
                {
                    stack.Push(new VariableExpression(token));
                }
                else
                {
                    stack.Push(new ConstantExpression(Double.Parse(token)));
                }
            }
            return stack.Single();
        }

        private static IEnumerable<string> Tokenize(string s)
        {
            Regex re = new Regex(@"[a-zA-Z]+|(\d*(\.\d*)?)");
            int index = 0;
            string last = "~";
            while (index < s.Length)
            {
                var match = re.Match(s, index);
                if (match.Success && match.Index == index && match.Length > 0)
                {
                    yield return last = s.Substring(index, match.Length);
                    index += match.Length;
                }
                else
                {
                    string next = s.Substring(index, 1);
                    if (next == "-" && !(Char.IsLetter(last[0]) || Char.IsDigit(last[0]) || last == ")"))
                    {
                        yield return last = "#";
                    }
                    else
                    {
                        yield return last = next;
                    }

                    index += 1;
                }
            }
        }

        public static Expression Parse(string formula)
        {
            return Parse(ShuntingYard.Algorithm(Tokenize(formula)));
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Console.Write("Formula?");
            var formula = Console.ReadLine();
            var expr = Parser.Parse(formula);
            var vars = expr.Variables().Distinct().ToList();
            var values = new Dictionary<string, double>();
            var errors = new Dictionary<string, double>();
            foreach (string v in vars)
            {
                Console.Write(" " + v + "?");
                values[v] = double.Parse(Console.ReadLine());
                Console.Write("Δ" + v + "?");
                errors[v] = double.Parse(Console.ReadLine());
            }
            double expected = expr.Evaluate(values);
            var error = vars.Sum(v => Math.Abs(expr.Derivate(v).Evaluate(values) * errors[v]));

            double errorMagnitude = Math.Floor(Math.Log10(error));
            double scale = Math.Pow(10, errorMagnitude);
            var scaledError = Math.Round(error / scale);
            var scaledValue = Math.Round(expected / scale);
            Console.WriteLine("{0}e{1} ± {2}e{3}", scaledValue, errorMagnitude, scaledError, errorMagnitude);
        }
    }
}
