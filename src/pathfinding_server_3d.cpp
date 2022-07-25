#include "pathfinding_server_3d.hpp"

#include <queue>
#include <algorithm>

PathfindingServer3D *PathfindingServer3D::singleton = nullptr;

void PathfindingServer3D::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("region_create"), &PathfindingServer3D::region_create);
	ClassDB::bind_method(D_METHOD("region_set_id", "region", "id"), &PathfindingServer3D::region_set_id);
	ClassDB::bind_method(D_METHOD("region_set_transform", "region", "transform"), &PathfindingServer3D::region_set_transform);
	ClassDB::bind_method(D_METHOD("region_set_mesh", "region", "mesh"), &PathfindingServer3D::region_set_mesh);
	ClassDB::bind_method(D_METHOD("region_free", "region"), &PathfindingServer3D::region_free);

	ClassDB::bind_method(D_METHOD("agent_create"), &PathfindingServer3D::agent_create);
	ClassDB::bind_method(D_METHOD("agent_set_region", "agent", "region"), &PathfindingServer3D::agent_set_region);
	ClassDB::bind_method(D_METHOD("agent_set_position", "agent", "position"), &PathfindingServer3D::agent_set_position);
	ClassDB::bind_method(D_METHOD("agent_navigate", "agent", "goal"), &PathfindingServer3D::agent_navigate);
	ClassDB::bind_method(D_METHOD("agent_get_next_position", "agent"), &PathfindingServer3D::agent_get_next_position);
	ClassDB::bind_method(D_METHOD("agent_free", "agent"), &PathfindingServer3D::agent_free);

	ClassDB::bind_method(D_METHOD("process", "delta"), &PathfindingServer3D::process);
}

PathfindingServer3D *PathfindingServer3D::get_singleton()
{
	return singleton;
}

std::map<Vector3i, PathfindingIndex> PathfindingServer3D::generate_voxels(const std::vector<PathfindingCell> &p_cells) const
{
	std::map<Vector3i, PathfindingIndex> voxels;
	for (uint64_t i = 0; i < p_cells.size(); ++i)
	{
		voxels[p_cells[i].voxel] = i;
	}
	return voxels;
}

//////////
// REGIONS
//////////

RID PathfindingServer3D::region_create()
{
	return regions.make_rid();
}

RID PathfindingServer3D::find_region(const StringName &p_id) const
{
	RID_Owner<PathfindingRegion> &regions = const_cast<RID_Owner<PathfindingRegion> &>(this->regions);

	const uint32_t rid_count = regions.get_rid_count();
	RID *const rids = (RID *)alloca(sizeof(RID) * rid_count);
	regions.fill_owned_buffer(rids);
	for (uint32_t i = 0; i < rid_count; i++)
	{
		const RID rid = rids[i];
		const PathfindingRegion *const region = regions.get_or_null(rid);
		if (region->id == p_id)
		{
			return rid;
		}
	}
	ERR_FAIL_V_MSG(RID(), String("Could not find region with name '%s'.").format(p_id));
}

void PathfindingServer3D::region_set_id(const RID &p_region, const StringName &p_id)
{
	PathfindingRegion *const region = regions.get_or_null(p_region);
	ERR_FAIL_NULL(region);

	region->id = p_id;
}

void PathfindingServer3D::region_set_mesh(const RID &p_region, const Ref<PathfindingMesh> &p_mesh)
{
	PathfindingRegion *const region = regions.get_or_null(p_region);
	ERR_FAIL_NULL(region);

	region->mesh = p_mesh;
}

void PathfindingServer3D::region_set_transform(const RID &p_region, const Transform3D &p_transform)
{
	PathfindingRegion *const region = regions.get_or_null(p_region);
	ERR_FAIL_NULL(region);

	region->transform = p_transform;
}

void PathfindingServer3D::region_free(const RID &p_region)
{
	regions.free(p_region);
}

//////////
// AGENTS
//////////

RID PathfindingServer3D::agent_create()
{
	return agents.make_rid();
}

void PathfindingServer3D::agent_set_region(const RID &p_agent, const StringName &p_region)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL(agent);

	agent->region = p_region;
}

void PathfindingServer3D::agent_set_position(const RID &p_agent, const Vector3 &p_position)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL(agent);

	agent->position = p_position;
	if (!agent->is_navigating)
	{
		agent->next_position = p_position;
	}
}

void PathfindingServer3D::agent_set_radius(const RID &p_agent, const real_t &p_radius)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL(agent);

	agent->radius = p_radius;
}

void PathfindingServer3D::agent_navigate(const RID &p_agent, const Vector3 &p_goal)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL(agent);

	agent->is_navigating = true;
	agent->goal = p_goal;

	// Causes the agent to stop the current movement.
	// Needed to initialize next_position to a non-zero vector.
	agent->next_position = agent->position;
}

