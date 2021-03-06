/*
 * Copyright (c) 2009-2019: G-CSC, Goethe University Frankfurt
 *
 * Author: Markus Breit
 * Creation date: 2017-10-30
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

#include "hh.h"

#include "lib_disc/spatial_disc/disc_util/geom_provider.h"  // for GeomProvider


namespace ug {
namespace neuro_collection {


template <typename TDomain>
void HH<TDomain>::set_conductances(number gk, number gna)
{
	m_gK = gk;
	m_gNa = gna;
}


template <typename TDomain>
void HH<TDomain>::set_reversal_potentials(number ek, number ena)
{
	m_eK = ek;
	m_eNa = ena;
}


template <typename TDomain>
void HH<TDomain>::set_reference_time(number refTime)
{
	m_refTime = refTime;
}


template <typename TDomain>
void HH<TDomain>::use_exact_gating_mode(number timeStep)
{
	m_bVoltageExplicitDiscMode = true;
	m_VEDMdt = timeStep;
}


template <typename TDomain>
void HH<TDomain>::use_gating_explicit_current_mode()
{
	m_bGatingExplicitCurrentMode = true;
}


template <typename TDomain>
HH<TDomain>::HH(const std::vector<std::string>& fcts, const std::vector<std::string>& subsets)
: IMembraneTransporter(fcts), IElemDisc<TDomain>(fcts, subsets),
  m_gK(2e-11), m_gNa(2e-11),
  m_eK(-0.077), m_eNa(0.05),
  m_refTime(1.0),
  m_bVoltageExplicitDiscMode(false),
  m_bGatingExplicitCurrentMode(false),
  m_VEDMdt(1e-5),
  m_bNonRegularGrid(false),
  m_bCurrElemIsHSlave(false)
{
	// nothing to do
}


template <typename TDomain>
HH<TDomain>::HH(const char* fcts, const char* subsets)
: IMembraneTransporter(fcts), IElemDisc<TDomain>(fcts, subsets),
  m_gK(2e-11), m_gNa(2e-11),
  m_eK(-0.077), m_eNa(0.05),
  m_refTime(1.0),
  m_bVoltageExplicitDiscMode(false),
  m_bGatingExplicitCurrentMode(false),
  m_VEDMdt(1e-5),
  m_bNonRegularGrid(false),
  m_bCurrElemIsHSlave(false)
{
	// nothing to do
}


template <typename TDomain>
HH<TDomain>::~HH()
{
	// nothing to do
}


template <typename TDomain>
void HH<TDomain>::calc_flux(const std::vector<number>& u, GridObject* e, std::vector<number>& flux) const
{
	const number vm = u[_PHII_] - u[_PHIO_]; // membrane potential
	const number n = u[_N_];                 // gating state n
	const number m = u[_M_];                 // gating state m
	const number h = u[_H_];                 // gating state h

	const number currentK = m_gK * n*n*n*n * (vm - m_eK);
	const number currentNa = m_gNa * m*m*m*h * (vm - m_eNa);

	flux[0] = currentK + currentNa;
}


template <typename TDomain>
void HH<TDomain>::calc_flux_deriv(const std::vector<number>& u, GridObject* e, std::vector<std::vector<std::pair<size_t, number> > >& flux_derivs) const
{
	const number vm = u[_PHII_] - u[_PHIO_]; // membrane potential
	const number n = u[_N_];                 // gating state n
	const number m = u[_M_];                 // gating state m
	const number h = u[_H_];                 // gating state h

	size_t i = 0;
	if (!has_constant_value(_PHII_))
	{
		flux_derivs[0][i].first = local_fct_index(_PHII_);
		flux_derivs[0][i].second = m_gK*n*n*n*n + m_gNa*m*m*m*h;
		++i;
	}
	if (!has_constant_value(_PHIO_))
	{
		flux_derivs[0][i].first = local_fct_index(_PHIO_);
		flux_derivs[0][i].second = - m_gK*n*n*n*n - m_gNa*m*m*m*h;
		++i;
	}

	if (!m_bGatingExplicitCurrentMode)
	{
		flux_derivs[0][i].first = local_fct_index(_N_);
		flux_derivs[0][i].second = m_gK * 4.0*n*n*n * (vm - m_eK);
		++i;

		flux_derivs[0][i].first = local_fct_index(_M_);
		flux_derivs[0][i].second = m_gNa * 3.0*m*m*h * (vm - m_eNa);
		++i;

		flux_derivs[0][i].first = local_fct_index(_H_);
		flux_derivs[0][i].second = m_gNa * m*m*m * (vm - m_eNa);
	}
	else
	{
		flux_derivs[0][i].first = local_fct_index(_N_);
		flux_derivs[0][i].second = 0.0;
		++i;

		flux_derivs[0][i].first = local_fct_index(_M_);
		flux_derivs[0][i].second = 0.0;
		++i;

		flux_derivs[0][i].first = local_fct_index(_H_);
		flux_derivs[0][i].second = 0.0;
	}
}


template <typename TDomain>
size_t HH<TDomain>::n_dependencies() const
{
	size_t n = 5;
	if (has_constant_value(_PHII_))
		--n;
	if (has_constant_value(_PHIO_))
		--n;

	return n;
}


template <typename TDomain>
size_t HH<TDomain>::n_fluxes() const
{
	return 1;
}


template <typename TDomain>
const std::pair<size_t,size_t> HH<TDomain>::flux_from_to(size_t flux_i) const
{
	// current goes from the inside charge density to the outside charge density
	size_t from, to;
	if (is_supplied(_PHIO_)) to = local_fct_index(_PHIO_); else to = InnerBoundaryConstants::_IGNORE_;
	if (is_supplied(_PHII_)) from = local_fct_index(_PHII_); else from = InnerBoundaryConstants::_IGNORE_;

	return std::pair<size_t, size_t>(from, to);
}


template <typename TDomain>
const std::string HH<TDomain>::name() const
{
	return std::string("Hodgkin-Huxley");
}


template <typename TDomain>
void HH<TDomain>::check_supplied_functions() const
{
	// Check that not both, inner and outer charge are not supplied;
	// in that case, calculation of a current would be of no consequence.
	if (!is_supplied(_PHII_) && !is_supplied(_PHIO_))
	{
		UG_THROW("Supplying neither inner nor outer charge density is not allowed.\n"
				"This would mean that the current calculation would be of no consequence\n"
				"and this channel would not do anything.");
	}
}


template <typename TDomain>
void HH<TDomain>::print_units() const
{
	std::string nm = name();
	size_t n = nm.size();
	UG_LOG(std::endl);
	UG_LOG("+------------------------------------------------------------------------------+"<< std::endl);
	UG_LOG("|  Units used in the implementation of " << nm << std::string(n>=40?0:40-n, ' ') << "|" << std::endl);
	UG_LOG("|------------------------------------------------------------------------------|"<< std::endl);
	UG_LOG("|    Input                                                                     |"<< std::endl);
	UG_LOG("|      Phi_i    inner potential  V                                             |"<< std::endl);
	UG_LOG("|      Phi_o    outer potential  V                                             |"<< std::endl);
	UG_LOG("|      n        gating param     1 (no dimension)                              |"<< std::endl);
	UG_LOG("|      m        gating param     1 (no dimension)                              |"<< std::endl);
	UG_LOG("|      h        gating param     1 (no dimension)                              |"<< std::endl);
	UG_LOG("|                                                                              |"<< std::endl);
	UG_LOG("|      E_K      K reversal potential    V                                      |"<< std::endl);
	UG_LOG("|      E_Na     Na reversal potential   V                                      |"<< std::endl);
	UG_LOG("|      g_K      K channel conductance   C/(Vs)                                 |"<< std::endl);
	UG_LOG("|      g_Na     Na channel conductance  C/(Vs)                                 |"<< std::endl);
	UG_LOG("|                                                                              |"<< std::endl);
	UG_LOG("|    Output                                                                    |"<< std::endl);
	UG_LOG("|      current  C/s                                                            |"<< std::endl);
	UG_LOG("+------------------------------------------------------------------------------+"<< std::endl);
	UG_LOG(std::endl);
}





static number alpha_n(number u)
{
	const number x = -(u + 0.055);
	if (fabs(x) > 1e-10)
		return 1e4*x / (exp(100.0*x) - 1.0);

	return 1e4 * (0.01 - 0.5*x);
}

static number beta_n(number u)
{
	return 125.0 * exp(-(u + 0.065) / 0.08);
}

static number n_infty(number u)
{
	return alpha_n(u) / (alpha_n(u) + beta_n(u));
}

static number tau_n(number u)
{
	return 1.0 / (alpha_n(u) + beta_n(u));
}

static number d_alpha_n_d_vm(number vm)
{
	const number x = -(vm + 0.055);
	if (fabs(x) > 1e-10)
		return 1e4*(-vm*(exp(100.0*x) - 1.0) + x*100.0*exp(100.0*x))
			   / ((exp(100.0*x) - 1.0)*(exp(100.0*x) - 1.0));
	return 1e4 * 0.5*vm;
}

static number d_beta_n_d_vm(number vm)
{
	return -125.0 / 0.08 * exp(-(vm + 0.065) / 0.08);
}

static number d_n_infty_d_vm(number vm)
{
	const number alpha = alpha_n(vm);
	const number beta = beta_n(vm);
	const number d_alpha = d_alpha_n_d_vm(vm);
	const number d_beta = d_beta_n_d_vm(vm);

	return (d_alpha * (alpha + beta) - alpha * (d_alpha + d_beta))
			/ ((alpha + beta) * (alpha + beta));
}

static number d_tau_n_d_vm(number vm)
{
	const number alpha = alpha_n(vm);
	const number beta = beta_n(vm);
	const number d_alpha = d_alpha_n_d_vm(vm);
	const number d_beta = d_beta_n_d_vm(vm);

	return (d_alpha + d_beta) / ((alpha + beta) * (alpha + beta));
}


static number alpha_m(number u)
{
	const number x = -(u + 0.04);
	if (fabs(x) > 1e-10)
		return  1e5*x / (exp(100.0*x) - 1.0);

	return 1e5 * (0.01 - 0.5*x);
}

static number beta_m(number u)
{
	return 4e3 * exp(-(u + 0.065) / 0.018);
}

static number m_infty(number u)
{
	return alpha_m(u) / (alpha_m(u) + beta_m(u));
}

static number tau_m(number u)
{
	return 1.0 / (alpha_m(u) + beta_m(u));
}

static number d_alpha_m_d_vm(number vm)
{
	const number x = -(vm + 0.04);
	if (fabs(x) > 1e-10)
		return 1e5*(-vm*(exp(100.0*x) - 1.0) + x*100.0*exp(100.0*x))
			   / ((exp(100.0*x) - 1.0)*(exp(100.0*x) - 1.0));
	return 1e5 * 0.5*vm;
}

static number d_beta_m_d_vm(number vm)
{
	return -4e3 / 0.018 * exp(-(vm + 0.065) / 0.018);
}

static number d_m_infty_d_vm(number vm)
{
	const number alpha = alpha_m(vm);
	const number beta = beta_m(vm);
	const number d_alpha = d_alpha_m_d_vm(vm);
	const number d_beta = d_beta_m_d_vm(vm);

	return (d_alpha * (alpha + beta) - alpha * (d_alpha + d_beta))
			/ ((alpha + beta) * (alpha + beta));
}

static number d_tau_m_d_vm(number vm)
{
	const number alpha = alpha_m(vm);
	const number beta = beta_m(vm);
	const number d_alpha = d_alpha_m_d_vm(vm);
	const number d_beta = d_beta_m_d_vm(vm);

	return (d_alpha + d_beta) / ((alpha + beta) * (alpha + beta));
}


static number alpha_h(number u)
{
	return 70.0 * exp(-(u + 0.065) / 0.02);
}

static number beta_h(number u)
{
	return 1e3 / (exp(-(u + 0.035) / 0.01) + 1.0);
}

static number h_infty(number u)
{
	return alpha_h(u) / (alpha_h(u) + beta_h(u));
}

static number tau_h(number u)
{
	return 1.0 / (alpha_h(u) + beta_h(u));
}

static number d_alpha_h_d_vm(number vm)
{
	return -70.0 / 0.02 * exp(-(vm + 0.065) / 0.02);
}

static number d_beta_h_d_vm(number vm)
{
	const number tmp = exp(-(vm + 0.035) / 0.01) + 1.0;
	return -1e3 / 0.01 * exp(-(vm + 0.035) / 0.01)
			/ (tmp*tmp);
}

static number d_h_infty_d_vm(number vm)
{
	const number alpha = alpha_h(vm);
	const number beta = beta_h(vm);
	const number d_alpha = d_alpha_h_d_vm(vm);
	const number d_beta = d_beta_h_d_vm(vm);

	return (d_alpha * (alpha + beta) - alpha * (d_alpha + d_beta))
			/ ((alpha + beta) * (alpha + beta));
}

static number d_tau_h_d_vm(number vm)
{
	const number alpha = alpha_h(vm);
	const number beta = beta_h(vm);
	const number d_alpha = d_alpha_h_d_vm(vm);
	const number d_beta = d_beta_h_d_vm(vm);

	return (d_alpha + d_beta) / ((alpha + beta) * (alpha + beta));
}




template <typename TDomain>
void HH<TDomain>::prepare_setting(const std::vector<LFEID>& vLfeID, bool bNonRegularGrid)
{
	// check that Lagrange 1st order
	for (size_t i = 0; i < vLfeID.size(); ++i)
		if (vLfeID[i].type() != LFEID::LAGRANGE || vLfeID[i].order() != 1)
			UG_THROW("Hodgkin-Huxley: 1st order Lagrange functions expected.");

	// update assemble functions
	m_bNonRegularGrid = bNonRegularGrid;
	register_all_fv1_funcs();
}


template <typename TDomain>
bool HH<TDomain>::use_hanging() const
{
	return true;
}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::prep_elem_loop(const ReferenceObjectID roid, const int si)
{}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::fsh_elem_loop()
{}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::
prep_elem(const LocalVector& u, GridObject* elem, const ReferenceObjectID roid, const MathVector<dim> vCornerCoords[])
{
#ifdef UG_PARALLEL
	DistributedGridManager& dgm = *this->approx_space()->domain()->grid()->distributed_grid_manager();
	m_bCurrElemIsHSlave = dgm.get_status(elem) & ES_H_SLAVE;
#endif

	// on horizontal interfaces: only treat hmasters
	if (m_bCurrElemIsHSlave) return;

	// update geometry for this element
	static TFVGeom& geo = GeomProvider<TFVGeom>::get();
	try {geo.update(elem, vCornerCoords, &(this->subset_handler()));}
	UG_CATCH_THROW("HH::prep_elem: Cannot update finite volume geometry.");
}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::add_def_A_elem
(
	LocalVector& d,
	const LocalVector& u,
	GridObject* elem,
	const MathVector<dim> vCornerCoords[]
)
{
	// on horizontal interfaces: only treat hmasters
	if (m_bCurrElemIsHSlave) return;

	// get finite volume geometry
	static TFVGeom& fvgeom = GeomProvider<TFVGeom>::get();

	// strictly speaking, we only need ODE assemblings here,
	// but it does not hurt to integrate over the boxes either
	for (size_t i = 0; i < fvgeom.num_bf(); ++i)
	{
		// get current BF
		const typename TFVGeom::BF& bf = fvgeom.bf(i);

		// get associated node
		const int co = bf.node_id();

		const number vm = u(_PHII_, co) * scale_input(_PHII_) - u(_PHIO_, co) * scale_input(_PHIO_);
		const number n = u(_N_, co);
		const number m = u(_M_, co);
		const number h = u(_H_, co);

		if (!m_bVoltageExplicitDiscMode)
		{
			d(_N_, co) -= (n_infty(vm) - n) / tau_n(vm) * m_refTime * bf.volume();
			d(_M_, co) -= (m_infty(vm) - m) / tau_m(vm) * m_refTime * bf.volume();
			d(_H_, co) -= (h_infty(vm) - h) / tau_h(vm) * m_refTime * bf.volume();
		}
		else
		{
			d(_N_, co) -= (n_infty(vm) - n) * (1.0 - exp(-m_VEDMdt*m_refTime/tau_n(vm))) * bf.volume() / m_VEDMdt;
			d(_M_, co) -= (m_infty(vm) - m) * (1.0 - exp(-m_VEDMdt*m_refTime/tau_m(vm))) * bf.volume() / m_VEDMdt;
			d(_H_, co) -= (h_infty(vm) - h) * (1.0 - exp(-m_VEDMdt*m_refTime/tau_h(vm))) * bf.volume() / m_VEDMdt;
		}
	}
}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::add_def_M_elem
(
	LocalVector& d,
	const LocalVector& u,
	GridObject* elem,
	const MathVector<dim> vCornerCoords[]
)
{
	// on horizontal interfaces: only treat hmasters
	if (m_bCurrElemIsHSlave) return;

	// get finite volume geometry
	static TFVGeom& fvgeom = GeomProvider<TFVGeom>::get();

	// strictly speaking, we only need ODE assemblings here,
	// but it does not hurt to integrate over the boxes either
	for (size_t i = 0; i < fvgeom.num_bf(); ++i)
	{
		// get current BF
		const typename TFVGeom::BF& bf = fvgeom.bf(i);

		// get associated node
		const int co = bf.node_id();

		d(_N_, co) += u(_N_, co) * bf.volume();
		d(_M_, co) += u(_M_, co) * bf.volume();
		d(_H_, co) += u(_H_, co) * bf.volume();
	}
}

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void HH<TDomain>::
add_rhs_elem(LocalVector& rhs, GridObject* elem, const MathVector<dim> vCornerCoords[])
{}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::add_jac_A_elem
(
	LocalMatrix& J,
	const LocalVector& u,
	GridObject* elem,
	const MathVector<dim> vCornerCoords[]
)
{
	// on horizontal interfaces: only treat hmasters
	if (m_bCurrElemIsHSlave) return;

	// get finite volume geometry
	static TFVGeom& fvgeom = GeomProvider<TFVGeom>::get();

	// strictly speaking, we only need ODE assemblings here,
	// but it does not hurt to integrate over the boxes either
	for (size_t i = 0; i < fvgeom.num_bf(); ++i)
	{
		// get current BF
		const typename TFVGeom::BF& bf = fvgeom.bf(i);

		// get associated node
		const int co = bf.node_id();

		const number vm = u(_PHII_, co) * scale_input(_PHII_) - u(_PHIO_, co) * scale_input(_PHIO_);
		const number n = u(_N_, co);
		const number m = u(_M_, co);
		const number h = u(_H_, co);

		const number t_n = tau_n(vm);
		const number t_m = tau_m(vm);
		const number t_h = tau_h(vm);

		const number dn_dvm = (d_n_infty_d_vm(vm) * t_n - (n_infty(vm) - n) * d_tau_n_d_vm(vm)) / (t_n*t_n);
		const number dm_dvm = (d_m_infty_d_vm(vm) * t_m - (m_infty(vm) - m) * d_tau_m_d_vm(vm)) / (t_m*t_m);
		const number dh_dvm = (d_h_infty_d_vm(vm) * t_h - (h_infty(vm) - h) * d_tau_h_d_vm(vm)) / (t_h*t_h);

		J(_N_, co, _N_, co) += 1.0 / t_n * m_refTime * bf.volume();
		J(_N_, co, _PHII_, co) -= dn_dvm * scale_input(_PHII_) * m_refTime * bf.volume();
		J(_N_, co, _PHIO_, co) += dn_dvm * scale_input(_PHIO_) * m_refTime * bf.volume();

		J(_M_, co, _M_, co) += 1.0 / t_m * m_refTime * bf.volume();
		J(_M_, co, _PHII_, co) -= dm_dvm * scale_input(_PHII_) * m_refTime * bf.volume();
		J(_M_, co, _PHIO_, co) += dm_dvm * scale_input(_PHIO_) * m_refTime * bf.volume();

		J(_H_, co, _H_, co) += 1.0 / t_h * m_refTime * bf.volume();
		J(_H_, co, _PHII_, co) -= dh_dvm * scale_input(_PHII_) * m_refTime * bf.volume();
		J(_H_, co, _PHIO_, co) += dh_dvm * scale_input(_PHIO_) * m_refTime * bf.volume();
	}
}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::add_jac_M_elem
(
	LocalMatrix& J,
	const LocalVector& u,
	GridObject* elem,
	const MathVector<dim> vCornerCoords[]
)
{
	// on horizontal interfaces: only treat hmasters
	if (m_bCurrElemIsHSlave) return;

	// get finite volume geometry
	static TFVGeom& fvgeom = GeomProvider<TFVGeom>::get();

	// strictly speaking, we only need ODE assemblings here,
	// but it does not hurt to integrate over the boxes either
	for (size_t i = 0; i < fvgeom.num_bf(); ++i)
	{
		// get current BF
		const typename TFVGeom::BF& bf = fvgeom.bf(i);

		// get associated node
		const int co = bf.node_id();

		J(_N_, co, _N_, co) += bf.volume();
		J(_M_, co, _M_, co) += bf.volume();
		J(_H_, co, _H_, co) += bf.volume();
	}
}



// register for 2D and 3d
template <typename TDomain>
void HH<TDomain>::
register_all_fv1_funcs()
{
//	get all grid element types in the dimension below
	typedef typename domain_traits<dim>::ManifoldElemList ElemList;

//	switch assemble functions
	boost::mpl::for_each<ElemList>(RegisterFV1(this));
}


template <typename TDomain>
template <typename TElem, typename TFVGeom>
void HH<TDomain>::
register_fv1_func()
{
	ReferenceObjectID id = geometry_traits<TElem>::REFERENCE_OBJECT_ID;
	typedef HH<TDomain> T;

	this->clear_add_fct(id);
	this->set_prep_elem_loop_fct(	id, &T::template prep_elem_loop<TElem, TFVGeom>);
	this->set_prep_elem_fct(		id, &T::template prep_elem<TElem, TFVGeom>);
	this->set_fsh_elem_loop_fct( 	id, &T::template fsh_elem_loop<TElem, TFVGeom>);
	this->set_add_jac_A_elem_fct(	id, &T::template add_jac_A_elem<TElem, TFVGeom>);
	this->set_add_jac_M_elem_fct(	id, &T::template add_jac_M_elem<TElem, TFVGeom>);
	this->set_add_def_A_elem_fct(	id, &T::template add_def_A_elem<TElem, TFVGeom>);
	this->set_add_def_M_elem_fct(	id, &T::template add_def_M_elem<TElem, TFVGeom>);
	this->set_add_rhs_elem_fct(	 	id, &T::template add_rhs_elem<TElem, TFVGeom>);
}




// explicit template specializations
#ifdef UG_DIM_1
	template class HH<Domain1d>;
#endif
#ifdef UG_DIM_2
	template class HH<Domain2d>;
#endif
#ifdef UG_DIM_3
	template class HH<Domain3d>;
#endif



} // namespace neuro_collection
} // namespace ug

