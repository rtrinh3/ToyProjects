using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class MulExpression : BinaryOp
    {
        public MulExpression(Expression l, Expression r) : base(l, r)
        {
        }
        protected override double DoEvaluate(double a, double b)
        {
            return a * b;
        }
        public override Expression Derivate(string variable)
        {
            //Expression newLeft = new MulExpression(left.Derivate(variable), right);
            //Expression newRight = new MulExpression(left, right.Derivate(variable));
            //return new AddExpression(newLeft, newRight);
            return (left.Derivate(variable) * right) + (left * right.Derivate(variable));
        }
        protected override Expression DoSimplify(ConstantExpression leftc, ConstantExpression rightc)
        {
            if ((leftc != null && leftc.Val == 0) || (rightc != null && rightc.Val == 0))
            {
                return ConstantExpression.Zero;
            }
            else if (leftc != null && leftc.Val == 1)
            {
                return right;
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
            return "*";
        }
    }
}
