using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class LnExpression : UnaryOp
    {
        public LnExpression(Expression arg) : base(arg)
        {
        }
        protected override double DoEvaluate(double x)
        {
            return Math.Log(x);
        }
        public override Expression Derivate(string variable)
        {
            Expression deriv = Arg.Derivate(variable);
            return new DivExpression(deriv, Arg);
        }

        protected override string Op()
        {
            return "ln";
        }
    }
}
