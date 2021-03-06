/*
 * Copyright (c) 2009-2019: G-CSC, Goethe University Frankfurt
 *
 * Author: Markus Breit
 * Creation date: 2016-12-27
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

#ifndef UG__PLUGINS__NEURO_COLLECTION__TEST__TEST_NEURITE_PROJ_H
#define UG__PLUGINS__NEURO_COLLECTION__TEST__TEST_NEURITE_PROJ_H

#include "common/math/ugmath_types.h"
#include "common/util/smart_pointer.h"
#include "lib_grid/multi_grid.h"
#include "lib_grid/refinement/projectors/neurite_projector.h"
#include "lib_grid/refinement/projectors/cylinder_projector.h" // CylinderProjector
#include <vector>
#include <string>
#include <list>
#include <utility>
#include "types.h"

namespace ug {
namespace neuro_collection {

/*!
 * \brief import a SWC file
 * \param[in] fileName
 * \param[out] vPointsOut
 * \param[in] correct
 * \param[in] scale
 */
void import_swc
(
    const std::string& fileName,
    std::vector<SWCPoint>& vPointsOut,
    bool correct,
    number scale
);

/*!
 * \brief import a SWC file
 * \param[in] fileName
 * \param[out] vPointsOut
 * \param[in] scale
 */
void import_swc
(
	const std::string& fileName,
	std::vector<SWCPoint>& vPointsOut,
	number scale
);


/*!
 * \brief smoothing
 * \param[inout] vPointsInOut
 * \param[in] n
 * \param[in] h
 * \param[in] gamma
 * Note:s Needs debugging.
 */
void smoothing
(
	std::vector<SWCPoint>& vPointsInOut,
	size_t n,
	number h,
	number gamma
);

/*!
 * \brief collapse short edges
 * \param[in] g
 * \param[in] sh
 * Note: Needs debugging
 */
void collapse_short_edges
(
	Grid& g,
	SubsetHandler& sh
);

/*!
 * \brief converts pointlist to a neuritelist
 * \param[in] vPoints
 * \param[in] vPosOut
 * \param[in] vRadOut
 * \param[in] vBPInfoOut
 * \param[in] vRootNeuriteIndsOut
 */
void convert_pointlist_to_neuritelist
(
    const std::vector<SWCPoint>& vPoints,
    std::vector<SWCPoint>& vSomaPoints,
    std::vector<std::vector<vector3> >& vPosOut,
    std::vector<std::vector<number> >& vRadOut,
    std::vector<std::vector<std::pair<size_t, std::vector<size_t> > > >& vBPInfoOut,
    std::vector<size_t>& vRootNeuriteIndsOut
);

void convert_pointlist_to_neuritelist_variant
(
    const std::vector<SWCPoint>& vPoints,
    std::vector<SWCPoint>& vSomaPoints,
    std::vector<std::vector<vector3> >& vPosOut,
    std::vector<std::vector<number> >& vRadOut,
    std::vector<std::vector<std::pair<size_t, std::vector<size_t> > > >& vBPInfoOut,
    std::vector<size_t>& vRootNeuriteIndsOut
);

void create_spline_data_for_neurites
(
    std::vector<NeuriteProjector::Neurite>& vNeuritesOut,
    const std::vector<std::vector<vector3> >& vPos,
    const std::vector<std::vector<number> >& vR,
    std::vector<std::vector<std::pair<size_t, std::vector<size_t> > > >* vBPInfo
);

/*!
 * \brief test smoothing
 * \param[in] fileName
 * \param[in] n
 * \param[in] h
 * \param[in] gamma
 */
void test_smoothing
(
	const std::string& fileName,
	size_t n,
	number h,
	number gamma
);

/*!
 * \brief test import swc
 * \param[in] fileName
 * \param[in] anisotropy
 * \param[in] numRefs
 */
void test_import_swc
(
	const std::string& fileName,
	number anisotropy = 2.0,
	size_t numRefs = 0
);

