function(tx_add_release_ops in_target)
	set_property(TARGET ${in_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>)

	# MSVC
	target_compile_options(${in_target} PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
			/Ox;/Ob2;/Ot;/arch:SSE2;/fp:fast;/GL
		>
	)
	target_link_options(${in_target} PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
			/LTCG
		>
	)

	# GCC / Clang
    target_compile_options(${in_target} PRIVATE
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,$<CONFIG:Release>>:
            -O3;-ffast-math;-msse2;-flto
        >
    )
    target_link_options(${in_target} PRIVATE
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,$<CONFIG:Release>>:
            -flto
        >
    )
endfunction()

function(tx_add_release_ops_interface in_target)
	# MSVC
	target_compile_options(${in_target} INTERFACE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
			/Ox;/Ob2;/Ot;/arch:SSE2;/fp:fast;/GL
		>
	)
	target_link_options(${in_target} INTERFACE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
			/LTCG
		>
	)

	# GCC / Clang
    target_compile_options(${in_target} INTERFACE
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,$<CONFIG:Release>>:
            -O3;-ffast-math;-msse2;-flto
        >
    )
    target_link_options(${in_target} INTERFACE
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,$<CONFIG:Release>>:
            -flto
        >
    )
endfunction()