using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public abstract class BinaryOp : Expression
    {
        protected Expression left { get; }
        protected Expression right { get; }
        public BinaryOp(Expression l, Expression r)
        {
            left = l.Simplify();
            right = r.Simplify();
        }

        protected abstract double DoEvaluate(double a, double b);
        public sealed override double Evaluate(IDictionary<string, double> values)
        {
            return DoEvaluate(left.Evaluate(values), right.Evaluate(values));
        }

        protected abstract Expression DoSimplify(ConstantExpression leftc, ConstantExpression rightc);
        public sealed override Expression Simplify()
        {
            var lc = left as ConstantExpression;
            var rc = right as ConstantExpression;
            if (lc != null && rc != null)
            {
                double newConst = DoEvaluate(lc.Val, rc.Val);
                return new ConstantExpression(newConst);
            }
            else
            {
                return DoSimplify(lc, rc);
            }
        }

        protected abstract string Op();

        public sealed override string ToString()
        {
            return string.Format("({0}{1}{2})", left.ToString(), Op(), right.ToString());
        }

        public sealed override string ToStringRpn()
        {
            return string.Format("{0} {2} {1}", left.ToStringRpn(), Op(), right.ToStringRpn());
        }

        public sealed override IEnumerable<string> Variables()
        {
            return left.Variables().Concat(right.Variables());
        }
    }
}
