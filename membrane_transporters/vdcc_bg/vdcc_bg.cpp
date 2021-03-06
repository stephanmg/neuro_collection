/*
 * Copyright (c) 2009-2019: G-CSC, Goethe University Frankfurt
 *
 * Author: Markus Breit
 * Creation date: 2013-02-05
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

#include "vdcc_bg.h"
#include "lib_disc/function_spaces/grid_function.h"
#include "lib_disc/io/vtkoutput.h"
#include "lib_disc/spatial_disc/disc_util/geom_provider.h"  // for GeomProvider
#include "lib_disc/spatial_disc/elem_disc/inner_boundary/inner_boundary.h"	// InnerBoundaryConstants

namespace ug {
namespace neuro_collection {


static std::vector<std::string> removeEmptyFunctionNames(const std::vector<std::string>& vFct)
{
	std::vector<std::string> res;
	const size_t nFct = vFct.size();
	for (size_t i = 0; i < nFct; ++i)
		if (!vFct[i].empty())
			res.push_back(vFct[i]);
	return res;
}


template<typename TDomain>
VDCC_BG<TDomain>::VDCC_BG
(
	const std::vector<std::string>& fcts,
	const std::vector<std::string>& subsets,
	SmartPtr<ApproximationSpace<TDomain> > approx
)
: IMembraneTransporter(fcts),
  IElemDisc<TDomain>(removeEmptyFunctionNames(fcts), subsets),
  R(8.314), T(310.0), F(96485.0),
  m_dom(approx->domain()), m_mg(m_dom->grid()), m_dd(approx->dof_distribution(GridLevel())),
  m_sh(m_dom->subset_handler()), m_aaPos(m_dom->position_accessor()), m_vSubset(subsets),
  m_localIndicesOffset(0),
  m_gpMGate(3.4, -21.0, 1.5), m_gpHGate(-2.0, -40.0, 75.0),
  m_spVtkOutput(SPNULL), m_spVmGF(SPNULL),
  m_time(0.0), m_oldTime(0.0),
  m_perm(3.8e-19), m_mp(2), m_hp(1), m_channelType(BG_Ltype),
  m_bUseGatingAttachments(true),
  m_initiated(false)
{
	after_construction();
}

template<typename TDomain>
VDCC_BG<TDomain>::VDCC_BG
(
	const char* fcts,
	const char* subsets,
	SmartPtr<ApproximationSpace<TDomain> > approx
)
: IMembraneTransporter(fcts),
  IElemDisc<TDomain>(removeEmptyFunctionNames(TokenizeString(fcts)), TokenizeString(subsets)),
  R(8.314), T(310.0), F(96485.0),
  m_dom(approx->domain()), m_mg(m_dom->grid()), m_dd(approx->dof_distribution(GridLevel())),
  m_sh(m_dom->subset_handler()), m_aaPos(m_dom->position_accessor()), m_vSubset(TokenizeString(subsets)),
  m_localIndicesOffset(0),
  m_gpMGate(3.4, -21.0, 1.5), m_gpHGate(-2.0, -40.0, 75.0),
  m_spVtkOutput(SPNULL), m_spVmGF(SPNULL),
  m_time(0.0), m_oldTime(0.0),
  m_perm(3.8e-19), m_mp(2), m_hp(1), m_channelType(BG_Ltype),
  m_bUseGatingAttachments(true),
  m_initiated(false)
{
	after_construction();
}

template<typename TDomain>
void VDCC_BG<TDomain>::after_construction()
{
	// process subsets

	//	remove white space
	for (size_t i = 0; i < m_vSubset.size(); ++i)
		RemoveWhitespaceFromString(m_vSubset[i]);

	// if no subset passed, clear subsets
	if (m_vSubset.size() == 1 && m_vSubset[0].empty())
		m_vSubset.clear();

	// if subsets passed with separator, but not all tokens filled, throw error
	for (size_t i = 0; i < m_vSubset.size(); ++i)
	{
		if (m_vSubset[i].empty())
		{
			UG_THROW("Error while setting subsets in " << name() << ": passed "
					 "subset string lacks a subset specification at position "
					 << i << "(of " << m_vSubset.size()-1 << ")");
		}
	}

	// if a function name is empty the IElemDisc local indices are set off by 1
	if (!is_supplied(_CCYT_) || !is_supplied(_CEXT_))
		m_localIndicesOffset = 1;

	// if all gating variables are provided, use them instead of attachments
	// we initialize with an L-type channel which does not have an h-gate,
	// so we only check for m here; if the channel type is reset, we need to recheck!
	if (is_supplied(_M_))
		m_bUseGatingAttachments = false;
	else
	{
		// attach attachments
		if (m_mg->template has_attachment<vm_grid_object>(this->m_MGate))
			UG_THROW("Attachment necessary for Borg-Graham channel dynamics "
				"could not be made, since it already exists.");
		m_mg->template attach_to<vm_grid_object>(this->m_MGate);

		if (has_hGate())
		{
			if (m_mg->template has_attachment<vm_grid_object>(this->m_HGate))
				UG_THROW("Attachment necessary for Borg-Graham channel dynamics "
					"could not be made, since it already exists.");
			m_mg->template attach_to<vm_grid_object>(this->m_HGate);
		}

		// create attachment accessors
		m_aaMGate = Grid::AttachmentAccessor<vm_grid_object, ADouble>(*m_mg, m_MGate);
		if (has_hGate())
			m_aaHGate = Grid::AttachmentAccessor<vm_grid_object, ADouble>(*m_mg, m_HGate);
	}


	// attach voltage attachment and create accessor
	if (m_mg->template has_attachment<vm_grid_object>(this->m_Vm))
		UG_THROW("Attachment necessary for Borg-Graham channel dynamics "
				 "could not be made, since it already exists.");
	m_mg->template attach_to<vm_grid_object>(this->m_Vm, true);
	m_aaVm = Grid::AttachmentAccessor<vm_grid_object, ADouble>(*m_mg, m_Vm);

	// check whether necessary functions are given
	check_supplied_functions();
}


template<typename TDomain>
VDCC_BG<TDomain>::~VDCC_BG()
{
	if (m_bUseGatingAttachments)
	{
		m_mg->template detach_from<vm_grid_object>(this->m_MGate);
		m_mg->template detach_from<vm_grid_object>(this->m_HGate);
	}
	m_mg->template detach_from<vm_grid_object>(this->m_Vm);
}


template<typename TDomain>
number VDCC_BG<TDomain>::calc_gating_start(const GatingParams& gp, number Vm) const
{
	return 1.0 / (1.0 + exp(-gp.z * (Vm - gp.V_12) * 1e-3*F/(R*T)));
}


template<typename TDomain>
void VDCC_BG<TDomain>::calc_gating_step(GatingParams& gp, number Vm, number dt, number& currVal)
{
	const number gp_inf = calc_gating_start(gp,Vm);

	// forward step: implicit
	if (dt >= 0)
	{
		// For calculating the next gating step it is recommended to use a time step
		// size no larger than 1e-5s = 1e-2ms in order to meet a sufficient accuracy.
		if (dt > 1e-2)
		{
			number vdcc_dt = 1e-2;
			number t0 = 0.0;

			// loop intermediate time steps until the current intermediate time point
			// is the final time point dt
			while (t0 < dt)
			{
				// compute next time point
				number t = t0 + vdcc_dt;

				// check if out of bounds, if yes:
				// set to final time point and adjust step size accordingly
				if (t > dt)
				{
					t = dt;
					vdcc_dt = dt - t0;
				}

				// compute next gating value
				currVal = (gp.tau_0*currVal + vdcc_dt*gp_inf) / (gp.tau_0 + vdcc_dt);

				// save new time as current time
				t0 = t;
			}
		}
		// sufficiently small time step size already specified
		else
			currVal = (gp.tau_0*currVal + dt*gp_inf) / (gp.tau_0 + dt);
	}

	// backward step: explicit
	else
	{
		// For calculating the next gating step it is recommended to use a time step
		// size no larger than 1e-5s = 1e-2ms in order to meet a sufficient accuracy.
		if (dt < -1e-2)
		{
			number vdcc_dt = -1e-2;
			number t0 = 0.0;

			// loop intermediate time steps until the current intermediate time point
			// is the final time point dt
			while (t0 > dt)
			{
				// compute next time point
				number t = t0 + vdcc_dt;

				// check if out of bounds, if yes:
				// set to final time point and adjust step size accordingly
				if (t < dt)
				{
					t = dt;
					vdcc_dt = dt - t0;
				}

				// compute next gating value
				currVal += vdcc_dt/gp.tau_0 * (gp_inf - currVal);

				// save new time as current time
				t0 = t;
			}
		}
		// sufficiently small time step size already specified
		currVal += dt/gp.tau_0 * (gp_inf - currVal);
	}
}


template <typename TDomain>
number VDCC_BG<TDomain>::average_attachment_value_on_grid_object
(
	const attachment_accessor_type& aa,
	GridObject* o
) const
{
	switch (o->base_object_id())
	{
		case VERTEX:
		{
			Vertex* vrt = static_cast<Vertex*>(o);
			return aa[vrt];
		}
		case EDGE:
			return average_attachment_value_on_grid_object<Edge>(aa, o);
		case FACE:
			return average_attachment_value_on_grid_object<Face>(aa, o);
		default:
		{
			UG_THROW("Base object id must be VERTEX, EDGE or FACE, but is "
				<< o->base_object_id() << ".");
		}
	}
}


template <typename TDomain>
template <typename TBaseElem>
number VDCC_BG<TDomain>::average_attachment_value_on_grid_object
(
	const attachment_accessor_type& aa,
	GridObject* o
) const
{
	TBaseElem* e = static_cast<TBaseElem*>(o);
	number retVal = 0.0;
	const size_t nVrt = e->num_vertices();
	for (size_t v = 0; v < nVrt; ++v)
		retVal += aa[e->vertex(v)];

	return retVal / nVrt;
}



template<typename TDomain>
void VDCC_BG<TDomain>::calc_flux(const std::vector<number>& u, GridObject* e, std::vector<number>& flux) const
{
	const number mGate = m_bUseGatingAttachments ?
		average_attachment_value_on_grid_object(m_aaMGate, e) : u[_M_];
	number gating = pow(mGate, m_mp);
	if (has_hGate())
	{
		const number hGate = m_bUseGatingAttachments ?
			average_attachment_value_on_grid_object(m_aaHGate, e) : u[_H_];
		gating *= pow(hGate, m_hp);
	}

	// flux derived from Goldman-Hodgkin-Katz equation,
	number maxFlux;
	const number vm = average_attachment_value_on_grid_object(m_aaVm, e);
	const number caCyt = u[_CCYT_];		// cytosolic Ca2+ concentration
	const number caExt = u[_CEXT_];		// extracellular Ca2+ concentration

	// near V_m == 0: approximate by first order Taylor to avoid relative errors and div-by-0
	if (fabs(vm) < 1e-8) maxFlux = m_perm * ((caExt - caCyt) - F/(R*T) * (caExt + caCyt)*vm);
	else maxFlux = -m_perm * 2*F/(R*T) * vm * (caExt - caCyt*exp(2*F/(R*T)*vm)) / (1.0 - exp(2*F/(R*T)*vm));

	flux[0] = gating * maxFlux;

	//UG_LOGN(std::setprecision(std::numeric_limits<long double>::digits10 + 1)
	//	<< "VDCC flux: " << flux[0] << " (m: " << mGate << ", vm: " << vm << ")");

	//UG_COND_THROW(flux[0] != flux[0],
	//	"VDCC NaN: gating = " << gating << ", maxFlux = " << maxFlux);
}


template<typename TDomain>
void VDCC_BG<TDomain>::calc_flux_deriv(const std::vector<number>& u, GridObject* e, std::vector<std::vector<std::pair<size_t, number> > >& flux_derivs) const
{
	const number mGate = m_bUseGatingAttachments ?
		average_attachment_value_on_grid_object(m_aaMGate, e) : u[_M_];
	number gating = pow(mGate, m_mp);
	number dGatingdM = m_mp * pow(mGate, m_mp - 1);
	number dGatingdH = gating;
	if (has_hGate())
	{
		const number hGate = m_bUseGatingAttachments ?
			average_attachment_value_on_grid_object(m_aaHGate, e) : u[_H_];
		gating *= pow(hGate, m_hp);
		dGatingdM *= pow(hGate, m_hp);
		dGatingdH *= m_hp * pow(hGate, m_hp - 1);
	}

	number dMaxFlux_dCyt, dMaxFlux_dExt;
	number vm = average_attachment_value_on_grid_object(m_aaVm, e);
	number maxFlux;
	const number caCyt = u[_CCYT_];		// cytosolic Ca2+ concentration
	const number caExt = u[_CEXT_];		// extracellular Ca2+ concentration

	// near V_m == 0: approximate by first order Taylor to avoid relative errors and div-by-0
	if (fabs(vm) < 1e-8)
	{
		maxFlux = m_perm * ((caExt - caCyt) - F/(R*T) * (caExt + caCyt)*vm);
		dMaxFlux_dCyt = m_perm * (-1.0 - F/(R*T) * vm);
		dMaxFlux_dExt = m_perm * (1.0 - F/(R*T) * vm);
	}
	else
	{
		maxFlux = -m_perm * 2*F/(R*T) * vm * (caExt - caCyt*exp(2*F/(R*T)*vm)) / (1.0 - exp(2*F/(R*T)*vm));
		dMaxFlux_dCyt = m_perm * 2*F/(R*T) * vm / (exp(-2*F/(R*T)*vm) - 1.0);
		dMaxFlux_dExt = m_perm * 2*F/(R*T) * vm / (exp(2*F/(R*T)*vm) - 1.0);
	}

	size_t i = 0;
	if (!has_constant_value(_CCYT_))
	{
		flux_derivs[0][i].first = local_fct_index(_CCYT_);
		flux_derivs[0][i].second = gating * dMaxFlux_dCyt;
	}
	if (!has_constant_value(_CEXT_))
	{
		++i;
		flux_derivs[0][i].first = local_fct_index(_CEXT_);
		flux_derivs[0][i].second = gating * dMaxFlux_dExt;
	}

	if (!m_bUseGatingAttachments)
	{
		++i;
		flux_derivs[0][i].first = local_fct_index(_M_);
		flux_derivs[0][i].second = dGatingdM * maxFlux;

		if (has_hGate())
		{
			++i;
			flux_derivs[0][i].first = local_fct_index(_H_);
			flux_derivs[0][i].second = dGatingdH * maxFlux;
		}
	}
}


template<typename TDomain>
size_t VDCC_BG<TDomain>::n_dependencies() const
{
	size_t n = 4;
	if (has_constant_value(_CCYT_)) --n;
	if (has_constant_value(_CEXT_)) --n;
	if (m_bUseGatingAttachments)
		n -= 2;
	else
		if (!has_hGate())
			--n;

	return n;
}


template<typename TDomain>
size_t VDCC_BG<TDomain>::n_fluxes() const
{
	return 1;
}


template<typename TDomain>
const std::pair<size_t,size_t> VDCC_BG<TDomain>::flux_from_to(size_t flux_i) const
{
	size_t from, to;
	if (is_supplied(_CCYT_)) to = local_fct_index(_CCYT_); else to = InnerBoundaryConstants::_IGNORE_;
	if (is_supplied(_CEXT_)) from = local_fct_index(_CEXT_); else from = InnerBoundaryConstants::_IGNORE_;

	return std::pair<size_t, size_t>(from, to);
}


template<typename TDomain>
const std::string VDCC_BG<TDomain>::name() const
{
	return std::string("VDCC_BG");
}


template<typename TDomain>
void VDCC_BG<TDomain>::check_supplied_functions() const
{
	// Check that not both, inner and outer calcium concentrations are not supplied;
	// in that case, calculation of a flux would be of no consequence.
	if (!is_supplied(_CCYT_) && !is_supplied(_CEXT_))
	{
		UG_THROW("Supplying neither cytosolic nor extracellular calcium concentrations is not allowed.\n"
				"This would mean that the flux calculation would be of no consequence\n"
				"and this channel would not do anything.");
	}
	if (!m_bUseGatingAttachments)
	{
		UG_COND_THROW(!is_supplied(_M_), "Function for gating variable m must be provided.");
		UG_COND_THROW(has_hGate() && !is_supplied(_H_), "Function for gating variable h must be provided.");
	}
}


template<typename TDomain>
void VDCC_BG<TDomain>::print_units() const
{
	std::string nm = name();
	size_t n = nm.size();
	UG_LOG(std::endl);
	UG_LOG("+------------------------------------------------------------------------------+"<< std::endl);
	UG_LOG("|  Units used in the implementation of " << nm << std::string(n>=40?0:40-n, ' ') << "|" << std::endl);
	UG_LOG("|------------------------------------------------------------------------------|"<< std::endl);
	UG_LOG("|    Input                                                                     |"<< std::endl);
	UG_LOG("|      [Ca_cyt]  mM (= mol/m^3)                                                |"<< std::endl);
	UG_LOG("|      [Ca_ext]  mM (= mol/m^3)                                                |"<< std::endl);
	UG_LOG("|      V_m       V                                                             |"<< std::endl);
	UG_LOG("|                                                                              |"<< std::endl);
	UG_LOG("|    Output                                                                    |"<< std::endl);
	UG_LOG("|      Ca flux   mol/s                                                         |"<< std::endl);
	UG_LOG("+------------------------------------------------------------------------------+"<< std::endl);
	UG_LOG(std::endl);
}


template<typename TDomain>
void VDCC_BG<TDomain>::set_permeability(const number perm)
{
	m_perm = perm;
}


template<typename TDomain>
void VDCC_BG<TDomain>::init(number time)
{
	m_time = time;
	m_initTime = time;

	typedef typename DoFDistribution::traits<vm_grid_object>::const_iterator itType;
	SubsetGroup ssGrp;
	try { ssGrp = SubsetGroup(m_dom->subset_handler(), this->m_vSubset);}
	UG_CATCH_THROW("Subset group creation failed.");

	for (std::size_t si = 0; si < ssGrp.size(); si++)
	{
		itType iterBegin = m_dd->template begin<vm_grid_object>(ssGrp[si]);
		itType iterEnd = m_dd->template end<vm_grid_object>(ssGrp[si]);
		for (itType iter = iterBegin; iter != iterEnd; ++iter)
		{
			update_potential(*iter);

			if (m_bUseGatingAttachments)
			{
				// calculate corresponding start condition for gates
				const number vm = m_aaVm[*iter];
				m_aaMGate[*iter] = calc_gating_start(m_gpMGate, 1e3*vm);
				if (has_hGate())
					m_aaHGate[*iter] = calc_gating_start(m_gpHGate, 1e3*vm);
			}
		}
	}

	this->m_initiated = true;
}


template<typename TDomain>
void VDCC_BG<TDomain>::update_gating(vm_grid_object* elem)
{
	if (!this->m_initiated)
		UG_THROW("Borg-Graham not initialized.\n"
			<< "Do not forget to do so before any updates by calling init(initTime).");

	// set new gating particle values
	number dt = 1e3*(m_time - m_oldTime);   // calculating in ms
	calc_gating_step(m_gpMGate, 1e3*m_aaVm[elem], dt, m_aaMGate[elem]);
	if (has_hGate())
		calc_gating_step(m_gpHGate, 1e3*m_aaVm[elem], dt, m_aaHGate[elem]);
}


template<typename TDomain>
void VDCC_BG<TDomain>::update_time(const number newTime)
{
	if (newTime != m_time)
	{
		m_oldTime = m_time;
		m_time = newTime;
	}
};


template<typename TDomain>
void VDCC_BG<TDomain>::export_membrane_potential_to_vtk
(const std::string& fileName, size_t step, number time)
{
	// when called for the first time, initiate vtk output object and transfer grid function
	if (!m_spVtkOutput.valid())
	{
		m_spVtkOutput = make_sp(new VTKOutput<TDomain::dim>());

		std::string subsetsString = "";
		const size_t nSubset = this->m_vSubset.size();
		for (size_t i = 0; i < nSubset; ++i)
			subsetsString += this->m_vSubset[i] + ((i < nSubset - 1) ? ", " : "");

		SmartPtr<ApproximationSpace<TDomain> > spApprox =
			make_sp(new ApproximationSpace<TDomain>(m_dom));
		spApprox->add("Vm", "Lagrange", 1, subsetsString.c_str());

		m_spVmGF = make_sp(new GridFunction<TDomain, CPUAlgebra>(spApprox));
		m_spVmGF->set(0.0);
	}

	// copy attachment values to grid function
	typedef typename DoFDistribution::traits<vm_grid_object>::const_iterator it_type;
	std::vector<DoFIndex> dofInd;

	SmartPtr<DoFDistribution> spdd = m_spVmGF->dof_distribution();

	SubsetGroup ssGrp;
	try {ssGrp = SubsetGroup(m_dom->subset_handler(), this->m_vSubset);}
	UG_CATCH_THROW("Subset group creation failed.");
	const size_t nSs = ssGrp.size();
	for (size_t si = 0; si < nSs; ++si)
	{
		// loop sides and update potential (and gatings, if needed)
		it_type it = spdd->begin<vm_grid_object>(ssGrp[si]);
		it_type it_end = spdd->end<vm_grid_object>(ssGrp[si]);
		for (; it != it_end; ++it)
		{
			spdd->inner_dof_indices(*it, 0, dofInd, true);
			UG_ASSERT(dofInd.size() == 1, "Unexpected number of DoF indices.");
			DoFRef(*m_spVmGF, dofInd[0]) = m_aaVm[*it];
		}
	}

	// write to vtk
	try
	{
		m_spVtkOutput->print(fileName.c_str(), *m_spVmGF, step, time);
		m_spVtkOutput->write_time_pvd(fileName.c_str(), *m_spVmGF);
	}
	UG_CATCH_THROW("VTK output file prefixed '" << fileName << "' could not be written to.");
}



template<typename TDomain>
void VDCC_BG<TDomain>::prepare_timestep
(
    number future_time, const number time, VectorProxyBase* upb
)
{
    // initiate if this has not already been done (or init again; stationary case)
	if (!m_initiated || future_time == m_initTime)
		init(time);

	update_time(future_time);
	const bool backwardsStep = m_time < m_oldTime;

    // TODO: Think about updating only on the base level and then propagating upwards.
    //       Typically, the potential does not need very fine resolution.
    //       This would save a lot of work for very fine surface levels.
	typedef typename DoFDistribution::traits<vm_grid_object>::const_iterator it_type;
	SubsetGroup ssGrp;
	try { ssGrp = SubsetGroup(m_dom->subset_handler(), this->m_vSubset);}
	UG_CATCH_THROW("Subset group creation failed.");
	const size_t nSs = ssGrp.size();
	for (size_t si = 0; si < nSs; ++si)
	{
		// loop sides and update potential (and gatings, if needed)
		it_type it = m_dd->begin<vm_grid_object>(ssGrp[si]);
		it_type it_end = m_dd->end<vm_grid_object>(ssGrp[si]);
		if (m_bUseGatingAttachments)
		{
			// in case of a backwards step, we have to first rewind the gating
			// with the latest potential, then the potential itself,
			// otherwise, we can update the potential and then the gatings
			//
			if (backwardsStep)
			{
				for (; it != it_end; ++it)
				{
					update_gating(*it);
					update_potential(*it);
				}
			}
			else
			{
				for (; it != it_end; ++it)
				{
					update_potential(*it);
					update_gating(*it);
				}
			}
		}
		else
		{
			// we simply update all potentials
			for (; it != it_end; ++it)
				update_potential(*it);
		}
	}
}







template<typename TDomain>
void VDCC_BG<TDomain>::prepare_setting(const std::vector<LFEID>& vLfeID, bool bNonRegularGrid)
{
	// check that we are not using attachment mode
	UG_COND_THROW(m_bUseGatingAttachments,
		"The VDCC_BG class is using attachments for the gating variables.\n"
		"It can therefore not be used as an element discretization for their update.\n"
		"Either do not add it to the domain discretization or provide the necessary\n"
		"gating variable functions in the constructor.");

	// check that Lagrange 1st order
	for (size_t i = 0; i < vLfeID.size(); ++i)
		if (vLfeID[i].type() != LFEID::LAGRANGE || vLfeID[i].order() != 1)
			UG_THROW("VDCC_BG: 1st order Lagrange functions expected.");

	// update assemble functions
	m_bNonRegularGrid = bNonRegularGrid;
	register_all_fv1_funcs();
}


template<typename TDomain>
bool VDCC_BG<TDomain>::use_hanging() const
{
	return true;
}


#if 0
template<typename TDomain>
void VDCC_BG<TDomain>::approximation_space_changed()
{}
#endif


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::prep_elem_loop(const ReferenceObjectID roid, const int si)
{}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::fsh_elem_loop()
{}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::prep_elem
(
	const LocalVector& u,
	GridObject* elem,
	const ReferenceObjectID roid,
	const MathVector<dim> vCornerCoords[]
)
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
	UG_CATCH_THROW("VDCC_BG::prep_elem: Cannot update finite volume geometry.");
}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::add_def_A_elem
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

	const number vm = average_attachment_value_on_grid_object(m_aaVm, elem);

	// strictly speaking, we only need ODE assemblings here,
	// but it does not hurt to integrate over the boxes either
	for (size_t i = 0; i < fvgeom.num_bf(); ++i)
	{
		// get current BF
		const typename TFVGeom::BF& bf = fvgeom.bf(i);

		// get associated node
		const int co = bf.node_id();

		const number m_inf = calc_gating_start(m_gpMGate, 1e3*vm);
		d(_M_ - m_localIndicesOffset, co) -= (m_inf - u(_M_ - m_localIndicesOffset, co)) / m_gpMGate.tau_0*1e3 * bf.volume();

		if (has_hGate())
		{
			const number h_inf = calc_gating_start(m_gpHGate, 1e3*vm);
			d(_H_ - m_localIndicesOffset, co) -= (h_inf - u(_H_ - m_localIndicesOffset, co)) / m_gpHGate.tau_0*1e3 * bf.volume();
		}
	}
}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::add_def_M_elem
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

		d(_M_ - m_localIndicesOffset, co) += u(_M_ - m_localIndicesOffset, co) * bf.volume();
		if (has_hGate())
			d(_H_ - m_localIndicesOffset, co) += u(_H_ - m_localIndicesOffset, co) * bf.volume();
	}
}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::add_rhs_elem
(
	LocalVector& rhs,
	GridObject* elem,
	const MathVector<dim> vCornerCoords[]
)
{}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::add_jac_A_elem
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

		J(_M_ - m_localIndicesOffset, co, _M_ - m_localIndicesOffset, co) += bf.volume() / m_gpMGate.tau_0*1e3;
		if (has_hGate())
			J(_H_ - m_localIndicesOffset, co, _H_ - m_localIndicesOffset, co) += bf.volume() / m_gpHGate.tau_0*1e3;
	}
}


template<typename TDomain>
template<typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::add_jac_M_elem
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

		J(_M_ - m_localIndicesOffset, co, _M_ - m_localIndicesOffset, co) += bf.volume();
		if (has_hGate())
			J(_H_ - m_localIndicesOffset, co, _H_ - m_localIndicesOffset, co) += bf.volume();
	}

}


template<typename TDomain>
void VDCC_BG<TDomain>::register_all_fv1_funcs()
{
	//	get all grid element types in the dimension below
		typedef typename domain_traits<dim>::ManifoldElemList ElemList;

	//	switch assemble functions
		boost::mpl::for_each<ElemList>(RegisterFV1(this));
}

template<typename TDomain>
template <typename TElem, typename TFVGeom>
void VDCC_BG<TDomain>::register_fv1_func()
{
	ReferenceObjectID id = geometry_traits<TElem>::REFERENCE_OBJECT_ID;
	typedef VDCC_BG<TDomain> T;

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
	template class VDCC_BG<Domain1d>;
#endif
#ifdef UG_DIM_2
	template class VDCC_BG<Domain2d>;
#endif
#ifdef UG_DIM_3
	template class VDCC_BG<Domain3d>;
#endif

} // neuro_collection
} // namespace ug
