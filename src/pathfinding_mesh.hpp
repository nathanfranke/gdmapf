#pragma once

#include <map>
#include <vector>

#include <godot_cpp/classes/resource.hpp>

#include "model/pathfinding_cell.hpp"
#include "model/pathfinding_index.hpp"

class PathfindingMesh : public Resource
{
	GDCLASS(PathfindingMesh, Resource);

	real_t cell_size = 1.0;
	real_t elevation = 0.0;
	real_t max_climb = 0.5;
	std::vector<PathfindingCell> cells;
	std::map<Vector3i, PathfindingIndex> voxels;

protected:
	static void _bind_methods();

public:
	void set_cell_size(const real_t &p_cell_size);
	real_t get_cell_size() const;

	void set_elevation(const real_t &p_elevation);
	real_t get_elevation() const;

	void set_max_climb(const real_t &p_max_climb);
	real_t get_max_climb() const;

	void set_cells(const std::vector<PathfindingCell> p_cells);
	std::vector<PathfindingCell> &get_cells();

	void set_cells_bundled(const Array &p_cells_bundled);
	Array get_cells_bundled() const;

	size_t get_cell_count() const;
	PathfindingCell &get_cell(const PathfindingIndex &p_index);
	bool find_cell(const Vector3i &p_voxel, PathfindingIndex &r_index) const;
};
