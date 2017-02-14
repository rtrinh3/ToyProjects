using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class NegExpression : UnaryOp
    {
        public NegExpression(Expression arg) : base(arg)
        {
        }
        protected override double DoEvaluate(double x)
        {
            return -x;
        }
        public override Expression Derivate(string variable)
        {
            return new NegExpression(Arg.Derivate(variable));
        }

        protected override string Op()
        {
            return "Neg";
        }
    }
}
