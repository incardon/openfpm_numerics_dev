/*
 * vector_dist_operators_apply_kernel.hpp
 *
 *  Created on: Jun 19, 2016
 *      Author: i-bird
 */

#ifndef OPENFPM_NUMERICS_SRC_OPERATORS_VECTOR_VECTOR_DIST_OPERATORS_APPLY_KERNEL_HPP_
#define OPENFPM_NUMERICS_SRC_OPERATORS_VECTOR_VECTOR_DIST_OPERATORS_APPLY_KERNEL_HPP_


/*! \brief is_expression check if a type is simple a type or is just an encapsulation of an expression
 *
 * return true if T::is_expression is a valid expression
 *
 */

template<typename ObjType, typename Sfinae = void>
struct is_expression: std::false_type {};

template<typename ObjType>
struct is_expression<ObjType, typename Void<typename ObjType::is_expression>::type> : std::true_type
{};

template<typename exp, bool is_exp = is_expression<exp>::value>
struct apply_kernel_rtype
{
	typedef typename exp::return_type rtype;
};

template<typename exp>
struct apply_kernel_rtype<exp,false>
{
	typedef exp rtype;
};


/*! \brief Meta-function to return a compatible zero-element
 *
 *
 */
template<typename rtype>
struct set_zero
{
	static rtype create()
	{
		return 0.0;
	}
};

template<unsigned int dim, typename T>
struct set_zero<Point<dim,T>>
{
	static Point<dim,T> create()
	{
		Point<dim,T> ret;

		ret.zero();
		return ret;
	}
};

/*! \brief Apply the kernel to particle differently that is a number or is an expression
 *
 *
 */
template<typename T, typename vector, typename exp,typename NN_type, typename Kernel, typename rtype, bool is_exp=is_expression<T>::value>
struct apply_kernel_is_number_or_expression
{
	inline static typename std::remove_reference<rtype>::type apply(const vector & vd, NN_type & cl, const exp & v_exp, const vect_dist_key_dx & key, Kernel & lker)
	{
	    // accumulator
		typename std::remove_reference<rtype>::type pse = set_zero<typename std::remove_reference<rtype>::type>::create();

	    // position of particle p
	    Point<vector::dims,typename vector::stype> p = vd.getPos(key);

	    // property of the particle x
	    rtype prp_p = v_exp.value(key);

	    // Get the neighborhood of the particle
	    auto NN = cl.template getNNIterator<NO_CHECK>(cl.getCell(p));
	    while(NN.isNext())
	    {
	    	auto nnp = NN.get();

	    	// Calculate contribution given by the kernel value at position p,
	    	// given by the Near particle, exclude itself
	    	if (nnp != key.getKey())
	    	{
	    	    // property of the particle x
	    		rtype prp_q = v_exp.value(nnp);

	    	    // position of the particle q
	    		Point<vector::dims,typename vector::stype> q = vd.getPos(nnp);

	    	    pse += lker.value(p,q,prp_p,prp_q);
	    	}

	    	// Next particle
	    	++NN;
	    }

	    return pse;
	}
};


/*! \brief Apply the kernel to particle differently that is a number or is an expression
 *
 *
 */
template<typename vector, typename exp,typename NN_type, typename Kernel, typename rtype>
struct apply_kernel_is_number_or_expression_sim
{
	inline static typename std::remove_reference<rtype>::type apply(const vector & vd, NN_type & cl, const vect_dist_key_dx & key, Kernel & lker)
	{
	    // accumulator
		typename std::remove_reference<rtype>::type pse = set_zero<typename std::remove_reference<rtype>::type>::create();

	    // position of particle p
	    Point<vector::dims,typename vector::stype> p = vd.getPos(key);

	    // Get the neighborhood of the particle
	    auto NN = cl.template getNNIterator<NO_CHECK>(cl.getCell(p));
	    while(NN.isNext())
	    {
	    	auto nnp = NN.get();

	    	// Calculate contribution given by the kernel value at position p,
	    	// given by the Near particle, exclude itself
	    	if (nnp != key.getKey())
	    	{
	    	    // position of the particle q
	    		Point<vector::dims,typename vector::stype> q = vd.getPos(nnp);

	    	    pse += lker.value(p,q);
	    	}

	    	// Next particle
	    	++NN;
	    }

	    return pse;
	}
};



/*! \brief Apply the kernel to particle differently that is a number or is an expression
 *
 *
 */
