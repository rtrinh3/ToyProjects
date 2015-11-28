#ifndef OP
#error Must define OP, the operator
#endif

#ifndef FUNC
#error Must define FUNC, the functor which implements the operator
#endif

template <size_t I1, size_t I2>
auto operator OP(Placeholder<I1> lhs, Placeholder<I2> rhs) {
	return makeExpression(FUNC{}, lhs, rhs);
}

template <class F1, class L1, class R1, class F2, class L2, class R2>
auto operator OP(Expression<F1, L1, R1> lhs, Expression<F2, L2, R2> rhs) {
	return makeExpression(FUNC{}, lhs, rhs);
}

template <size_t I, class F, class L, class R>
auto operator OP(Placeholder<I> lhs, Expression<F, L, R> rhs) {
	return makeExpression(FUNC{}, lhs, rhs);
}

template <class F, class L, class R, size_t I>
auto operator OP(Expression<F, L, R> lhs, Placeholder<I> rhs) {
	return makeExpression(FUNC{}, lhs, rhs);
}

template <size_t I, typename T>
auto operator OP(Placeholder<I> lhs, T&& rhs) {
	return makeExpression(FUNC{}, lhs, makeConstant(rhs));
}

template <typename T, size_t I>
auto operator OP(T&& lhs, Placeholder<I> rhs) {
	return makeExpression(FUNC{}, makeConstant(lhs), rhs);
}

template <class F, class L, class R, typename T>
auto operator OP(Expression<F, L, R> lhs, T&& rhs) {
	return makeExpression(FUNC{}, lhs, makeConstant(rhs));
}

template <typename T, class F, class L, class R>
auto operator OP(T&& lhs, Expression<F, L, R> rhs) {
	return makeExpression(FUNC{}, makeConstant(lhs), rhs);
}
