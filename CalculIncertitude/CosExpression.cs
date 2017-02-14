using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class CosExpression : UnaryOp
    {
        public CosExpression(Expression arg) : base(arg)
        {
        }
        protected override double DoEvaluate(double x)
        {
            return Math.Cos(x);
        }
        public override Expression Derivate(string variable)
        {
            Expression newLeft = new SinExpression(Arg);
            Expression newRight = Arg.Derivate(variable);
            return new NegExpression(new MulExpression(newLeft, newRight));
        }

        protected override string Op()
        {
            return "cos";
        }
    }
}