template<typename T, typename vector, typename exp,typename NN_type, typename Kernel, typename rtype, bool is_exp=is_expression<T>::value>
struct apply_kernel_is_number_or_expression_gen
{
	inline static typename std::remove_reference<rtype>::type apply(const vector & vd, NN_type & cl, const exp & v_exp, const vect_dist_key_dx & key, Kernel & lker)
	{
	    // accumulator
	    typename std::remove_reference<rtype>::type pse = set_zero<typename std::remove_reference<rtype>::type>::create();

	    // property of the particle x
	    rtype prp_p = v_exp.value(key);

	    // position of particle p
	    auto & p = vd.getPos(key);

	    // Get the neighborhood of the particle
	    auto NN = cl.template getNNIterator<NO_CHECK>(cl.getCell(p));
	    while(NN.isNext())
	    {
	    	auto nnp = NN.get();

	    	// Calculate contribution given by the kernel value at position p,
	    	// given by the Near particle, exclude itself
	    	if (nnp != key.getKey())
	    	{
	    	    // property of the particle x
	    		rtype prp_q = v_exp.value(nnp);

	    	    pse += lker.value(key.getKey(),nnp,prp_p,prp_q,vd);
	    	}

	    	// Next particle
	    	++NN;
	    }

	    return pse;
	}
};


/*! \brief Apply kernel operation
 *
 * \tparam exp1 expression1
 * \tparam NN list
 *
 */
template <typename exp1,typename vector_type>
class vector_dist_expression_op<exp1,vector_type,VECT_APPLYKER_IN>
{
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<0>>::type NN;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<1>>::type Kernel;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<2>>::type vector_orig;

	const exp1 o1;
	NN & cl;
	Kernel & ker;
	const vector_orig & vd;

	typedef typename apply_kernel_rtype<decltype(o1.value(vect_dist_key_dx(0)))>::rtype rtype;

public:

	/*! \brief This function must be called before value
	 *
	 * it initialize the expression if needed
	 *
	 */
	inline void init() const
	{
		o1.init();
	}

	vector_dist_expression_op(const exp1 & o1, NN & cl, Kernel & ker, const vector_orig & vd)
	:o1(o1),cl(cl),ker(ker),vd(vd)
	{}

	/*! \brief Evaluate the expression
	 *
	 * \param key where to evaluate the expression
	 *
	 */
	inline typename std::remove_reference<rtype>::type value(const vect_dist_key_dx & key) const
	{
		return apply_kernel_is_number_or_expression<decltype(o1.value(key)),vector_orig,exp1,NN,Kernel,rtype>::apply(vd,cl,o1,key,ker);
	}
};

/*! \brief Apply kernel operation
 *
 * \tparam exp1 expression1
 * \tparam NN list
 *
 */
template <typename exp1,typename vector_type>
class vector_dist_expression_op<exp1,vector_type,VECT_APPLYKER_IN_SIM>
{
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<0>>::type NN;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<1>>::type Kernel;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<2>>::type vector_orig;

	NN & cl;
	Kernel & ker;
	const vector_orig & vd;

	typedef typename apply_kernel_rtype<decltype(std::declval<Kernel>().value(Point<vector_orig::dims,typename vector_orig::stype>(0.0), Point<vector_orig::dims,typename vector_orig::stype>(0.0) ) )>::rtype rtype;


public:

	/*! \brief This function must be called before value
	 *
	 * it initialize the expression if needed
	 *
	 */
	inline void init() const
	{
	}

	vector_dist_expression_op(NN & cl, Kernel & ker, const vector_orig & vd)
	:cl(cl),ker(ker),vd(vd)
	{}

	/*! \brief Evaluate the expression
	 *
	 * \param key where to evaluate the expression
	 *
	 */
	inline typename std::remove_reference<rtype>::type value(const vect_dist_key_dx & key) const
	{
		return apply_kernel_is_number_or_expression_sim<vector_orig,exp1,NN,Kernel,rtype>::apply(vd,cl,key,ker);
	}
};


/*! \brief Apply kernel operation
 *
 * \tparam exp1 expression1
 * \tparam NN list
 *
 */
template <typename exp1,typename vector_type>
class vector_dist_expression_op<exp1,vector_type,VECT_APPLYKER_IN_GEN>
{
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<0>>::type NN;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<1>>::type Kernel;
	typedef typename boost::mpl::at<vector_type,boost::mpl::int_<2>>::type vector_orig;

	const exp1 o1;
	NN & cl;
	Kernel & ker;
	const vector_orig & vd;

