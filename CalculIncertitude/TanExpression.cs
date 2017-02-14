using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class TanExpression : UnaryOp
    {
        public TanExpression(Expression arg) : base(arg)
        {
        }
        protected override double DoEvaluate(double x)
        {
            return Math.Tan(x);
        }
        public override Expression Derivate(string variable)
        {
            Expression deriv = Arg.Derivate(variable);
            Expression one = ConstantExpression.One;
            Expression tanSq = new MulExpression(this, this);
            Expression onePlusTanSq = new AddExpression(one, tanSq);
            return new MulExpression(deriv, onePlusTanSq);
        }

        protected override string Op()
        {
            return "tan";
        }
    }
}
