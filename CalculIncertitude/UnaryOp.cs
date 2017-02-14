using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public abstract class UnaryOp : Expression
    {
        protected Expression Arg { get; }
        public UnaryOp(Expression arg)
        {
            Arg = arg;
        }

        protected abstract double DoEvaluate(double x);
        public sealed override double Evaluate(IDictionary<string, double> values)
        {
            return DoEvaluate(Arg.Evaluate(values));
        }

        public override Expression Simplify()
        {
            var ac = Arg as ConstantExpression;
            if (ac != null)
            {
                double newConst = DoEvaluate(ac.Val);
                return new ConstantExpression(newConst);
            }
            else
            {
                return this;
            }
        }

        protected abstract string Op();

        public sealed override string ToString()
        {
            return string.Format("{0}({1})", Op(), Arg.ToString());
        }

        public sealed override string ToStringRpn()
        {
            return string.Format("{1} {0}", Op(), Arg.ToStringRpn());
        }

        public sealed override IEnumerable<string> Variables()
        {
            return Arg.Variables();
        }
    }
}
