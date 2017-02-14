using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class SinExpression : UnaryOp
    {
        public SinExpression(Expression arg) : base(arg)
        {
        }
        protected override double DoEvaluate(double x)
        {
            return Math.Sin(x);
        }
        public override Expression Derivate(string variable)
        {
            Expression newLeft = new CosExpression(Arg);
            Expression newRight = Arg.Derivate(variable);
            return new MulExpression(newLeft, newRight);
        }

        protected override string Op()
        {
            return "sin";
        }
    }
}