	typedef typename apply_kernel_rtype<decltype(o1.value(vect_dist_key_dx(0)))>::rtype rtype;

public:

	/*! \brief This function must be called before value
	 *
	 * it initialize the expression if needed
	 *
	 */
	inline void init() const
	{
		o1.init();
	}

	vector_dist_expression_op(const exp1 & o1, NN & cl, Kernel & ker, const vector_orig & vd)
	:o1(o1),cl(cl),ker(ker),vd(vd)
	{}

	/*! \brief Evaluate the expression
	 *
	 * \param key where to evaluate the expression
	 *
	 */
	inline typename std::remove_reference<rtype>::type value(const vect_dist_key_dx & key) const
	{
		return apply_kernel_is_number_or_expression_gen<decltype(o1.value(key)),vector_orig,exp1,NN,Kernel,rtype>::apply(vd,cl,o1,key,ker);
	}
};


///////////////////////////////////////////// Apply kernel operator ////
////////////////////////////////////////////////////////////////////////

/* \brief Apply kernel expression
 *
 * \param va vector expression one
 * \param vb vector expression two
 *
 * \return an object that encapsulate the expression
 *
 */
template<typename exp1, typename exp2, unsigned int op1, typename NN, typename Kernel, typename vector_type>
inline vector_dist_expression_op<vector_dist_expression_op<exp1,exp2,op1>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN>
applyKernel_in(const vector_dist_expression_op<exp1,exp2,op1> & va, vector_type & vd, NN & cl, Kernel & ker)
{
	vector_dist_expression_op<vector_dist_expression_op<exp1,exp2,op1>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN> exp_sum(va,cl,ker,vd);

	return exp_sum;
}


/* \brief Apply kernel expression
 *
 * \param va vector expression one
 * \param vb vector expression two
 *
 * \return an object that encapsulate the expression
 *
 */
template<typename exp1, typename exp2, unsigned int op1, typename NN, typename Kernel, typename vector_type>
inline vector_dist_expression_op<vector_dist_expression_op<exp1,exp2,op1>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_GEN>
applyKernel_in_gen(const vector_dist_expression_op<exp1,exp2,op1> & va, vector_type & vd, NN & cl, Kernel & ker)
{
	vector_dist_expression_op<vector_dist_expression_op<exp1,exp2,op1>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_GEN> exp_sum(va,cl,ker,vd);

	return exp_sum;
}


//////////////////////////////////////// For vector expression ///////////////////////

/* \brief Apply kernel expression
 *
 * \param va vector expression one
 * \param vb vector expression two
 *
 * \return an object that encapsulate the expression
 *
 */
template<unsigned int prp, typename NN, typename Kernel, typename vector_type>
inline vector_dist_expression_op<vector_dist_expression<prp,vector_type>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN>
applyKernel_in(const vector_dist_expression<prp,vector_type> & va, vector_type & vd, NN & cl, Kernel & ker)
{
	vector_dist_expression_op<vector_dist_expression<prp,vector_type>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN> exp_sum(va,cl,ker,vd);

	return exp_sum;
}


/* \brief Apply kernel expression
 *
 * \param va vector expression one
 * \param vb vector expression two
 *
 * \return an object that encapsulate the expression
 *
 */
template<unsigned int prp, typename NN, typename Kernel, typename vector_type>
inline vector_dist_expression_op<vector_dist_expression<prp,vector_type>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_GEN>
applyKernel_in_gen(const vector_dist_expression<prp,vector_type> & va, vector_type & vd, NN & cl, Kernel & ker)
{
	vector_dist_expression_op<vector_dist_expression<prp,vector_type>,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_GEN> exp_sum(va,cl,ker,vd);

	return exp_sum;
}


/* \brief Apply kernel expression
 *
 * \param va vector expression one
 * \param vb vector expression two
 *
 * \return an object that encapsulate the expression
 *
 */
template<typename NN, typename Kernel, typename vector_type>
inline vector_dist_expression_op<void,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_SIM>
applyKernel_in_sim(vector_type & vd, NN & cl, Kernel & ker)
{
	vector_dist_expression_op<void,boost::mpl::vector<NN,Kernel,vector_type>,VECT_APPLYKER_IN_SIM> exp_sum(cl,ker,vd);

	return exp_sum;
}


#endif /* OPENFPM_NUMERICS_SRC_OPERATORS_VECTOR_VECTOR_DIST_OPERATORS_APPLY_KERNEL_HPP_ */