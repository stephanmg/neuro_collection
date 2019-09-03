/*
 * Copyright (c) 2009-2019: G-CSC, Goethe University Frankfurt
 *
 * Author: Myra Huymayer
 * Creation date: 2017-09-21
 *
 * This file is part of NeuroBox, which is based on UG4.
 *
 * NeuroBox and UG4 are free software: You can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3
 * (as published by the Free Software Foundation) with the following additional
 * attribution requirements (according to LGPL/GPL v3 §7):
 *
 * (1) The following notice must be displayed in the appropriate legal notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 *
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 *
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating PDE based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * "Stepniewski, M., Breit, M., Hoffer, M. and Queisser, G.
 *   NeuroBox: computational mathematics in multiscale neuroscience.
 *   Computing and visualization in science (2019).
 * "Breit, M. et al. Anatomically detailed and large-scale simulations studying
 *   synapse loss and synchrony using NeuroBox. Front. Neuroanat. 10 (2016), 8"
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

#include "hh_util.h"
#include <cmath>


namespace ug {
namespace neuro_collection {


number alpha_n(number vm)
{
	number x = 10.0 - (vm + 65.0);
	if (std::abs(x) > 1e-7)
		return 0.01 * x/(std::exp(x/10.0)-1.0);

	return 0.1 - 0.005*x;
}

number beta_n(number vm)
{
	return 0.125*std::exp(-(vm + 65.0)/80.0);
}

number n_infty(number vm)
{
	return alpha_n(vm)/(alpha_n(vm) + beta_n(vm));
}

number tau_n(number vm)
{
	return 1.0/(alpha_n(vm)+beta_n(vm));
}


number alpha_m(number vm)
{
	number x = 25.0 - (vm + 65.0);
	if (fabs(x) > 1e-7)
		return 0.1 * x/(std::exp(x/10.0)-1.0);

	return 1.0 - 0.05*x;
}

number beta_m(number vm)
{
	return 4.0 * std::exp(-(vm + 65.0)/18.0);
}

number m_infty(number vm)
{
	return alpha_m(vm)/(alpha_m(vm) + beta_m(vm));
}

number tau_m(number vm)
{
	return 1.0/(alpha_m(vm) + beta_m(vm));
}


number alpha_h(number vm)
{
	return 0.07*std::exp(-(vm + 65.0)/20.0);
}

number beta_h(number vm)
{
	return 1/(std::exp((30.0 - (vm +65.0))/ 10.0 ) + 1.0 );
}

number h_infty(number vm)
{
	return alpha_h(vm)/(alpha_h(vm) + beta_h(vm));
}

number tau_h(number vm)
{
	return 1.0/(alpha_h(vm) + beta_h(vm));
}



} // namespace neuro_collection
} // namespace ug
