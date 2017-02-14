using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CalculIncertitude
{
    public class VariableExpression : Expression
    {
        private string Name
        {
            get;
        }
        public VariableExpression(string name)
        {
            Name = name;
        }
        public override double Evaluate(IDictionary<string, double> values)
        {
            return values[Name];
        }
        public override Expression Derivate(string variable)
        {
            return (variable == Name) ? ConstantExpression.One : ConstantExpression.Zero;
        }
        public override string ToString()
        {
            return Name;
        }

        public override IEnumerable<string> Variables()
        {
            yield return Name;
        }
        public override string ToStringRpn()
        {
            return Name;
        }
    }
}
