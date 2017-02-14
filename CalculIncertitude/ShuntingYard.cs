using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    static class ShuntingYard
    {
        private struct PrecedenceAssociativity
        {
            public int Precedence { get; }
            public enum Assoc { LEFT, RIGHT };
            public Assoc Associativity { get; }
            public PrecedenceAssociativity(int p, Assoc a)
            {
                Precedence = p;
                Associativity = a;
            }
        }

        // http://stackoverflow.com/a/20248984
        // Nous allons representer l'inversion (moins unaire) avec "#"

        private static Dictionary<string, PrecedenceAssociativity> precedence;
        static ShuntingYard()
        {
            precedence = new Dictionary<string, PrecedenceAssociativity>();
            precedence["#"] = new PrecedenceAssociativity(6, PrecedenceAssociativity.Assoc.RIGHT);
            precedence[")"] = new PrecedenceAssociativity(5, PrecedenceAssociativity.Assoc.LEFT);
            precedence["sin"] = precedence["cos"] = precedence["tan"] = precedence["ln"] = new PrecedenceAssociativity(4, PrecedenceAssociativity.Assoc.RIGHT);
            precedence["^"] = new PrecedenceAssociativity(3, PrecedenceAssociativity.Assoc.RIGHT);
            precedence["*"] = precedence["/"] = new PrecedenceAssociativity(2, PrecedenceAssociativity.Assoc.LEFT);
            precedence["+"] = precedence["-"] = new PrecedenceAssociativity(1, PrecedenceAssociativity.Assoc.LEFT);
            precedence["("] = new PrecedenceAssociativity(0, PrecedenceAssociativity.Assoc.LEFT);
        }

        public static IEnumerable<string> Algorithm(IEnumerable<string> infix)
        {
            Stack<string> opStack = new Stack<string>();
            foreach (string token in infix)
            {
                if ("(" == token)
                {
                    opStack.Push(token);
                }
                else if (")" == token)
                {
                    while (opStack.Count > 0 && opStack.Peek() != "(")
                    {
                        yield return opStack.Pop();
                    }
                    if (opStack.Count <= 0)
                    {
                        throw new Exception("Mismatched parentheses");
                    }
                    if (opStack.Peek() == "(")
                    {
                        opStack.Pop(); // Discard
                    }
                    // then
                    if (opStack.Count > 0 && Char.IsLetter(opStack.Peek()[0]))
                    {
                        yield return opStack.Pop();
                    }
                }
                else if (precedence.ContainsKey(token))
                {
                    int myPrecedence = precedence[token].Precedence;
                    if (precedence[token].Associativity == PrecedenceAssociativity.Assoc.LEFT)
                    {
                        while (opStack.Count > 0 && myPrecedence <= precedence[opStack.Peek()].Precedence)
                        {
                            yield return opStack.Pop();
                        }
                    }
                    else if (precedence[token].Associativity == PrecedenceAssociativity.Assoc.RIGHT)
                    {
                        while (opStack.Count > 0 && myPrecedence < precedence[opStack.Peek()].Precedence)
                        {
                            yield return opStack.Pop();
                        }
                    }
                    opStack.Push(token);
                }
                else
                {
                    yield return token;
                }
            }
            while (opStack.Count > 0)
            {
                if (opStack.Peek() == "(" || opStack.Peek() == ")")
                {
                    throw new Exception("Mismatched parentheses");
                }
                else
                {
                    yield return opStack.Pop();
                }
            }
        }
    }
}