/*!
 * \brief test import swc with er
 * \param[in] fileName
 * \param[in] correct
 * \param[in] erScaleFactor
 * \param[in] anisotropy
 * \param[in] numRefs
 */
void test_import_swc_with_er
(
	const std::string& fileNameIn,
	const std::string& fileNameOut,
	number erScaleFactor,
	number anisotropy = 2.0,
	size_t numRefs = 0
);

/*!
 * \brief test import swc surf
 * \param[in] fileName
 */
void test_import_swc_surf
(
	const std::string& fileName
);

/*!
 * \brief test import swc 1d
 * \param[in] fileName
 * \param[in] anisotropy
 * \param[in] numRefs
 * \param[in] scale
 */
void test_import_swc_1d
(
	const std::string& fileName,
	number anisotropy = 2.0,
	size_t numRefs = 0,
	number scale = 1e-6
);

/*!
 * \brief shrink quadrilateral copy
 * \param[in] vVrt
 * \param[out] outvVrt
 * \param[in] oldVertices
 * \param[out] outvEdge
 * \param[in] g
 * \param[in] aaPos
 * \param[in] percentage
 * \param[in] createFaces
 * \param[in] outSel
 * \param[in] currentDir
 */
void shrink_quadrilateral_copy
(
	const std::vector<Vertex*>& vVrt,
	std::vector<Vertex*>& outvVrt,
	const std::vector<Vertex*>& oldVertices,
	std::vector<Edge*>& outvEdge,
	Grid& g,
	Grid::VertexAttachmentAccessor<APosition>& aaPos,
	number percentage,
	bool createFaces,
	ISelector* outSel,
	ug::vector3* currentDir
);

/*!
 * \brief test smoothing
 * \param[in] fileName
 * \param[in] n
 * \param[in] h
 * \param[in] gamma
 * \param[in] scale
 */
void test_smoothing
(
	const std::string& fileName,
	size_t n,
	number h,
	number gamma,
	number scale
);

/*!
 * \brief test import swc
 * \param[in] fileName
 * \param[in] correct
 */
void test_import_swc
(
	const std::string& fileName,
	bool correct
);

/*!
 * \brief test import swc scale
 * \param[in] fileName
 * \param[in] correct
 * \param[in] scale
 */
void test_import_swc_scale
(
	const std::string& fileName,
	bool correct,
	number scale
);

/*!
 * \brief test import swc general
 * Builds geometry with or without ER and connects to soma
 * \param[in] fileName
 * \param[in] correct
 * \param[in] erScaleFactor
 * \param[in] withER
 * \param[in] anisotropy
 * \param[in] numRefs
 */
void test_import_swc_general
(
	const std::string& fileName,
	bool correct,
	number erScaleFactor,
	bool withER,
	number anisotropy = 2.0,
	size_t numRefs = 1,
	bool regularize = false,
	const std::string& option = "identity",
	number segLength=-1
);

/*!
 * \brief test import swc general smooth
 * \param[in] fileName
 * \param[in] correct
 * \param[in] shrinkPercentage
 * \param[in] withER
 */
void test_import_swc_general_smooth
(
	const std::string& fileName,
	bool correct,
	number shrinkPercentage,
	bool withER
);

/*!
 * \brief test neurite projector with four section tube
 */
void test_neurite_projector_with_four_section_tube();

/*!
 * \brief test neurite projector with four section tube and branch point
 */
void test_neurite_projector_with_four_section_tube_and_branch_point();

/*!
 * \brief test shrink geometry
 * \param[in] percentage
 */
void test_shrink_geom
(
	number percentage
);

/*!
 * \brief test shrink geom copy
 * \param[in] percentage
 */
void test_shrink_geom_copy
(
	number percentage
);

/*!
 * \brief test split geom
 * \param[in] percentage
 */
void test_split_geom
(
	number percentage
);

/*!
 * \brief apply neurite projector
 * \param[inout] mg
 * \param[in] neuriteProj
 */
