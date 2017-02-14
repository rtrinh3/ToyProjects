using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class ConstantExpression : Expression
    {
        public double Val
        {
            get;
        }
        public ConstantExpression(double v)
        {
            Val = v;
        }
        public override double Evaluate(IDictionary<string, double> values)
        {
            return Val;
        }
        public override Expression Derivate(string variable)
        {
            return Zero;
        }
        public override string ToString()
        {
            return Val.ToString();
        }

        public static readonly ConstantExpression Zero = new ConstantExpression(0);
        public static readonly ConstantExpression One = new ConstantExpression(1);
        public static readonly ConstantExpression E = new ConstantExpression(Math.E);
        public static readonly ConstantExpression Pi = new ConstantExpression(Math.PI);

        public override IEnumerable<string> Variables()
        {
            return Enumerable.Empty<string>();
        }

        public override string ToStringRpn()
        {
            return Val.ToString();
        }
    }
}
