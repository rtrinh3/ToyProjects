using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    abstract public class Expression
    {
        public abstract double Evaluate(IDictionary<string, double> values);
        public abstract Expression Derivate(string variable);
        public virtual Expression Simplify()
        {
            return this;
        }

        public abstract IEnumerable<string> Variables();
        public abstract override string ToString();
        public abstract string ToStringRpn();

        // Convenience operators

        public static AddExpression operator +(Expression left, Expression right)
        {
            return new AddExpression(left, right);
        }

        public static SubExpression operator -(Expression left, Expression right)
        {
            return new SubExpression(left, right);
        }

        public static MulExpression operator *(Expression left, Expression right)
        {
            return new MulExpression(left, right);
        }

        public static DivExpression operator /(Expression left, Expression right)
        {
            return new DivExpression(left, right);
        }

        public static NegExpression operator -(Expression arg)
        {
            return new NegExpression(arg);
        }
    }
}