void apply_neurite_projector
(
	MultiGrid& mg,
	SmartPtr<NeuriteProjector> neuriteProj
);

/*!
 * \brief converts swc to ugx
 * \param[in] fileName
 */
void test_convert_swc_to_ugx
(
	const std::string& fileName
);

/*!
 * \brief new main strategy to generate computational meshs for 1d/3d simulations
 * TODO: refactor to eliminate not necessary VR grid generation parameters and options
 * => OptionBuilder?!
 * \param[in] fileName
 * \param[in] correct
 * \param[in] erScaleFactor
 * \param[in] withEr
 * \param[in] anisotropy
 * \param[in] numRefs
 * \param[in] regularize
 * \param[in] blowUpFactor
 * \param[in] forVR
 * \param[in] dryRun
 * \param[in] option
 */
void test_import_swc_general_var(
	const std::string& fileName,
	bool correct,
	number erScaleFactor,
	bool withER,
	number anisotropy,
	size_t numRefs,
	bool regularize,
	number blowUpFactor,
	bool forVR,
	bool dryRun,
	const std::string& option,
	number segLength
);

/*!
 * \brief for internal use only
 */
int test_import_swc_general_var_benchmark(
		const std::string& fileName,
		bool correct,
		number erScaleFactor,
		bool withER,
		number anisotropy,
		size_t numRefs,
		bool regularize,
		number blowUpFactor,
		bool forVR,
		bool dryRun,
		const std::string& option,
		number segLength
);

/*!
 * \brief for internal use only
 */
int test_import_swc_general_var_benchmark_var(
		const std::string& fileName,
		number erScaleFactor,
		size_t numRefs
);


/*!
 * \brief vr strategy (potentially deprecated!)
 */
void test_import_swc_general_var_for_vr
(
	const std::string& fileName,
	bool correct,
	number erScaleFactor,
	bool withER,
	number anisotropy,
	size_t numRefs,
	bool regularize,
	number blowUpFactor,
	number segLength
);

/*!
 * \brief refine swc grid
 * \param[in] inFileName
 * \param[in] outFileName
 */
void refine_swc_grid
(
	const std::string& inFileName,
	const std::string& outFileName
);

// Graph class represents a directed graph
// using adjacency list representation
class Graph
{
    int V;    // No. of vertices

    // Pointer to an array containing
    // adjacency lists
    std::list<std::pair<int, double> >*adj;

    // A recursive function used by DFS
    void DFSUtil(int v, bool visited[], std::vector<int>& indices);
public:
    Graph(int V);   // Constructor

    // function to add an edge to graph
    void addEdge(int v, int w);
    void addEdge(int u, int v, double w);

    // DFS traversal of the vertices
    // reachable from v
    void DFS(int v, std::vector<int>& indices);
    void BFS(int v, std::vector<int>& indices);
    std::vector<double> shortestPath(int s);
};



/*!
 * \brief naive refinement variant which will preserve HINES ordering
 * \param[in] fileName
 * \param[in] outName
 * \param[in] writeMatrix
 */
void refine_swc_grid_variant(
	const std::string& fileName,
	const std::string& outName,
	bool writeMatrix
);

/*!
 * \brief naive coarsening by split and collapse of edges to get average edge length
 * \param[in] fileName
 * \param[in] factor
 */
void coarsen_1d_grid(
	const std::string& fileName,
	number factor
);


/*!
 * \brief old, deprecated method potentially
 * \param[in] fileName
 * \param[in] anisotropy
 * \param[in] numRefs
 */
void test_import_swc_vr
(
	const std::string& fileName,
	number anisotropy,
	size_t numRefs
);

/*!
 * \brief main routine to generate grids for VR
 */
void test_import_swc_general_var_for_vr_var(
	const std::string& fileName,
	bool correct,
	number erScaleFactor,
	bool withER,
	number anisotropy,
	size_t numRefs,
	bool regularize,
	number blowUpFactor,
	const std::string& option,
	number segLength
);

