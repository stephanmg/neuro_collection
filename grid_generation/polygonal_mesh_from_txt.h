#ifndef UG__PLUGINS__NEURO_COLLECTION__GRID_GENERATION__POLYGONAL_MESH_FROM_TXT_H
#define UG__PLUGINS__NEURO_COLLECTION__GRID_GENERATION__POLYGONAL_MESH_FROM_TXT_H

#include <string>

namespace ug {
	namespace neuro_collection {
		/*!
		 * \brief Creates a 2d polygonal mesh from specified filename
		 * \param[in] fileName input file containing 2d coordinates
		 *
		 * Note, the data in the input file is assumed to contain one
		 * 2d coordinate per line separated by white space and the
		 * connectivity is given implicitly by assuming consecutive
		 * coordinates are connected by an edge and the last and first
	   	 * coordinates are also connected by an additional edge
		 */
		void polygonal_mesh_from_txt(const std::string& fileName);
	} // neuro_collection
} // ug

#endif // UG__PLUGINS__NEURO_COLLECTION__GRID_GENERATION__POLYGONAL_MESH_FROM_TXT_H