void PathfindingServer3D::agent_stop(const RID &p_agent)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL(agent);

	agent->is_navigating = false;
}

bool PathfindingServer3D::agent_is_navigating(const RID &p_agent)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL_V(agent, false);

	return agent->is_navigating;
}

Vector3 PathfindingServer3D::agent_get_next_position(const RID &p_agent)
{
	PathfindingAgent *const agent = agents.get_or_null(p_agent);
	ERR_FAIL_NULL_V(agent, Vector3());

	return agent->next_position;
}

void PathfindingServer3D::agent_free(const RID &p_agent)
{
	agents.free(p_agent);
}

//////////

void PathfindingServer3D::process(const real_t &p_delta)
{
	MutexLock lock(mutex);

	//////////

	// Keep a collection of heap-allocated arrays of some length.
	// These are re-used essentially every time the server processes.

	static std::map<size_t, std::vector<real_t *>> array_cache;

	std::map<size_t, std::vector<real_t *>> array_usage;

	const auto get_array = [&array_usage](const size_t p_size) -> real_t *
	{
		std::vector<real_t *> list = array_usage[p_size];
		real_t *array;
		if (true || list.empty())
		{
			array = (real_t *)malloc(sizeof(real_t) * p_size);
		}
		else
		{
			array = list.back();
			list.pop_back();
		}
		array_usage[p_size].push_back(array);
		return array;
	};

	const auto flush_arrays = [&array_usage]() -> void
	{
		for (const std::pair<const size_t, std::vector<real_t *>> &pair : array_usage)
		{
			const size_t size = pair.first;
			std::vector<real_t *> list = array_cache[size];
			for (real_t *const array : pair.second)
			{
				list.push_back(array);
			}
		}
	};

	//////////

	// Pathfinding-specific constructs.

	static const real_t AGENT_STATIONARY_DENSITY = 50.0;
	static const real_t AGENT_MOVING_DENSITY = 1.5;
	static const int REQUIRED_EXTRA_ITERATIONS = 5; // Iterations to continue after required cells found.

	struct Grid
	{
		size_t size;
		real_t *cells = nullptr;
	};

	struct DensityGrid : public Grid
	{
	};

	struct PotentialGrid : public Grid
	{
		std::vector<PathfindingIndex> required_cells;
	};

	struct RegionSystem
	{
		PathfindingRegion *region;

		Transform3D transform;
		Transform3D transform_inv;
		real_t cell_size;

		DensityGrid density;
		std::map<PathfindingIndex, PotentialGrid> potentials;
	};

	struct AgentSystem
	{
		PathfindingAgent *agent;
		PathfindingRegion *region;

		PathfindingIndex origin_cell;

		DensityGrid *density;
		PotentialGrid *potential;
	};

	/**
	 * Map of Region RID->Region System.
	 * +-- Region System (Different system for each PathfindingRegion* node).
	 * |   +-- transform, transform_inv, cell_size
	 * |   +-- Density Grid (Occupation of each cell).
	 * |   |   +-- Size and contents.
	 * |   +-- Potential Grids (Maps Goal Cell->Potential Grid).
	 * |   |   +-- Potential Grid (Decreasing value as cells approach goal).
	 * |   |   |   +-- Size and contents.
	 * |   |   |   +-- List of required cells.
	 */
	std::map<RID, RegionSystem> region_systems;

	/**
	 * Map of Agent RID->Agent System.
	 * +-- Agent System
	 * |   +-- Origin Cell
	 * |   +-- Goal Cell
	 */
	std::map<RID, AgentSystem> agent_systems;

	//////////

	const auto occupy = [&](const RegionSystem &region_system, const Vector3 &p_position, const real_t &p_radius, const real_t &p_weight) -> void
	{
		const Transform3D transform_inv = region_system.transform_inv;
		const real_t cell_size = region_system.cell_size;

		const Vector3 offset(p_radius - 0.001, p_radius - 0.001, p_radius - 0.001);

		const Vector3i from = ((transform_inv.xform(p_position) - offset) / cell_size).floor();
		const Vector3i to = ((transform_inv.xform(p_position) + offset) / cell_size).floor();

		PathfindingIndex index;
		for (int x = from.x; x <= to.x; ++x)
		{
			for (int y = from.y; y <= to.y; ++y)
			{
				for (int z = from.z; z <= to.z; ++z)
				{
					const Vector3i voxel = Vector3i(x, y, z);
					const bool has_voxel = region_system.region->mesh->find_cell(voxel, index);
					if (has_voxel)
					{
						real_t &den = region_system.density.cells[index];
						den += p_weight;

#ifdef DEBUG_ENABLED
						PathfindingCell &cell = region_system.region->mesh->get_cell(index);
						cell.debug_density = den;
#endif
					}
				}
			}
		}
	};

	//////////

	// Step 1: Populate the region and agent systems.

	const uint32_t agent_id_count = agents.get_rid_count();
	RID *const agent_ids = (RID *)alloca(sizeof(RID) * agent_id_count);
	agents.fill_owned_buffer(agent_ids);
	for (uint32_t i = 0; i < agent_id_count; ++i)
	{
		const RID agent_id = agent_ids[i];
		PathfindingAgent *const agent = agents.get_or_null(agent_id);
		ERR_FAIL_NULL(agent);

		// The agent is not navigating.
		if (!agent->is_navigating)
		{
			continue;
		}

		// The agent is already moving towards a target.
		if (!agent->position.is_equal_approx(agent->next_position))
		{
			continue;
		}

		// Find the region this agent is navigating on.
		const RID region_id = find_region(agent->region);
		PathfindingRegion *const region = regions.get_or_null(region_id);
		ERR_FAIL_NULL(region);

		// Read data from region.
		const Transform3D region_transform = region->transform;
		const Transform3D region_transform_inv = region_transform.affine_inverse();
		const real_t cell_size = region->mesh->get_cell_size();

		// Helper function to get the cell index given target position.
		const auto find_cell = [&](const Vector3 &p_position, PathfindingIndex &r_cell) -> bool
		{
			static const Vector3i UP(0, 1, 0);

			const Vector3i voxel = (region_transform_inv.xform(p_position) / cell_size).floor();

			if (region->mesh->find_cell(voxel, r_cell))
			{
				return true;
			}
			if (region->mesh->find_cell(voxel + UP, r_cell))
			{
				return true;
			}
			if (region->mesh->find_cell(voxel - UP, r_cell))
			{
				return true;
			}
			return false; // Position is not inside the map.
		};

		// Find the current cell of the agent.
		PathfindingIndex origin_cell;
		if (!find_cell(agent->position, origin_cell))
		{
			// Cannot find origin cell.
			agent->is_navigating = false;

#ifdef DEBUG_ENABLED
			ERR_PRINT("Agent is not inside the pathfinding region.");
#endif
			continue;
		}

		// Find the goal cell of the agent.
		PathfindingIndex goal_cell;
		if (!find_cell(agent->goal, goal_cell))
		{
			// Cannot find goal cell.
			agent->is_navigating = false;

#ifdef DEBUG_ENABLED
			ERR_PRINT("Goal is not inside the pathfinding region.");
#endif
			continue;
		}

		// Populate region system.
		RegionSystem &region_system = region_systems[region_id];
		region_system.region = region;
		region_system.transform = region_transform;
		region_system.transform_inv = region_transform_inv;
		region_system.cell_size = cell_size;

		// Add agent cell to list of required cells.
		PotentialGrid &goal_potential = region_system.potentials[goal_cell];
		goal_potential.required_cells.push_back(origin_cell);

		// Populate agent system.
		AgentSystem &agent_system = agent_systems[agent_id];
		agent_system.agent = agent;
		agent_system.region = region;
		agent_system.origin_cell = origin_cell;
		agent_system.density = &region_system.density;
		agent_system.potential = &goal_potential;
	}

	// UtilityFunctions::print("region systems: ", region_systems.size(), ", agent systems: ", agent_systems.size());

	//////////

	// Step 2: Generate density and potential grids.

	for (std::pair<const RID, RegionSystem> &pair : region_systems)
	{
		// Get the region system.
		RegionSystem &region_system = pair.second;
		PathfindingRegion *const region = region_system.region;

		// Read data from region.
		const Transform3D region_transform = region->transform;
		const Transform3D region_transform_inv = region_transform.affine_inverse();
		const real_t cell_size = region->mesh->get_cell_size();
		const size_t cell_count = region->mesh->get_cell_count();

		//////////
		// DENSITY GRID
		//////////

		DensityGrid &density = region_system.density;
		density.cells = get_array(cell_count);
		std::fill(&density.cells[0], &density.cells[cell_count], 0.0);

#ifdef DEBUG_ENABLED
		for (PathfindingCell &cell : region->mesh->get_cells())
		{
			cell.debug_density = 0.0;
			cell.debug_potential = 0.0;
		}
#endif

		for (uint32_t i = 0; i < agent_id_count; i++)
		{
			const RID r = agent_ids[i];
			PathfindingAgent *const a = agents.get_or_null(r);
			ERR_FAIL_NULL(a);

			if (a->is_navigating)
			{
				occupy(region_system, a->position, a->radius, AGENT_MOVING_DENSITY);
				occupy(region_system, a->next_position, a->radius, AGENT_MOVING_DENSITY);
			}
			else
			{
				occupy(region_system, a->position, a->radius, AGENT_STATIONARY_DENSITY);
			}
		}

		for (std::pair<const PathfindingIndex, PotentialGrid> &pair : region_system.potentials)
		{
			PathfindingIndex goal_cell = pair.first;
			PotentialGrid &potential = pair.second;

			//////////
			// POTENTIAL GRID
			//////////

			potential.cells = get_array(cell_count);
			std::fill(&potential.cells[0], &potential.cells[cell_count], INFINITY);

			potential.cells[goal_cell] = 0.0;

			std::vector<PathfindingIndex> required = potential.required_cells;

			int required_extra = REQUIRED_EXTRA_ITERATIONS;

			std::vector<PathfindingIndex> active;
			active.push_back(goal_cell);
			while (!active.empty() && (!required.empty() || --required_extra >= 0))
			{
				std::vector<PathfindingIndex> next_active;

				for (const PathfindingIndex index : active)
				{
					required.erase(std::remove(required.begin(), required.end(), index), required.end());

					PathfindingCell &cell = region->mesh->get_cell(index);
					const real_t current_pot = potential.cells[index];

					for (const std::pair<const PathfindingIndex, PathfindingConnection> &entry : cell.connections)
					{
						real_t &next_pot = potential.cells[entry.first];
						const real_t new_pot = current_pot + entry.second.weight + density.cells[entry.first];
						if (new_pot < next_pot)
						{
							next_pot = new_pot;
							next_active.push_back(entry.first);

#ifdef DEBUG_ENABLED
							region->mesh->get_cell(entry.first).debug_potential = next_pot;
#endif
						}
					}
				}

				active = next_active;
			}
		}
	}

	//////////

	// Step 3: Navigate agents along the potential grid.

	for (std::pair<const RID, AgentSystem> &pair : agent_systems)
	{
		// Get the agent structure.
		const RID agent_id = pair.first;
		PathfindingAgent *const agent = agents.get_or_null(agent_id);
		ERR_FAIL_NULL(agent);

		// Get the agent system.
		const AgentSystem &agent_system = pair.second;
		PathfindingRegion *const region = agent_system.region;

		const PathfindingIndex &origin_cell = agent_system.origin_cell;

		DensityGrid *const density = agent_system.density;
		PotentialGrid *const potential = agent_system.potential;

		//////////
		// NAVIGATION
		//////////

		// Subtract current cell density to not bias other cells.
		const real_t current_pot = potential->cells[origin_cell] - density->cells[origin_cell] + 0.1;
		real_t best_pot = current_pot;
		PathfindingIndex best_cell = origin_cell;

		if (unlikely(best_pot <= CMP_EPSILON))
		{
			// Reached destination.
			agent->is_navigating = false;
		}

		// Could be possible if optimization is too aggressive.
		if (UtilityFunctions::is_inf(best_pot))
		{
			agent->is_navigating = false;
		}

		for (const std::pair<const PathfindingIndex, PathfindingConnection> &entry : region->mesh->get_cell(origin_cell).connections)
		{
			const PathfindingIndex &cell = entry.first;
			const real_t pot = potential->cells[cell];

			if (unlikely(density->cells[cell] > CMP_EPSILON))
			{
				goto density_invalid;
			}
			for (const PathfindingIndex obstacle : entry.second.obstacles)
			{
				if (unlikely(density->cells[obstacle] > CMP_EPSILON))
				{
					goto density_invalid;
				}
			}
			goto density_valid;
		density_invalid:
			continue;
		density_valid:
			if (pot < best_pot + CMP_EPSILON)
			{
				best_pot = pot;
				best_cell = cell;
			}
		}

		if (best_cell == origin_cell)
		{
			for (const std::pair<const PathfindingIndex, PathfindingConnection> &entry : region->mesh->get_cell(origin_cell).connections)
			{
				const PathfindingIndex &cell = entry.first;
				const real_t pot = potential->cells[cell];

				// Near destination if:
				// - Best cell is the current cell (previous statement).
				// - Adjacent density is a stationary agent.
				// - Adjacent potential is less than current potential.
				if (density->cells[cell] >= AGENT_STATIONARY_DENSITY - CMP_EPSILON && pot < current_pot + CMP_EPSILON)
				{
					agent->is_navigating = false;
				}
			}
		}

		// print_line(String() + "Next cell: " + String(Variant(best_cell)) + " at " + region->mesh->get_cell(best_cell).voxel + " with density " + String(Variant(density.cells[best_cell])));

		agent->next_position = region->transform.xform(region->mesh->get_cell(best_cell).position);

		for (std::pair<const RID, RegionSystem> &pair : region_systems)
		{
			occupy(pair.second, agent->next_position, agent->radius, AGENT_MOVING_DENSITY);
		}
	}

	// Returns checked-out arrays back into the pool.
	flush_arrays();
}

//////////

PathfindingServer3D::PathfindingServer3D()
{
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

PathfindingServer3D::~PathfindingServer3D()
{
	singleton = nullptr;
}
