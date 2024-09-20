# Function to collect all currently added targets in all subdirectories
#
# Parameters:
# - result the list containing all found targets
# - dir root directory to start looking from
function(get_all_targets result dir)
  get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)

  foreach(subdir IN LISTS subdirs)
    get_all_targets(${result} "${subdir}")
  endforeach()

  get_directory_property(sub_targets DIRECTORY "${dir}" BUILDSYSTEM_TARGETS)
  set(${result} ${${result}} ${sub_targets} PARENT_SCOPE)
endfunction()
