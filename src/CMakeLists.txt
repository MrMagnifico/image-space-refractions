target_sources(RefractionsLib
	PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/ray_tracing/bounding_volume_hierarchy.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ray_tracing/interpolate.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ray_tracing/intersect.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/render/environment_map.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/mesh_manager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/refraction.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/texture.cpp"
        
        "${CMAKE_CURRENT_LIST_DIR}/ui/menu.cpp"
        
        "${CMAKE_CURRENT_LIST_DIR}/utils/numerical_utils.cpp")
