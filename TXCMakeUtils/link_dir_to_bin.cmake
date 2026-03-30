function(tx_link_dir_to_bin in_target in_dirPath)
    get_filename_component(folderName "${in_dirPath}" NAME)
    set(dst_dir "$<TARGET_FILE_DIR:${in_target}>/${folderName}")

    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${in_dirPath}" "${dst_dir}"
    )
endfunction()