using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class ExpExpression : BinaryOp
    {
        public ExpExpression(Expression l, Expression r) : base(l, r)
        {
        }
        protected override double DoEvaluate(double a, double b)
        {
            return Math.Pow(a, b);
        }
        public override Expression Derivate(string v)
        {
            // https://en.wikipedia.org/wiki/Differentiation_rules#Generalized_power_rule
            //var ld = left.Derivate(v);
            //var sumLeft = new MulExpression(ld, new DivExpression(right, left));
            //var rd = right.Derivate(v);
            //var sumRight = new MulExpression(rd, new LnExpression(left));
            //var sum = new AddExpression(sumLeft, sumRight);
            //var res = new MulExpression(this, sum);
            //return res;
            return this * (left.Derivate(v) * right / left + right.Derivate(v) * new LnExpression(left));
        }
        protected override Expression DoSimplify(ConstantExpression leftc, ConstantExpression rightc)
        {
            if (leftc != null)
            {
                if (leftc.Val == 0)
                {
                    return ConstantExpression.Zero;
                }
                else if (leftc.Val == 1)
                {
                    return ConstantExpression.One;
                }
            }
            if (rightc != null)
            {
                if (rightc.Val == 0)
                {
                    return ConstantExpression.One;
                }
                else if (rightc.Val == 1)
                {
                    return left;
                }
            }
            return this;
        }

        protected override string Op()
        {
            return "^";
        }
    }
}
