/*
 *	Discretization for the SERCA calcium pump in the ER membrane
 *
 *  Created on: 20.12.2011
 *      Author: mbreit
 */

#ifndef __UG__PLUGINS__EXPERIMENTAL__NEURO_COLLECTION__SERCA_H__
#define __UG__PLUGINS__EXPERIMENTAL__NEURO_COLLECTION__SERCA_H__


#include "membrane_transporter_interface.h"
#include "lib_disc/spatial_disc/elem_disc/inner_boundary/inner_boundary.h"


namespace ug{
namespace neuro_collection{
        
        
///@addtogroup plugin_neuro_collection
///@{


/// Discretization for the SERCA calcium pump in the ER membrane
/**
 * This class implements the InnerBoundaryElemDisc to provide flux densities
 * and their derivatives for the Sneyd et al. (2003) SERCA model.
 *
 * Units used in the implementation of this channel:
 * [Ca_cyt]  mM (= mol/m^3)
 * [Ca_er]   mM (= mol/m^3)
 *
 * Ca flux   mol/s
 */

class SERCA : public IMembraneTransporter
{
    public:
        enum{_CCYT_=0, _CER_};
    

    protected:
		const number VS;			// maxmimal transport power per pump (divided by [ca_ER])
		const number KS;			// concentration at which halfmaximal pumping occurs

	public:
		/// @copydoc IMembraneTransporter::IMembraneTransporter(const std::vector<std::string)
		SERCA(const std::vector<std::string>& fcts);

		/// @copydoc IMembraneTransporter::IMembraneTransporter()
		SERCA(const char* fcts);

		/// @copydoc IMembraneTransporter::IMembraneTransporter()
        virtual ~SERCA();
        
        /// @copydoc IMembraneTransporter::calc_flux()
        virtual void calc_flux(const std::vector<number>& u, GridObject* e, std::vector<number>& flux) const;
        
        /// @copydoc IMembraneTransporter::calc_flux_deriv()
        virtual void calc_flux_deriv(const std::vector<number>& u, GridObject* e, std::vector<std::vector<std::pair<size_t, number> > >& flux_derivs) const;
        
        /// @copydoc IMembraneTransporter::n_dependencies()
        virtual size_t n_dependencies() const;
        
        /// @copydoc IMembraneTransporter::n_fluxes()
        virtual size_t n_fluxes() const;
        
        /// @copydoc IMembraneTransporter::flux_from_to()
        virtual const std::pair<size_t,size_t> flux_from_to(size_t flux_i) const;
        
        /// @copydoc IMembraneTransporter::name()
        virtual const std::string name() const;
        
        /// @copydoc IMembraneTransporter::check_supplied_functions()
        virtual void check_supplied_functions() const;
        
        /// @copydoc IMembraneTransporter::print_units()
        virtual void print_units() const;
};
    
///@}


} // namespace neuro_collection
} // namespace ug

#endif // __UG__PLUGINS__EXPERIMENTAL__NEURO_COLLECTION__SERCA_H__