int test_import_swc_general_var_for_vr_var_benchmark(
	const std::string& fileName,
	bool correct,
	number erScaleFactor,
	bool withER,
	number anisotropy,
	size_t numRefs,
	bool regularize,
	number blowUpFactor,
	const std::string& option,
	number segLength
);

////////////////////////////////////////////////////////////////////////
/// Helper methods to flip a pair and a map
////////////////////////////////////////////////////////////////////////
template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

template<typename A, typename B>
std::multimap<B,A> flip_map(const std::map<A,B> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}

/*!
 * \brief special method to generate two way branches
 * \param[in] fileName
 * \param[in] erScaleFactor
 * \param[in] numRefs
 * \param[in] assignMeasurementSubsets
 */
void create_branches_from_swc(
	const std::string& fileName,
	number erScaleFactor,
	size_t numRefs,
	bool assignMeasurementSubsets
);

/*!
 * \brief delegate
 * \see create_branches_from_swc
 */
void create_branches_from_swc(
	const std::string& fileName,
	number erScaleFactor,
	size_t numRefs
);

/*!
 * \brief converts SWCPoint vector to a grid
 */
void swc_points_to_grid
(
	const std::vector<SWCPoint>& vPts,
	Grid& grid,
	SubsetHandler& sh,
	number scale_length = 1.0
);

/*!
 * \brief special method to respect reordering during UGX export
 * \param[in] vPts
 * \param[in] sh
 * \param[in] mapping
 * \param[in] scale
 */
void swc_points_to_grid_var(
	const std::vector<SWCPoint>& vPts,
	Grid& g,
	SubsetHandler& sh,
	const std::map<int, int>& mapping,
	number scale_length = 1.0
);

/*!
 * \brief special method to save grid as UGX with preserved ordering
 * The grid is written as SWC, then read in as SWC, and then saved to UGX
 */
void to_ugx(
	Grid& grid,
	SubsetHandler& sh,
	const std::string& fileName
);

/*!
 * \brief regularizes the 1d geometry
 *
 * Options are: Use user specified segment length ("user") or automatically
 * determine minimum possible segment length ("auto"). If user prescribed
 * segment length is not compatible, the user will be warned and prompted
 * to provide either a new segment length, ignore the warning or use the
 * automatically calculated admissible segment length for mesh regularization
 *
 * \param[in] fileName name of the input geometry
 * \param[in] segLength desired segment length
 * \param[in] choice one of the two options above
 * \param[in] ref desired refinement level
 * \param[in] forceAdditionalPoint forces insertion of an additional point
 * \param[in] includeSoma include soma in regularization - or not
 * \param[in] maxInflation for angle length criterion
 */
void test_import_swc_and_regularize
(
	const std::string& fileName,
	number segLength,
	const std::string& choice,
	size_t ref,
	bool forceAdditionalPoint,
	bool includeSoma,
	number maxInflation
);

/*!
 * \brief see test_import_swc_and_regularize
 */
void test_import_swc_and_regularize
(
	const std::string& fileName,
	number segLength,
	const std::string& choice,
	size_t ref,
	bool forceAdditionalPoint,
	bool includeSoma
);

/*!
 * \brief regularizes the 1d geometry
 *
 * \param[in] fileName name of the input geometry
 * \see test_import_swc_and_regularize
 */
void test_import_swc_and_regularize
(
	const std::string& fileName
);

/*!
 * \brief regularizes the 1d geometry
 *
 * \param[in] fileName name of the input geometry
 * \param[in] forceAdditionalPoint forces insertion of an additional point
 * \param[in] includeSoma soma point will be included in regularization
 * \see test_import_swc_and_regularize
 */
void test_import_swc_and_regularize
(
	const std::string& fileName,
	bool forceAdditionalPoint,
	bool includeSoma
);

/*!
 * \brief import swc and regularize seg length by GQ's angle-length criterion
 * \param[in] fileName name of the input geometry
 * \param[in] inflation
 * \see test_import_swc_and_regularize
 *
 * Note that the segment length is chosen by the minimum admissible segment
 * length as determined by GQ's angle-length criterion at branching points
 */
