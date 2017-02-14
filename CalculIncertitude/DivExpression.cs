using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class DivExpression : BinaryOp
    {
        public DivExpression(Expression l, Expression r) : base(l, r)
        {
        }
        protected override double DoEvaluate(double a, double b)
        {
            return a / b;
        }
        public override Expression Derivate(string variable)
        {
            //Expression newLeft = new MulExpression(left.Derivate(variable), right);
            //Expression newRight = new MulExpression(left, right.Derivate(variable));
            //Expression numerator = new SubExpression(newLeft, newRight);
            //Expression denominator = new MulExpression(right, right);
            //return new DivExpression(numerator, denominator);
            return (left.Derivate(variable) * right - left * right.Derivate(variable)) / (right * right);
        }
        protected override Expression DoSimplify(ConstantExpression leftc, ConstantExpression rightc)
        {
            if ((leftc != null && leftc.Val == 0))
            {
                return ConstantExpression.Zero;
            }
            else if (rightc != null && rightc.Val == 1)
            {
                return left;
            }
            else
            {
                return this;
            }
        }

        protected override string Op()
        {
            return "/";
        }
    }
}
