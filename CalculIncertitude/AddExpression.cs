using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class AddExpression : BinaryOp
    {
        public AddExpression(Expression l, Expression r) : base(l, r)
        {
        }
        protected override double DoEvaluate(double a, double b)
        {
            return a + b;
        }
        public override Expression Derivate(string variable)
        {
            return left.Derivate(variable) + right.Derivate(variable);
        }
        protected override Expression DoSimplify(ConstantExpression leftc, ConstantExpression rightc)
        {
            if (leftc != null && leftc.Val == 0)
            {
                return right;
            }
            else if (rightc != null && rightc.Val == 0)
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
            return "+";
        }
    }
}