void test_import_swc_and_regularize_var
(
	const std::string& fileName,
	number inflation
);

/*!
 * \brief check if all fragments are separated by minimum distance
 *
 * The methods returns true if all fragments agree with the minimum distance,
 * otherwise if at least once segment is below the required distance the method
 * returns false
 *
 * \param[in] vFraments fragments
 * \param[in] desiredSegLength desired segment length
 *
 * \return \c bool
 */
bool check_fragments
(
	const std::vector<std::vector<vector3> >& vFragments,
	number desiredSegLength
);

/*!
 * \brief sample spline equidistant in arclength
 *
 * \param[in] vNeurites spline data for each fragment of neurite
 * \param[in] desiredSegLength desired segment length (arclength!)
 * \param[in] ref desired refinement level
 * \param[in] forceAdditionalPoint forces insertion of an additional point
 */
void eval_spline
(
	const std::vector<NeuriteProjector::Neurite>& vNeurites,
	number desiredSegLength,
	size_t ref,
	bool forceAdditionalPoint,
	Grid& g,
	SubsetHandler& sh,
	bool somaIncluded,
	number postProcessLength
);

void eval_spline_var
(
	const std::vector<NeuriteProjector::Neurite>& vNeurites,
	const number desiredSegLength,
	const size_t ref,
	const bool forceAdditionalPoint,
	Grid& g,
	SubsetHandler& sh,
	const bool somaIncluded,
	const number postProcessLength
);

/*!
 * \brief calculates the minimum allowed segment length for fragments
 *
 * Calculates the minimum distance by this formular:
 * \f$min_{i,j} { (||S, BP_i||_2), (||BP_i, BP_j||_2), (||BP_i, T_j||_2 \forall i != j\f$
 * where BP_i is a branching point, S is the single soma and T_i are the neurite tips
 *
 * The method returns the minimum distance between fragments
 *
 * \param[in] vFragments fragments
 *
 * \return \c number
 */
number calculate_minimum_seg_length_between_fragments
(
	const std::vector<std::vector<vector3> >& vFragments
);

/*!
 * \brief finds and sets the permissible render vector for neurites
 * \param[in] fileName input 1d structure
 * \param[out] vNeurites neurites
 *
 */
void set_permissible_render_vector
(
	const std::vector<std::vector<ug::vector3> >& vPos,
	std::vector<NeuriteProjector::Neurite>& vNeurites
);

/*!
 * \brief finds and sets the permissible render vector for neurites globally
 * \param[in] fileName input 1d structure
 * \param[out] vNeurites neurites
 *
 */
void set_permissible_render_vector_global
(
	const std::vector<std::vector<ug::vector3> >& vPos,
	std::vector<NeuriteProjector::Neurite>& vNeurites
);


/*!
 * \brief finds the global minimum allowed segment length for branching points
 * Returns the minimum global allowed segment length
 * \param[in] fileName
 * \param[in] inflation
 *
 * \return \c number
 */
number find_min_bp_dist
(
	const std::string& fileName,
	number inflation=1.0
);

void create_piecewise_cylinder_projectors
(
	Grid& grid,
	std::vector<SmartPtr<CylinderProjector> >& vProjectors,
	Grid::VertexAttachmentAccessor<APosition>& aaPos
);

void write_piecewise_cylinder_projectors
(
	const std::string& fileName,
	Grid& grid,
	SubsetHandler& sh,
	const std::vector<SmartPtr<CylinderProjector> >& vProjectors
);

void refine_piecewise_cylindrical
(
	const std::string& fileName,
	const uint numRefs
);

int test_statistics
(
	const std::string& fileName,
	number erScaleFactor
);

int test_statistics_soma
(
	const std::string& fileName,
	number erScaleFactor
);


} // namespace neuro_collection
} // namespace ug

#endif // UG__PLUGINS__NEURO_COLLECTION__TEST__TEST_NEURITE_PROJ_H
